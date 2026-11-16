// ==============================
// File:			TDCLChunkStream.cpp
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
// The Original Code is TDCLChunkStream.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLChunkStream.h>

// DCL
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>

// ISO C++
#include <utility>

TDCLChunkStream::TDCLChunkStream( EmitFn inEmitFn, std::size_t inAutoFlushThreshold /* = 0 */ )
	: mEmitFn( std::move( inEmitFn ) ),
	  mAutoFlushThreshold( inAutoFlushThreshold )
{
}

void
TDCLChunkStream::Read( void*, KUInt32* ioCount )
{
	if ( ioCount != nullptr )
	{
		*ioCount = 0;
	}
	throw DCLNotImplementedError;
}

void
TDCLChunkStream::Write( const void* inBuffer, KUInt32* ioCount )
{
	if ( ioCount == nullptr || *ioCount == 0 || inBuffer == nullptr )
	{
		return;
	}
	const auto* bytes = static_cast<const std::uint8_t*>( inBuffer );
	mBuffer.insert( mBuffer.end(), bytes, bytes + *ioCount );
	if ( mAutoFlushThreshold != 0 && mBuffer.size() >= mAutoFlushThreshold )
	{
		FlushOutput();
	}
}

void
TDCLChunkStream::FlushOutput( void )
{
	if ( mBuffer.empty() )
	{
		return;
	}
	std::vector<std::uint8_t> chunk;
	chunk.swap( mBuffer );
	if ( mEmitFn )
	{
		mEmitFn( std::move( chunk ) );
	}
}

KUInt8
TDCLChunkStream::PeekByte( void )
{
	throw DCLNotImplementedError;
}
