// ==============================
// File:			TDCLCommandFramer.h
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
// The Original Code is TDCLCommandFramer.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLCOMMANDFRAMER_H
#define _TDCLCOMMANDFRAMER_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>

// ISO C++
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <memory>
#include <vector>

///
/// Incremental non-blocking parser for Dock ('newt''dock') and Inspector
/// ('newt''ntp ') commands over arbitrary byte chunks.
///
class TDCLCommandFramer
{
public:
	enum class EMode
	{
		kDock,
		kInspector
	};

	enum class EStatus
	{
		kNeedMoreBytes,
		kCommandReady,
		kEOF,
		kError
	};

	explicit TDCLCommandFramer( EMode inMode = EMode::kDock );
	~TDCLCommandFramer( void ) = default;

	TDCLCommandFramer( const TDCLCommandFramer& ) = delete;
	TDCLCommandFramer& operator=( const TDCLCommandFramer& ) = delete;

	void SetMode( EMode inMode ) noexcept { mMode = inMode; }
	EMode GetMode( void ) const noexcept { return mMode; }

	void Reset( void );

	/// Feeds raw bytes into the framer and parses any complete commands.
	EStatus Feed( const std::uint8_t* inData, std::size_t inSize );
	EStatus Feed( const std::vector<std::uint8_t>& inBytes )
	{
		return Feed( inBytes.data(), inBytes.size() );
	}

	bool HasCommand( void ) const noexcept { return !mCommands.empty(); }
	std::unique_ptr<TDCLDockCommand> TakeCommand( void );

	bool IsEOF( void ) const noexcept { return mState == EState::kEOF; }
	bool HasError( void ) const noexcept { return mState == EState::kError; }
	std::exception_ptr GetError( void ) const noexcept { return mError; }

private:
	enum class EState
	{
		kHeader,
		kBody,
		kBackupIDs,
		kEOF,
		kError
	};

	std::size_t Available( void ) const noexcept { return mBuffer.size() - mOffset; }
	const std::uint8_t* Cursor( void ) const noexcept { return mBuffer.data() + mOffset; }
	void Consume( std::size_t inCount );
	void SetError( std::exception_ptr inError );

	std::unique_ptr<TDCLDockCommand> BuildCommand(
		KUInt32 inCommand,
		KUInt32 inLength,
		const void* inData );

	EMode mMode;
	EState mState = EState::kHeader;

	std::vector<std::uint8_t> mBuffer;
	std::size_t mOffset = 0;

	KUInt32 mCommand = 0;
	KUInt32 mBodyLength = 0;
	KUInt32 mPaddedLength = 0;

	std::vector<KUInt16> mBackupShorts;
	bool mBackupConsumePadding = false;

	std::deque<std::unique_ptr<TDCLDockCommand>> mCommands;
	std::exception_ptr mError;
};

#endif
// _TDCLCOMMANDFRAMER_H
