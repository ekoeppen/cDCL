// ==============================
// File: TDCLClientSocket.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>
#include "TDCLClientSocket.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <chrono>

#if TARGET_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#endif

#include <DCL/Communication_Layers/TDCLBufferedPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

TDCLClientSocket::TDCLClientSocket(
		IDCLThreads* inThreadsIntf,
		std::string inHost,
		unsigned short inPort,
		long inTimeout )
	:
		TDCLPlatformSocket( inThreadsIntf, inPort, 1, inTimeout ),
		mHost( std::move( inHost ) ),
		mPort( inPort ),
		mConnectedSocket( kInvalidSocket ),
		mConnected( false ),
		mStopped( false )
{
}

TDCLClientSocket::~TDCLClientSocket( void )
{
	if (mConnectedSocket != kInvalidSocket)
	{
		CloseSocket( mConnectedSocket );
		mConnectedSocket = kInvalidSocket;
	}

	mWakeWait.Close();
}

void
TDCLClientSocket::DoStartListening( void )
{
	int wakeErr = mWakeWait.EnsureOpen();
	if (wakeErr != 0)
	{
		ThrowSocketError( wakeErr );
	}
	mConnected = false;
	mStopped = false;
}

void
TDCLClientSocket::DoStopListening( void )
{
	mStopped = true;
	mWakeWait.Trigger();
	if (mConnectedSocket != kInvalidSocket)
	{
		CloseSocket( mConnectedSocket );
		mConnectedSocket = kInvalidSocket;
	}
}

Boolean
TDCLClientSocket::WaitForIncomingRequest( void )
{
	if (mConnected || mStopped || StopRequested())
	{
		return false;
	}

	struct addrinfo hints;
	(void) ::memset( &hints, 0, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char portStr[16];
	(void) ::snprintf( portStr, sizeof(portStr), "%u", mPort );

	struct addrinfo* addrList = nullptr;
	int err = ::getaddrinfo( mHost.c_str(), portStr, &hints, &addrList );
	if (err != 0 || addrList == nullptr)
	{
		throw DCLPlatformBadParamError( err );
	}

	dcl_socket_t sock = kInvalidSocket;
	int lastErr = 0;
	bool timedOut = false;

	for (struct addrinfo* cur = addrList; cur != nullptr; cur = cur->ai_next)
	{
		if (mStopped || StopRequested())
		{
			break;
		}

		sock = ::socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );
		if (sock == kInvalidSocket)
		{
			lastErr = GetSocketError();
			continue;
		}

		SetNonBlocking( sock, true );

		int res = ::connect( sock, cur->ai_addr, (socklen_t) cur->ai_addrlen );
		if (res < 0)
		{
			int sockErr = GetSocketError();
#if TARGET_OS_WIN32
			bool inProgress = (sockErr == WSAEWOULDBLOCK || sockErr == WSAEINPROGRESS);
#else
			bool inProgress = (sockErr == EINPROGRESS);
#endif
			if (inProgress)
			{
				long toSecs = GetTimeout();
				if (toSecs <= 0)
				{
					toSecs = 30;
				}
				int waitErr = 0;
				TDCLWakeableWait::EResult waitRes = mWakeWait.WaitReady(
					(dcl_wait_handle_t) sock,
					TDCLWakeableWait::EDirection::kWrite,
					std::chrono::seconds( toSecs ),
					GetStopToken(),
					&waitErr );
				if (waitRes == TDCLWakeableWait::EResult::kTimeout)
				{
					timedOut = true;
				}
				else if (waitRes == TDCLWakeableWait::EResult::kCancelled)
				{
					CloseSocket( sock );
					sock = kInvalidSocket;
					break;
				}
				else if (waitRes == TDCLWakeableWait::EResult::kReady)
				{
					int connectErr = 0;
					socklen_t optLen = sizeof(connectErr);
					if (::getsockopt( sock, SOL_SOCKET, SO_ERROR, (char*) &connectErr, &optLen ) == 0)
					{
						if (connectErr == 0)
						{
							res = 0;
						}
						else
						{
							lastErr = connectErr;
						}
					}
				}
				else
				{
					lastErr = waitErr;
				}
			}
			else
			{
				lastErr = sockErr;
			}
		}

		if (res == 0)
		{
			SetNonBlocking( sock, false );
			int nodelay = 1;
			(void) ::setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, (const char*) &nodelay, sizeof(nodelay) );
			break;
		}

		CloseSocket( sock );
		sock = kInvalidSocket;
	}

	::freeaddrinfo( addrList );

	if (mStopped || StopRequested())
	{
		if (sock != kInvalidSocket)
		{
			CloseSocket( sock );
		}
		return false;
	}

	if (sock != kInvalidSocket)
	{
		mConnectedSocket = sock;
		mConnected = true;
		return true;
	}

	if (timedOut)
	{
		throw DCLTimeout;
	}

	if (lastErr != 0)
	{
		throw DCLPlatformUnknownError( lastErr );
	}

	throw DCLNotAvailable;
}

TDCLPipe*
TDCLClientSocket::DoAccept( void )
{
	TDCLPipe* pipe = nullptr;
	if (mConnectedSocket != kInvalidSocket)
	{
		dcl_socket_t s = mConnectedSocket;
		mConnectedSocket = kInvalidSocket;
		pipe = new TDCLBufferedPipe( CreateSocketPipe( s, GetTimeout() ) );
	}
	return pipe;
}

void
TDCLClientSocket::DoRefuse( void )
{
	if (mConnectedSocket != kInvalidSocket)
	{
		CloseSocket( mConnectedSocket );
		mConnectedSocket = kInvalidSocket;
	}
}
