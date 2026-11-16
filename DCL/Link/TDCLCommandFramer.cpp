// ==============================
// File:			TDCLCommandFramer.cpp
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
// The Original Code is TDCLCommandFramer.cpp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLCommandFramer.h>

// K
#include <K/Defines/UByteSex.h>

// DCL
#include <DCL/Exceptions/Link_Exceptions/TDCLBadDockCmdException.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdArray.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdGeneric.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonInfo.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNewtonName.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNoData.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdNSObject.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPackageList.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdPassword.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdSingleLong.h>
#include <DCL/Link/Dock_Commands/TDCLDockCmdTwoNSObjects.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCommand.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCode.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdCodeBlock.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdException.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdExceptionRef.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdFuncObj.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdResult.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdStackFrame.h>
#include <DCL/Link/Inspector_Commands/TDCLInspectorCmdText.h>

// ISO C++
#include <cstring>

namespace
{
KUInt32
ReadBE32( const std::uint8_t* inPtr )
{
	KUInt32 raw = 0;
	std::memcpy( &raw, inPtr, sizeof( raw ) );
	return UByteSex_FromBigEndian( raw );
}

KUInt16
ReadBE16( const std::uint8_t* inPtr )
{
	KUInt16 raw = 0;
	std::memcpy( &raw, inPtr, sizeof( raw ) );
	return UByteSex_FromBigEndian( raw );
}
} // namespace

TDCLCommandFramer::TDCLCommandFramer( EMode inMode /* = EMode::kDock */ )
	: mMode( inMode )
{
}

void
TDCLCommandFramer::Reset( void )
{
	mState = EState::kHeader;
	mBuffer.clear();
	mOffset = 0;
	mCommand = 0;
	mBodyLength = 0;
	mPaddedLength = 0;
	mBackupShorts.clear();
	mBackupConsumePadding = false;
	mCommands.clear();
	mError = nullptr;
}

std::unique_ptr<TDCLDockCommand>
TDCLCommandFramer::TakeCommand( void )
{
	if ( mCommands.empty() )
	{
		return nullptr;
	}
	auto cmd = std::move( mCommands.front() );
	mCommands.pop_front();
	return cmd;
}

void
TDCLCommandFramer::Consume( std::size_t inCount )
{
	mOffset += inCount;
	if ( mOffset == mBuffer.size() )
	{
		mBuffer.clear();
		mOffset = 0;
	}
	else if ( mOffset >= 4096 )
	{
		mBuffer.erase( mBuffer.begin(), mBuffer.begin() + static_cast<std::ptrdiff_t>( mOffset ) );
		mOffset = 0;
	}
}

void
TDCLCommandFramer::SetError( std::exception_ptr inError )
{
	mState = EState::kError;
	mError = std::move( inError );
}

