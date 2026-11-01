// ==============================
// File: TDCLClientSocket.cpp
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#include <DCL/Headers/DCLDefinitions.h>
#include "TDCLClientSocket.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

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
	mInterruptPair[0] = kInvalidSocket;
	mInterruptPair[1] = kInvalidSocket;
}

TDCLClientSocket::~TDCLClientSocket( void )
{
	if (mConnectedSocket != kInvalidSocket)
	{
		CloseSocket( mConnectedSocket );
		mConnectedSocket = kInvalidSocket;
	}

	if (mInterruptPair[0] != kInvalidSocket)
	{
		CloseSocket( mInterruptPair[0] );
		mInterruptPair[0] = kInvalidSocket;
	}

	if (mInterruptPair[1] != kInvalidSocket)
	{
		CloseSocket( mInterruptPair[1] );
		mInterruptPair[1] = kInvalidSocket;
	}
}

void
TDCLClientSocket::DoStartListening( void )
{
	if (mInterruptPair[0] == kInvalidSocket)
	{
		if (MakeSocketPair( mInterruptPair ) != 0)
		{
			ThrowSocketError( GetSocketError() );
		}
		if (SetNonBlocking( mInterruptPair[1], true ) != 0)
		{
			ThrowSocketError( GetSocketError() );
		}
	}
	mConnected = false;
	mStopped = false;
}

void
TDCLClientSocket::DoStopListening( void )
{
	mStopped = true;
	if (mInterruptPair[0] != kInvalidSocket)
	{
		char wakeByte = 1;
		(void) ::send( mInterruptPair[0], &wakeByte, sizeof(wakeByte), 0 );
	}
	if (mConnectedSocket != kInvalidSocket)
	{
		CloseSocket( mConnectedSocket );
		mConnectedSocket = kInvalidSocket;
	}
}

Boolean
TDCLClientSocket::WaitForIncomingRequest( void )
{
	if (mConnected || mStopped)
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
		if (mStopped)
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
				fd_set writeSet;
				fd_set exceptSet;
				fd_set readSet;
				FD_ZERO( &writeSet );
				FD_ZERO( &exceptSet );
				FD_ZERO( &readSet );
				FD_SET( sock, &writeSet );
				FD_SET( sock, &exceptSet );
				if (mInterruptPair[1] != kInvalidSocket)
				{
					FD_SET( mInterruptPair[1], &readSet );
				}

				int maxFd = (int) sock;
				if (mInterruptPair[1] != kInvalidSocket && (int) mInterruptPair[1] > maxFd)
				{
					maxFd = (int) mInterruptPair[1];
				}

				struct timeval tv;
				long toSecs = GetTimeout();
				if (toSecs <= 0)
				{
					toSecs = 30;
				}
				tv.tv_sec = toSecs;
				tv.tv_usec = 0;

				int sel = ::select( maxFd + 1, &readSet, &writeSet, &exceptSet, &tv );
				if (sel == 0)
				{
					timedOut = true;
				}
				else if (sel > 0)
				{
					if (mInterruptPair[1] != kInvalidSocket && FD_ISSET( mInterruptPair[1], &readSet ))
					{
						CloseSocket( sock );
						sock = kInvalidSocket;
						break;
					}

					if (FD_ISSET( sock, &writeSet ))
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
				}
				else
				{
					lastErr = GetSocketError();
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

	if (mStopped)
	{
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
