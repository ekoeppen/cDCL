// ==============================
// File:			TDCLWakeableWaitWin32.cpp
// Project:			Desktop Connection Library
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is TDCLWakeableWaitWin32.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>

#if !TARGET_OS_WIN32
#error TDCLWakeableWaitWin32 requires Win32
#endif

#include <algorithm>
#include <climits>
#include <string.h>
#include <ws2tcpip.h>

namespace
{
int
MakeLoopbackSocketPair( SOCKET outPair[2] )
{
	SOCKET listener = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( listener == INVALID_SOCKET )
	{
		return ::WSAGetLastError();
	}

	struct sockaddr_in addr;
	(void)::memset( &addr, 0, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	addr.sin_port = 0;

	if ( ::bind( listener, (struct sockaddr*)&addr, sizeof( addr ) ) != 0 )
	{
		int err = ::WSAGetLastError();
		::closesocket( listener );
		return err;
	}

	int len = sizeof( addr );
	if ( ::getsockname( listener, (struct sockaddr*)&addr, &len ) != 0 )
	{
		int err = ::WSAGetLastError();
		::closesocket( listener );
		return err;
	}

	if ( ::listen( listener, 1 ) != 0 )
	{
		int err = ::WSAGetLastError();
		::closesocket( listener );
		return err;
	}

	outPair[0] = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( outPair[0] == INVALID_SOCKET )
	{
		int err = ::WSAGetLastError();
		::closesocket( listener );
		return err;
	}

	if ( ::connect( outPair[0], (struct sockaddr*)&addr, sizeof( addr ) ) != 0 )
	{
		int err = ::WSAGetLastError();
		::closesocket( outPair[0] );
		::closesocket( listener );
		return err;
	}

	struct sockaddr_in clientAddr;
	int clientLen = sizeof( clientAddr );
	if ( ::getsockname( outPair[0], (struct sockaddr*)&clientAddr, &clientLen ) != 0 )
	{
		int err = ::WSAGetLastError();
		::closesocket( outPair[0] );
		::closesocket( listener );
		return err;
	}

	for ( ;; )
	{
		struct sockaddr_in acceptedAddr;
		int acceptedLen = sizeof( acceptedAddr );
		outPair[1] = ::accept( listener, (struct sockaddr*)&acceptedAddr, &acceptedLen );
		if ( outPair[1] == INVALID_SOCKET )
		{
			int err = ::WSAGetLastError();
			::closesocket( outPair[0] );
			::closesocket( listener );
			return err;
		}

		if ( ( acceptedLen >= (int)sizeof( acceptedAddr ) ) && ( acceptedAddr.sin_family == clientAddr.sin_family ) &&
		     ( acceptedAddr.sin_addr.s_addr == clientAddr.sin_addr.s_addr ) &&
		     ( acceptedAddr.sin_port == clientAddr.sin_port ) )
		{
			break;
		}

		::closesocket( outPair[1] );
	}

	::closesocket( listener );
	return 0;
}

int
SetNonBlockingSocket( SOCKET inSocket )
{
	u_long mode = 1;
	if ( ::ioctlsocket( inSocket, FIONBIO, &mode ) != 0 )
	{
		return ::WSAGetLastError();
	}
	return 0;
}
} // namespace

TDCLWakeableWait::TDCLWakeableWait( void ) : mWakeSend( INVALID_SOCKET ), mWakeRecv( INVALID_SOCKET )
{
}

TDCLWakeableWait::~TDCLWakeableWait( void )
{
	Close();
}

int
TDCLWakeableWait::EnsureOpen( void )
{
	if ( ( mWakeSend != INVALID_SOCKET ) && ( mWakeRecv != INVALID_SOCKET ) )
	{
		return 0;
	}

	SOCKET pair[2] = { INVALID_SOCKET, INVALID_SOCKET };
	int err = MakeLoopbackSocketPair( pair );
	if ( err != 0 )
	{
		return err;
	}

	err = SetNonBlockingSocket( pair[0] );
	if ( err == 0 )
	{
		err = SetNonBlockingSocket( pair[1] );
	}
	if ( err != 0 )
	{
		::closesocket( pair[0] );
		::closesocket( pair[1] );
		return err;
	}

	mWakeSend = pair[0];
	mWakeRecv = pair[1];
	return 0;
}

void
TDCLWakeableWait::Close( void )
{
	if ( mWakeSend != INVALID_SOCKET )
	{
		::closesocket( mWakeSend );
		mWakeSend = INVALID_SOCKET;
	}
	if ( mWakeRecv != INVALID_SOCKET )
	{
		::closesocket( mWakeRecv );
		mWakeRecv = INVALID_SOCKET;
	}
}

void
TDCLWakeableWait::Trigger( void )
{
	if ( mWakeSend != INVALID_SOCKET )
	{
		const char someByte = 0;
		(void)::send( mWakeSend, &someByte, sizeof( someByte ), 0 );
	}
}

void
TDCLWakeableWait::Drain( void )
{
	if ( mWakeRecv == INVALID_SOCKET )
	{
		return;
	}

	char someBuffer[64];
	while ( ::recv( mWakeRecv, someBuffer, sizeof( someBuffer ), 0 ) > 0 )
	{
	}
}

TDCLWakeableWait::EResult
TDCLWakeableWait::WaitReady( dcl_wait_handle_t inHandle, EDirection inDirection, std::chrono::milliseconds inTimeout,
                             const std::stop_token& inStopToken, int* outError )
{
	if ( inHandle == INVALID_SOCKET )
	{
		if ( outError )
		{
			*outError = WSAENOTSOCK;
		}
		return EResult::kError;
	}

	if ( inStopToken.stop_requested() )
	{
		return EResult::kCancelled;
	}

	const int openErr = EnsureOpen();
	if ( openErr != 0 )
	{
		if ( outError )
		{
			*outError = openErr;
		}
		return EResult::kError;
	}

	Drain();

	struct TStopCallback
	{
		TDCLWakeableWait* mSelf;
		void
		operator()( void ) const
		{
			mSelf->Trigger();
		}
	};

	std::stop_callback<TStopCallback> onStop( inStopToken, TStopCallback{ this } );

	if ( inStopToken.stop_requested() )
	{
		Drain();
		return EResult::kCancelled;
	}

	const bool hasTimeout = ( inTimeout >= std::chrono::milliseconds::zero() );
	const auto deadline =
	    hasTimeout ? ( std::chrono::steady_clock::now() + inTimeout ) : std::chrono::steady_clock::time_point{};

	bool firstIter = true;
	for ( ;; )
	{
		int pollMs = -1;
		if ( hasTimeout )
		{
			auto remaining =
			    std::chrono::duration_cast<std::chrono::milliseconds>( deadline - std::chrono::steady_clock::now() );
			if ( remaining <= std::chrono::milliseconds::zero() )
			{
				if ( !firstIter )
				{
					return EResult::kTimeout;
				}
				pollMs = 0;
			}
			else
			{
				pollMs = (int)std::min<long long>( remaining.count(), INT_MAX );
			}
		}
		firstIter = false;

		WSAPOLLFD pfd[2];
		pfd[0].fd = inHandle;
		pfd[0].events = ( inDirection == EDirection::kRead ) ? POLLRDNORM : POLLWRNORM;
		pfd[0].revents = 0;
		pfd[1].fd = mWakeRecv;
		pfd[1].events = POLLRDNORM;
		pfd[1].revents = 0;

		int rc = ::WSAPoll( pfd, 2, pollMs );
		if ( rc == SOCKET_ERROR )
		{
			int err = ::WSAGetLastError();
			if ( err == WSAEINTR )
			{
				if ( inStopToken.stop_requested() )
				{
					return EResult::kCancelled;
				}
				continue;
			}
			if ( outError )
			{
				*outError = err;
			}
			return EResult::kError;
		}

		if ( rc == 0 )
		{
			if ( inStopToken.stop_requested() )
			{
				return EResult::kCancelled;
			}
			return EResult::kTimeout;
		}

		if ( pfd[1].revents & POLLRDNORM )
		{
			Drain();
			if ( inStopToken.stop_requested() )
			{
				return EResult::kCancelled;
			}
			if ( pfd[0].revents == 0 )
			{
				continue;
			}
		}

		if ( inStopToken.stop_requested() )
		{
			return EResult::kCancelled;
		}

		if ( pfd[0].revents & POLLNVAL )
		{
			if ( outError )
			{
				*outError = WSAENOTSOCK;
			}
			return EResult::kError;
		}

		if ( pfd[0].revents != 0 )
		{
			return EResult::kReady;
		}
	}
}

TDCLWakeableWait::EResult
TDCLWakeableWait::PollNow( dcl_wait_handle_t inHandle, EDirection inDirection, int* outError )
{
	if ( inHandle == INVALID_SOCKET )
	{
		if ( outError )
		{
			*outError = WSAENOTSOCK;
		}
		return EResult::kError;
	}

	WSAPOLLFD pfd;
	pfd.fd = inHandle;
	pfd.events = ( inDirection == EDirection::kRead ) ? POLLRDNORM : POLLWRNORM;
	pfd.revents = 0;

	int rc;
	do
	{
		rc = ::WSAPoll( &pfd, 1, 0 );
	} while ( ( rc == SOCKET_ERROR ) && ( ::WSAGetLastError() == WSAEINTR ) );

	if ( rc == SOCKET_ERROR )
	{
		if ( outError )
		{
			*outError = ::WSAGetLastError();
		}
		return EResult::kError;
	}

	if ( rc == 0 )
	{
		return EResult::kTimeout;
	}

	if ( pfd.revents & POLLNVAL )
	{
		if ( outError )
		{
			*outError = WSAENOTSOCK;
		}
		return EResult::kError;
	}

	return EResult::kReady;
}
