// ==============================
// File:			TDCLCommLayer.h
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
// The Original Code is TDCLCommLayer.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef __TDCLCOMMLAYER__
#define __TDCLCOMMLAYER__

#include <DCL/Headers/DCLDefinitions.h>

// C++
#include <memory>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLSessionId.h>
#include <DCL/Interfaces/TStoppable.h>

class TDCLServer;
class TDCLSessionBus;

///
/// Abstract base class for communication layers (listeners/transports) that
/// accept connections into session-bus endpoints without exposing TDCLPipe.
///
class TDCLCommLayer
	:
		public TStoppable
{
public:
	enum {
		kDefaultPipeCount	=	0
	};

	enum {
		kNoTimeout			=	-1,
		kDefaultTimeout		=	0
	};

	TDCLCommLayer(
		Boolean	inFlushAvailable = false,
		KUInt32	inMaxPipeCount = kDefaultPipeCount );

	virtual ~TDCLCommLayer( void ) = default;

	virtual void			StartListening( TDCLServer* inServer );
	virtual void			StopListening( void );

	/// Accepts a pending connection and returns a TDCLSessionId registered on
	/// the server's TDCLSessionBus, or an invalid TDCLSessionId if none.
	virtual TDCLSessionId	AcceptSession( void ) = 0;

	/// Refuses the pending connection, if any.
	virtual void			Refuse( void ) = 0;

	/// Returns true if accepted sessions require MNP framing (e.g. serial MNP).
	virtual bool			UsesMNP( void ) const noexcept { return false; }

	virtual bool			IsCurrentThread( void ) const noexcept { return false; }

	inline Boolean			FlushAvailable( void ) const noexcept { return mFlushAvailable; }
	virtual void			Idle( void );

	virtual void			SetTimeout( long inTimeout ) = 0;
	virtual long			GetTimeout( void ) = 0;

protected:
	virtual void			DoStartListening( void ) = 0;
	virtual void			DoStopListening( void ) = 0;

	inline void				SetFlushAvailable( Boolean inNewFlushAvailable ) noexcept
	{
		mFlushAvailable = inNewFlushAvailable;
	}

	inline KUInt32			GetMaxPipeCount( void ) const noexcept { return mMaxPipeCount; }
	inline void				SetMaxPipeCount( KUInt32 inMaxPipeCount ) noexcept { mMaxPipeCount = inMaxPipeCount; }

	inline TDCLServer*		GetServer( void ) const noexcept { return mServer; }
	inline void				SetServer( TDCLServer* inServer ) noexcept { mServer = inServer; }

	inline const std::shared_ptr<TDCLSessionBus>& GetSessionBus( void ) const noexcept
	{
		return mSessionBus;
	}

private:
	TDCLCommLayer( const TDCLCommLayer& inCopy ) = delete;
	TDCLCommLayer& operator = ( const TDCLCommLayer& inCopy ) = delete;

	Boolean							mFlushAvailable;
	KUInt32							mMaxPipeCount;
	TDCLServer*						mServer;
	std::shared_ptr<TDCLSessionBus>	mSessionBus;
};

#endif
// __TDCLCOMMLAYER__
