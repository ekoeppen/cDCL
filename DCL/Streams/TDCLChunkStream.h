// ==============================
// File:			TDCLChunkStream.h
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
// The Original Code is TDCLChunkStream.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLCHUNKSTREAM_H
#define _TDCLCHUNKSTREAM_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStream.h>

// ISO C++
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

///
/// Write-only TDCLStream adapter that buffers bytes and emits chunks via a
/// callback on FlushOutput() (or optionally when the buffer exceeds a threshold).
/// Allows TDCLDockCommand::SendCommand() and TDCLInspectorCommand::SendCommand()
/// to serialize directly onto the event bus without blocking transport I/O.
///
class TDCLChunkStream
	:
		public TDCLStream
{
public:
	using EmitFn = std::function<void( std::vector<std::uint8_t>&& )>;

	explicit TDCLChunkStream( EmitFn inEmitFn, std::size_t inAutoFlushThreshold = 0 );
	virtual ~TDCLChunkStream( void ) override = default;

	TDCLChunkStream( const TDCLChunkStream& ) = delete;
	TDCLChunkStream& operator=( const TDCLChunkStream& ) = delete;

	virtual void Read( void* outBuffer, KUInt32* ioCount ) override;
	virtual void Write( const void* inBuffer, KUInt32* ioCount ) override;
	virtual void FlushOutput( void ) override;
	virtual KUInt8 PeekByte( void ) override;

	std::size_t BufferedSize( void ) const noexcept { return mBuffer.size(); }

private:
	EmitFn mEmitFn;
	std::size_t mAutoFlushThreshold;
	std::vector<std::uint8_t> mBuffer;
};

#endif
// _TDCLCHUNKSTREAM_H
