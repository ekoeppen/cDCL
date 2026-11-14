// ==============================
// File:			TDCLWakeableWaitPOSIX.cpp
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
// The Original Code is TDCLWakeableWaitPOSIX.cpp.
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

#if !TARGET_OS_COMPAT_POSIX
#error TDCLWakeableWaitPOSIX requires POSIX compatibility
#endif

#include <algorithm>
#include <climits>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

namespace
{
int
SetNonBlockingAndCloexec( int inFD )
{
	int flags = ::fcntl( inFD, F_GETFL, 0 );
	if ( flags < 0 )
	{
		return errno;
	}
	if ( ::fcntl( inFD, F_SETFL, flags | O_NONBLOCK ) < 0 )
	{
		return errno;
	}
	int fdFlags = ::fcntl( inFD, F_GETFD, 0 );
	if ( fdFlags >= 0 )
	{
		(void)::fcntl( inFD, F_SETFD, fdFlags | FD_CLOEXEC );
	}
	return 0;
}
} // namespace

TDCLWakeableWait::TDCLWakeableWait( void ) : mWakeReadFD( -1 ), mWakeWriteFD( -1 )
{
}

TDCLWakeableWait::~TDCLWakeableWait( void )
{
	Close();
}

int
TDCLWakeableWait::EnsureOpen( void )
{
	if ( ( mWakeReadFD >= 0 ) && ( mWakeWriteFD >= 0 ) )
	{
		return 0;
	}

	int fds[2];
	if ( ::pipe( fds ) != 0 )
	{
		return errno;
	}

	int err = SetNonBlockingAndCloexec( fds[0] );
	if ( err == 0 )
	{
		err = SetNonBlockingAndCloexec( fds[1] );
	}
	if ( err != 0 )
	{
		(void)::close( fds[0] );
		(void)::close( fds[1] );
		return err;
	}

	mWakeReadFD = fds[0];
	mWakeWriteFD = fds[1];
	return 0;
}

void
TDCLWakeableWait::Close( void )
{
	if ( mWakeReadFD >= 0 )
	{
		(void)::close( mWakeReadFD );
		mWakeReadFD = -1;
	}
	if ( mWakeWriteFD >= 0 )
	{
		(void)::close( mWakeWriteFD );
		mWakeWriteFD = -1;
	}
}

void
TDCLWakeableWait::Trigger( void )
{
	if ( mWakeWriteFD >= 0 )
	{
		const char someByte = 0;
		while ( ::write( mWakeWriteFD, &someByte, sizeof( someByte ) ) == -1 )
		{
			if ( errno != EINTR )
			{
				break;
			}
		}
	}
}

void
TDCLWakeableWait::Drain( void )
{
	if ( mWakeReadFD < 0 )
	{
		return;
	}

	char someBuffer[64];
	for ( ;; )
	{
		ssize_t readBytes = ::read( mWakeReadFD, someBuffer, sizeof( someBuffer ) );
		if ( readBytes > 0 )
		{
			continue;
		}
		if ( ( readBytes == -1 ) && ( errno == EINTR ) )
		{
			continue;
		}
		break;
	}
}

TDCLWakeableWait::EResult
TDCLWakeableWait::WaitReady( dcl_wait_handle_t inHandle, EDirection inDirection, std::chrono::milliseconds inTimeout,
                             const std::stop_token& inStopToken, int* outError )
{
	if ( inHandle < 0 )
	{
		if ( outError )
		{
			*outError = EBADF;
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

		struct pollfd pfd[2];
		pfd[0].fd = inHandle;
		pfd[0].events = ( inDirection == EDirection::kRead ) ? POLLIN : POLLOUT;
		pfd[0].revents = 0;
		pfd[1].fd = mWakeReadFD;
		pfd[1].events = POLLIN;
		pfd[1].revents = 0;

		int rc = ::poll( pfd, 2, pollMs );
		if ( rc < 0 )
		{
			int err = errno;
			if ( err == EINTR )
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

		if ( pfd[1].revents & POLLIN )
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
				*outError = EBADF;
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
	if ( inHandle < 0 )
	{
		if ( outError )
		{
			*outError = EBADF;
		}
		return EResult::kError;
	}

	struct pollfd pfd;
	pfd.fd = inHandle;
	pfd.events = ( inDirection == EDirection::kRead ) ? POLLIN : POLLOUT;
	pfd.revents = 0;

	int rc;
	do
	{
		rc = ::poll( &pfd, 1, 0 );
	} while ( ( rc < 0 ) && ( errno == EINTR ) );

	if ( rc < 0 )
	{
		if ( outError )
		{
			*outError = errno;
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
			*outError = EBADF;
		}
		return EResult::kError;
	}

	return EResult::kReady;
}