TDCLCommandFramer::EStatus
TDCLCommandFramer::Feed( const std::uint8_t* inData, std::size_t inSize )
{
	if ( mState == EState::kError )
	{
		return EStatus::kError;
	}
	if ( mState == EState::kEOF )
	{
		return mCommands.empty() ? EStatus::kEOF : EStatus::kCommandReady;
	}

	if ( inSize != 0 && inData != nullptr )
	{
		mBuffer.insert( mBuffer.end(), inData, inData + inSize );
	}

	while ( mState != EState::kEOF && mState != EState::kError )
	{
		if ( mState == EState::kHeader )
		{
			if ( Available() < 16 )
			{
				break;
			}

			const std::uint8_t* hdr = Cursor();
			const KUInt32 w0 = ReadBE32( hdr );
			const KUInt32 w1 = ReadBE32( hdr + 4 );
			const KUInt32 cmd = ReadBE32( hdr + 8 );
			const KUInt32 len = ReadBE32( hdr + 12 );
			Consume( 16 );

			if ( w0 == 0 || w1 == 0 )
			{
				mState = EState::kEOF;
				break;
			}

			const KUInt32 expectedSecond = ( mMode == EMode::kDock ) ? 'dock' : 'ntp ';
			if ( w0 != 'newt' || w1 != expectedSecond )
			{
				SetError( std::make_exception_ptr( DCLBadDockCmd ) );
				break;
			}

			mCommand = cmd;
			if ( len == 0 )
			{
				mCommands.push_back( std::make_unique<TDCLDockCmdNoData>( cmd ) );
				continue;
			}

			if ( len == 0xFFFFFFFFu )
			{
				if ( mMode == EMode::kDock && cmd != TDCLDockCommand::kDBackupIDs )
				{
					SetError( std::make_exception_ptr( DCLBadDockCmd ) );
					break;
				}
				mBackupShorts.clear();
				mBackupConsumePadding = false;
				mState = EState::kBackupIDs;
				continue;
			}

			if ( len > 0x7FFFFFF0u )
			{
				SetError( std::make_exception_ptr( DCLBadDockCmd ) );
				break;
			}

			mBodyLength = len;
			mPaddedLength = ( mMode == EMode::kDock ) ? ( ( len + 3u ) & ~3u ) : len;
			mState = EState::kBody;
		}
		else if ( mState == EState::kBody )
		{
			if ( Available() < mPaddedLength )
			{
				break;
			}
			try
			{
				auto obj = BuildCommand( mCommand, mBodyLength, Cursor() );
				Consume( mPaddedLength );
				if ( obj )
				{
					mCommands.push_back( std::move( obj ) );
				}
				mState = EState::kHeader;
			}
			catch ( ... )
			{
				Consume( mPaddedLength );
				SetError( std::current_exception() );
				break;
			}
		}
		else if ( mState == EState::kBackupIDs )
		{
			if ( Available() < sizeof( KUInt16 ) )
			{
				break;
			}
			const KUInt16 value = ReadBE16( Cursor() );
			Consume( sizeof( KUInt16 ) );

			if ( mBackupConsumePadding )
			{
				mCommands.push_back( std::make_unique<TDCLDockCmdGeneric>(
					mCommand,
					static_cast<KUInt32>( mBackupShorts.size() * sizeof( KUInt16 ) ),
					mBackupShorts.data() ) );
				mBackupShorts.clear();
				mBackupConsumePadding = false;
				mState = EState::kHeader;
				continue;
			}

			mBackupShorts.push_back( value );
			if ( value == 0x8000 )
			{
				if ( ( mBackupShorts.size() & 1u ) != 0 )
				{
					mBackupConsumePadding = true;
				}
				else
				{
					mCommands.push_back( std::make_unique<TDCLDockCmdGeneric>(
						mCommand,
						static_cast<KUInt32>( mBackupShorts.size() * sizeof( KUInt16 ) ),
						mBackupShorts.data() ) );
					mBackupShorts.clear();
					mState = EState::kHeader;
				}
			}
		}
	}

	if ( !mCommands.empty() )
	{
		return EStatus::kCommandReady;
	}
	if ( mState == EState::kError )
	{
		return EStatus::kError;
	}
	if ( mState == EState::kEOF )
	{
		return EStatus::kEOF;
	}
	return EStatus::kNeedMoreBytes;
}

