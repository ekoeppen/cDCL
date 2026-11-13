// ==============================
// File:			TDCLWakeableWait.h
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
// The Original Code is TDCLWakeableWait.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLWAKEABLEWAIT_H
#define _TDCLWAKEABLEWAIT_H

#include <DCL/Headers/DCLDefinitions.h>

#include <chrono>
#include <stop_token>

#if TARGET_OS_WIN32
#include <winsock2.h>
typedef SOCKET dcl_wait_handle_t;
#else
typedef int dcl_wait_handle_t;
#endif

///
/// I/O wait utility wakeable by std::stop_token.
///
class TDCLWakeableWait
{
public:
	enum class EDirection
	{
		kRead,
		kWrite
	};

	enum class EResult
	{
		kReady,
		kTimeout,
		kCancelled,
		kError
	};

	TDCLWakeableWait( void );
	~TDCLWakeableWait( void );

	TDCLWakeableWait( const TDCLWakeableWait& ) = delete;
	TDCLWakeableWait& operator=( const TDCLWakeableWait& ) = delete;

	///
	/// Opens the wake-up descriptors if needed.
	///
	/// \return 0 on success, or the system error code.
	///
	int EnsureOpen( void );

	///
	/// Closes the wake-up descriptors.
	///
	void Close( void );

	///
	/// Manually triggers the wake-up signal (thread-safe).
	///
	void Trigger( void );

	///
	/// Waits until \p inHandle is ready in direction \p inDirection,
	/// \p inTimeout expires, or \p inStopToken is cancelled.
	///
	/// \param inHandle		file descriptor (POSIX) or SOCKET (Win32) to monitor.
	/// \param inDirection	read or write.
	/// \param inTimeout	maximum wait duration (< 0 means infinite).
	/// \param inStopToken	cooperative cancellation token.
	/// \param outError		optional OS error code when result is kError.
	///
	EResult WaitReady( dcl_wait_handle_t inHandle, EDirection inDirection, std::chrono::milliseconds inTimeout,
	                   const std::stop_token& inStopToken, int* outError = nil );

	///
	/// Checks without blocking (timeout 0) whether \p inHandle is ready.
	/// Stateless and safe to call from any thread.
	///
	static EResult PollNow( dcl_wait_handle_t inHandle, EDirection inDirection, int* outError = nil );

private:
	void Drain( void );

#if TARGET_OS_WIN32
	SOCKET mWakeSend;
	SOCKET mWakeRecv;
#else
	int mWakeReadFD;
	int mWakeWriteFD;
#endif
};

#endif
// _TDCLWAKEABLEWAIT_H
