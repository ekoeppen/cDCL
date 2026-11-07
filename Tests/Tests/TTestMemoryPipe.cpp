// ==============================
// Fichier:			TTestMemoryPipe.cpp
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
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

Boolean
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
