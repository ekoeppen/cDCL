// ==============================
// File:			TTestMemoryPipe.h
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
// The Original Code is TTestMemoryPipe.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef _TTESTMEMORYPIPE_H
#define _TTESTMEMORYPIPE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipe.h>

#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <vector>

///
/// In-memory duplex pipe for tests. The test feeds inbound bytes (possibly
/// from another thread) and inspects the bytes written by the code under
/// test.
///
/// Thread-safe: Feed/Close may be called from a thread other than the
/// reader.
///
class TTestMemoryPipe : public TDCLPipe
{
public:
	TTestMemoryPipe( void );

	/// \name TDCLPipe
	virtual void Read( void* outBuffer, KUInt32* ioCount ) override;
	virtual void Write( const void* inBuffer, KUInt32* ioCount ) override;
	virtual void FlushOutput( void ) override;
	virtual Boolean BytesAvailable( void ) override;
	virtual void Disconnect( void ) override;
	virtual KUInt8 PeekByte( void ) override;
	virtual long GetTimeout( void ) override;
	virtual void SetTimeout( long inTimeout ) override;
	virtual void ClearDataPresent( void ) override;

	/// \name Test control
	void Feed( const std::vector<std::uint8_t>& inBytes );
	/// Ends the inbound stream: further reads past the data throw EOF.
	void Close( void );
	/// Takes and clears everything written so far.
	std::vector<std::uint8_t> TakeWritten( void );
	bool IsDisconnected( void );
	/// Called after every Write with the bytes written (outside the lock),
	/// e.g. to let a scripted peer answer. It may call Feed.
	void SetOnWrite( std::function<void( const std::vector<std::uint8_t>& )> inHandler );

private:
	std::mutex mMutex;
	std::deque<std::uint8_t> mInbound;
	std::vector<std::uint8_t> mWritten;
	long mTimeout = 0;
	bool mClosed = false;
	bool mDisconnected = false;
	std::function<void( const std::vector<std::uint8_t>& )> mOnWrite;
};

#endif
// _TTESTMEMORYPIPE_H
