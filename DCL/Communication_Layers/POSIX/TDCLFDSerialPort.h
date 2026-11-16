// ==============================
// File:			TDCLFDSerialPort.h
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
// The Original Code is TDCLFDSerialPort.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacik <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Simon Stapleton <simon@tufty.co.uk>
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLFDSERIALPORT_H
#define _TDCLFDSERIALPORT_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <DCL/Interfaces/TDCLEventBus.h>

// C++
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <exception>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

// POSIX
#include <termios.h>

///
/// POSIX serial-port communication layer with single-FD ownership on one
/// worker thread alternating between Listening, PendingAccept, and Connected
/// states and communicating via TDCLSessionBus.
///
class TDCLFDSerialPort
	:
		public TDCLCommLayer
{
public:
	enum {
		kModePlain,
		kModeMNP,
		kModeHammer
	};

	TDCLFDSerialPort(
		const char* inDevPath,
		long inBaud,
		KUInt32 inMode );

	TDCLFDSerialPort(
		const char* inDevPath,
		long inBaud = 38400,
		Boolean inUseMNP = true );

	virtual ~TDCLFDSerialPort( void );

	virtual void			StopListening( void ) override;
	virtual TDCLSessionId	AcceptSession( void ) override;
	virtual void			Refuse( void ) override;
	virtual bool			UsesMNP( void ) const noexcept override { return mPipeMode == kModeMNP; }
	virtual bool			IsCurrentThread( void ) const noexcept override;
	virtual void			SetTimeout( long inTimeout ) override;
	virtual long			GetTimeout( void ) override;

protected:
	virtual void			DoStartListening( void ) override;
	virtual void			DoStopListening( void ) override;

	inline int				GetSerialFD( void ) const noexcept { return mSerialFD; }
	void					Init( const char* inDevPath );

private:
	enum class EPortState
	{
		kStopped,
		kListening,
		kPendingAccept,
		kConnected
	};

	TDCLFDSerialPort( const TDCLFDSerialPort& inCopy ) = delete;
	TDCLFDSerialPort& operator = ( const TDCLFDSerialPort& inCopy ) = delete;

	void				WorkerEntry( std::stop_token inStopToken );
	void				OpenAndConfigure( void );
	static void			CloseSerialFD( int inFD );
	void				CloseCurrentFD( bool inDrainOutput, bool inFlushInput );
	void				ResetSerialFD( bool inDrainOutput, bool inFlushInput );
	bool				QueueOutbound( TDCLSessionId inSessionId, std::vector<std::uint8_t>&& inBytes );
	void				RequestCloseSession( TDCLSessionId inSessionId );

	int							mSerialFD;
	char*						mDevPath;
	long						mBaud;
	KUInt32						mPipeMode;
	struct termios				mOriginalOptions;
	long						mTimeout;
	TDCLWakeableWait			mWakeWait;

	std::mutex					mStateMutex;
	std::condition_variable_any	mStateCV;
	EPortState					mPortState;
	bool						mRefuseRequested;
	std::exception_ptr			mRefuseException;
	bool						mCloseRequested;
	bool						mCancelFlushRequested;
	std::stop_source*			mIterStopSource;

	TDCLSessionId				mActiveSessionId;
	TDCLSessionBus::DownstreamGuard mDownstreamGuard;
	std::deque<std::vector<std::uint8_t>> mOutboundQueue;
	std::size_t					mOutboundFrontOffset;

	std::jthread				mWorkerThread;
	std::atomic<std::thread::id> mWorkerThreadId;
};

#endif
// _TDCLFDSERIALPORT_H
