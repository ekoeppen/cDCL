// ==============================
// File:			TTestMemoryPipe.cpp
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
// The Original Code is TTestMemoryPipe.cpp.
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

#include "TTestMemoryPipe.h"

#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLPositionException.h>

#include <algorithm>

TTestMemoryPipe::TTestMemoryPipe( void ) : TDCLPipe( nullptr )
{
}

void
TTestMemoryPipe::Read( void* outBuffer, KUInt32* ioCount )
{
	std::lock_guard<std::mutex> lock( mMutex );
	const KUInt32 wanted = *ioCount;
	const KUInt32 count = static_cast<KUInt32>( std::min<std::size_t>( wanted, mInbound.size() ) );
	std::copy_n( mInbound.begin(), count, static_cast<std::uint8_t*>( outBuffer ) );
	mInbound.erase( mInbound.begin(), mInbound.begin() + count );
	*ioCount = count;
	if ( count < wanted && ( mClosed || mDisconnected ) )
	{
		throw DCLEOF;
	}
}

void
TTestMemoryPipe::Write( const void* inBuffer, KUInt32* ioCount )
{
	const auto* bytes = static_cast<const std::uint8_t*>( inBuffer );
	std::function<void( const std::vector<std::uint8_t>& )> handler;
	{
		std::lock_guard<std::mutex> lock( mMutex );
		if ( mDisconnected )
		{
			*ioCount = 0;
			throw DCLEOF;
		}
		mWritten.insert( mWritten.end(), bytes, bytes + *ioCount );
		handler = mOnWrite;
	}
	if ( handler )
	{
		handler( std::vector<std::uint8_t>( bytes, bytes + *ioCount ) );
	}
}

void
TTestMemoryPipe::SetOnWrite( std::function<void( const std::vector<std::uint8_t>& )> inHandler )
{
	std::lock_guard<std::mutex> lock( mMutex );
	mOnWrite = std::move( inHandler );
}

void
TTestMemoryPipe::FlushOutput( void )
{
}

bool
TTestMemoryPipe::BytesAvailable( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	return !mInbound.empty();
}

void
TTestMemoryPipe::Disconnect( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	mDisconnected = true;
}

KUInt8
TTestMemoryPipe::PeekByte( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	if ( mInbound.empty() )
	{
		throw DCLPosition;
	}
	return mInbound.front();
}

long
TTestMemoryPipe::GetTimeout( void )
{
	return mTimeout;
}

void
TTestMemoryPipe::SetTimeout( long inTimeout )
{
	mTimeout = inTimeout;
}

void
TTestMemoryPipe::ClearDataPresent( void )
{
}

void
TTestMemoryPipe::Feed( const std::vector<std::uint8_t>& inBytes )
{
	std::lock_guard<std::mutex> lock( mMutex );
	mInbound.insert( mInbound.end(), inBytes.begin(), inBytes.end() );
}

void
TTestMemoryPipe::Close( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	mClosed = true;
}

std::vector<std::uint8_t>
TTestMemoryPipe::TakeWritten( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	std::vector<std::uint8_t> result;
	result.swap( mWritten );
	return result;
}

bool
TTestMemoryPipe::IsDisconnected( void )
{
	std::lock_guard<std::mutex> lock( mMutex );
	return mDisconnected;
}