std::unique_ptr<TDCLDockCommand>
TDCLCommandFramer::BuildCommand( KUInt32 inCommand, KUInt32 inLength, const void* inData )
{
	if ( mMode == EMode::kInspector )
	{
		if ( inLength == sizeof( KUInt32 ) )
		{
			const KUInt32 val = ReadBE32( static_cast<const std::uint8_t*>( inData ) );
			return std::make_unique<TDCLDockCmdSingleLong>( inCommand, val );
		}
		switch ( inCommand )
		{
		case TDCLInspectorCommand::kDStackFrame:
			return std::make_unique<TDCLInspectorCmdStackFrame>( inLength, inData );
		case TDCLInspectorCommand::kDText:
			return std::make_unique<TDCLInspectorCmdText>( inLength, inData );
		case TDCLInspectorCommand::kDException:
			return std::make_unique<TDCLInspectorCmdException>( inLength, inData );
		case TDCLInspectorCommand::kDExceptionRef:
			return std::make_unique<TDCLInspectorCmdExceptionRef>( inLength, inData );
		case TDCLInspectorCommand::kDCode:
			return std::make_unique<TDCLInspectorCmdCode>( inLength, inData );
		case TDCLInspectorCommand::kDCodeBlock:
			return std::make_unique<TDCLInspectorCmdCodeBlock>( inLength, inData );
		case TDCLInspectorCommand::kDResult:
			return std::make_unique<TDCLInspectorCmdResult>( inLength, inData );
		case TDCLInspectorCommand::kDFunctionObject:
			return std::make_unique<TDCLInspectorCmdFuncObj>( inLength, inData );
		default:
			return std::make_unique<TDCLDockCmdGeneric>( inCommand, inLength, inData );
		}
	}

	switch ( inCommand )
	{
	case TDCLDockCommand::kDNewtonName:
		return std::make_unique<TDCLDockCmdNewtonName>( inLength, inData );
	case TDCLDockCommand::kDPassword:
		return std::make_unique<TDCLDockCmdPassword>( inLength, inData );
	case TDCLDockCommand::kDNewtonInfo:
		return std::make_unique<TDCLDockCmdNewtonInfo>( inLength, inData );
	case TDCLDockCommand::kDPackageIDList:
		return std::make_unique<TDCLDockCmdPackageList>( inLength, inData );
	case TDCLDockCommand::kDHello:
		throw DCLBadDockCmd;
	case TDCLDockCommand::kDQuery:
	case TDCLDockCommand::kDDefaultStore:
	case TDCLDockCommand::kDAppNames:
	case TDCLDockCommand::kDImportParameterSlipResult:
	case TDCLDockCommand::kDPackageInfo:
	case TDCLDockCommand::kDCallResult:
	case TDCLDockCommand::kDRemovePackage:
	case TDCLDockCommand::kDGetPackageInfo:
	case TDCLDockCommand::kDImportParametersSlip:
	case TDCLDockCommand::kDGetPassword:
	case TDCLDockCommand::kDSetStoreName:
	case TDCLDockCommand::kDRefTest:
	case TDCLDockCommand::kDSyncOptions:
	case TDCLDockCommand::kDSyncResults:
	case TDCLDockCommand::kDSetStoreGetNames:
	case TDCLDockCommand::kDRequestToBrowse:
	case TDCLDockCommand::kDSetPath:
	case TDCLDockCommand::kDSetDrive:
	case TDCLDockCommand::kDGetFileInfo:
	case TDCLDockCommand::kDInternalStore:
	case TDCLDockCommand::kDDevices:
	case TDCLDockCommand::kDFilters:
	case TDCLDockCommand::kDPath:
	case TDCLDockCommand::kDFilesAndFolders:
	case TDCLDockCommand::kDFileInfo:
	case TDCLDockCommand::kDImportFile:
	case TDCLDockCommand::kDTranslatorList:
	case TDCLDockCommand::kDSoupsChanged:
	case TDCLDockCommand::kDLoadPackageFile:
	case TDCLDockCommand::kDRestoreFile:
	case TDCLDockCommand::kDRestoreOptions:
	case TDCLDockCommand::kDRestorePackage:
	case TDCLDockCommand::kDStoreNames:
	case TDCLDockCommand::kDIndexDescription:
	case TDCLDockCommand::kDEntry:
	case TDCLDockCommand::kDSetCurrentStore:
	case TDCLDockCommand::kDAddEntry:
	case TDCLDockCommand::kDChangedEntry:
	case TDCLDockCommand::kDSoupInfo:
		return std::make_unique<TDCLDockCmdNSObject>( inCommand, inLength, inData );
	case TDCLDockCommand::kDSoupNames:
		return std::make_unique<TDCLDockCmdTwoNSObjects>( inCommand, inLength, inData );
	case TDCLDockCommand::kDSoupIDs:
	case TDCLDockCommand::kDChangedIDs:
		return std::make_unique<TDCLDockCmdArray>( inCommand, inLength, const_cast<void*>( inData ) );
	default:
		if ( inLength == sizeof( KUInt32 ) )
		{
			const KUInt32 val = ReadBE32( static_cast<const std::uint8_t*>( inData ) );
			return std::make_unique<TDCLDockCmdSingleLong>( inCommand, val );
		}
		return std::make_unique<TDCLDockCmdGeneric>( inCommand, inLength, inData );
	}
}
