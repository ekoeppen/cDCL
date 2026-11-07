// ==============================
// Fichier:			TTestMemoryPipe.h
// Projet:			Desktop Connection Library
//
// Version: MPL 1.1
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
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
