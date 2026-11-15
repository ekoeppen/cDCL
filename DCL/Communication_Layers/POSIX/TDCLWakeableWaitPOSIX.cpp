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

bool
TDCLWakeableWait::IsValidHandle( dcl_wait_handle_t inHandle, int* outError )
{
	if ( inHandle < 0 )
	{
		if ( outError )
		{
			*outError = EBADF;
		}
		return false;
	}
	return true;
}

int
TDCLWakeableWait::OpenWakePairLocked( void )
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
TDCLWakeableWait::CloseWakePairLocked( void )
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
TDCLWakeableWait::TriggerWakePairLocked( void )
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

TDCLWakeableWait::EPollResult
TDCLWakeableWait::PollOne( dcl_wait_handle_t inHandle, EDirection inDirection, int* outError )
{
	struct pollfd pfd;
	pfd.fd = inHandle;
	pfd.events = ( inDirection == EDirection::kRead ) ? POLLIN : POLLOUT;
	pfd.revents = 0;

	const int rc = ::poll( &pfd, 1, 0 );
	if ( rc < 0 )
	{
		const int err = errno;
		if ( err == EINTR )
		{
			return EPollResult::kInterrupted;
		}
		if ( outError )
		{
			*outError = err;
		}
		return EPollResult::kError;
	}
	if ( rc == 0 )
	{
		return EPollResult::kTimeout;
	}
	if ( pfd.revents & POLLNVAL )
	{
		if ( outError )
		{
			*outError = EBADF;
		}
		return EPollResult::kError;
	}
	return ( pfd.revents != 0 ) ? EPollResult::kReady : EPollResult::kTimeout;
}

TDCLWakeableWait::EPollResult
TDCLWakeableWait::PollTwo( dcl_wait_handle_t inHandle, EDirection inDirection, int inTimeoutMs, int* outError ) const
{
	struct pollfd pfd[2];
	pfd[0].fd = inHandle;
	pfd[0].events = ( inDirection == EDirection::kRead ) ? POLLIN : POLLOUT;
	pfd[0].revents = 0;
	pfd[1].fd = mWakeReadFD;
	pfd[1].events = POLLIN;
	pfd[1].revents = 0;

	const int rc = ::poll( pfd, 2, inTimeoutMs );
	if ( rc < 0 )
	{
		const int err = errno;
		if ( err == EINTR )
		{
			return EPollResult::kInterrupted;
		}
		if ( outError )
		{
			*outError = err;
		}
		return EPollResult::kError;
	}

	if ( rc == 0 )
	{
		return EPollResult::kTimeout;
	}

	if ( pfd[0].revents & POLLNVAL )
	{
		if ( outError )
		{
			*outError = EBADF;
		}
		return EPollResult::kError;
	}

	const bool handleReady = ( pfd[0].revents != 0 );
	const bool wakeReady = ( ( pfd[1].revents & POLLIN ) != 0 );
	if ( handleReady && wakeReady )
	{
		return EPollResult::kReadyAndWake;
	}
	if ( wakeReady )
	{
		return EPollResult::kWake;
	}
	return handleReady ? EPollResult::kReady : EPollResult::kTimeout;
}
