// ==============================
// File:			TDCLLink.h
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
// The Original Code is TDCLLink.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2000-2004 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLLINK_H
#define _TDCLLINK_H

#include <DCL/Headers/DCLDefinitions.h>

// C++
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Communication_Layers/TMNPProtocol.h>
#include <DCL/Interfaces/TDCLEventBus.h>
#include <DCL/Interfaces/TDCLEventQueue.h>
#include <DCL/Interfaces/TDCLSessionId.h>
#include <DCL/Link/TDCLCommandFramer.h>
#include <DCL/Link/Dock_Commands/TDCLDockCommand.h>
#include <DCL/Streams/TDCLChunkStream.h>

class TDCLApplication;
class TDCLAppCommand;
class TDCLServer;

///
/// Abstract base class for Newton connection links driven by a single
/// TDCLEventQueue loop over TDCLSessionBus (no TDCLPipe or sidecar watcher).
///
class TDCLLink
{
public:
	enum {
		kDefaultIdlePeriod	= 15000
	};

	enum EState {
		kDisconnected,
		kConnecting,
		kConnected,
		kRunning,
		kDisconnecting,
		kUnknown
	};

	inline TDCLApplication*	GetApplication( void )
	{
		return mApplication;
	}

	inline KUInt32		GetType( void )
	{
		return mType;
	}

	inline void			SetType( KUInt32 inType )
	{
		mType = inType;
	}

	/// Connects the link to an accepted session managed by \p inServer.
	Boolean				Connect(
							TDCLSessionId inSessionId,
							TDCLServer* inServer,
							TDCLCommLayer* inCommLayer );

	/// Connects the link directly to a session on \p inBus (e.g. unit tests).
	Boolean				Connect(
							TDCLSessionId inSessionId,
							std::shared_ptr<TDCLSessionBus> inBus,
							bool inUseMNP = false,
							long inTimeout = TDCLCommLayer::kDefaultTimeout );

	Boolean				Disconnect( void );
	Boolean				Close( void );

	bool				RequestStop( void );
	bool				StopRequested( void ) const noexcept;
	bool				Wait( void );

	inline bool			IsCurrentThread( void ) const noexcept
	{
		return std::this_thread::get_id() == mThreadId.load();
	}

	Boolean				IsConnected( void );

	inline Boolean		IsBusy( void )
	{
		return mIsBusy;
	}

	inline TDCLSessionId	GetSessionId( void ) const noexcept { return mSessionId; }
	inline TDCLCommLayer*	GetCommLayer( void ) const noexcept { return mCommLayer; }
	inline long			GetTimeout( void ) const noexcept { return mTimeout; }
	inline Boolean		PeerDisconnected( void ) const noexcept { return mPeerDisconnected; }

	/// Outbound stream & command helpers used by link subclasses and engines.
	inline TDCLStream*		GetOutputStream( void ) noexcept { return &mOutputStream; }
	void				SendCommand(
							TDCLDockCommand* inCommand,
							ProgressFuncPtr inProgressFuncPtr = nil,
							void* inRefCon = nil );
	void				FlushOutput( void );

	inline const KUInt16*	GetNewtonName( void ) { return mNewtonName; }
	inline KUInt32			GetNewtonID( void ) { return mNewtonID; }
	inline KUInt32			GetManufacturer( void ) { return mManufacturer; }
	inline KUInt32			GetMachineType( void ) { return mMachineType; }
	inline KUInt32			GetROMVersion( void ) { return mROMVersion; }
	inline KUInt32			GetROMStage( void ) { return mROMStage; }
	inline KUInt32			GetRAMSize( void ) { return mRAMSize; }
	inline KUInt32			GetScreenHeight( void ) { return mScreenHeight; }
	inline KUInt32			GetScreenWidth( void ) { return mScreenWidth; }
	inline KUInt32			GetSystemUpdate( void ) { return mSystemUpdate; }
	inline KUInt32			GetNOSVersion( void ) { return mNOSVersion; }
	inline KUInt32			GetInternalStoreSig( void ) { return mInternalStoreSig; }
	inline KUInt32			GetVertScreenRes( void ) { return mVertScreenRes; }
	inline KUInt32			GetHoriScreenRes( void ) { return mHoriScreenRes; }
	inline KUInt32			GetScreenDepth( void ) { return mScreenDepth; }

	virtual		~TDCLLink( void );

protected:
	enum {
		kDefaultTimeout	=	30
	};

	TDCLLink(
		TDCLApplication* inApplication,
		KUInt32 inType = TDCLDockCommand::kNone_SType,
		KUInt32 inIdlePeriod = kDefaultIdlePeriod );

	Boolean				PostAppCommand( TDCLAppCommand* inCommand );

	virtual void		DoConnect( void );
	virtual void		DoDisconnect( void );

	virtual	EState	ProcessDockCommand(
								TDCLDockCommand* inCommand,
								Boolean* outProcessed ) = 0;

	virtual	EState	ProcessAppCommand(
								TDCLAppCommand* inCommand,
								Boolean* outProcessed );

	virtual	EState			Idle( void );
	virtual void	Run( void );
	virtual void	HandleException( TDCLException* inException );

	inline void			PostEvent( TDCLEvent* inEvent )
	{
		mEventQueue.PostEvent( inEvent );
	}

	void				NotifyConnected( void );
	virtual void		Connected( void );
	virtual void		Disconnected( void );
	virtual KUInt32		GetAppCommandMask( void );

	/// Non-blocking-I/O event-queue command receiver (replaces blocking TDCLPipe::Read).
	virtual std::unique_ptr<TDCLDockCommand> ReceiveCommand( void );
	KSInt32				GetResultCmd( void );

	void				SetCommandMode( TDCLCommandFramer::EMode inMode ) noexcept;
	void				SetMNPKeepAlive( bool inKeepAlive ) noexcept;
	void				SetMNPStallTimeout( std::chrono::milliseconds inTimeout ) noexcept;
	std::chrono::milliseconds	GetMNPStallTimeout( void ) const noexcept { return mMNPStallTimeout; }

private:
	void				ThreadEntry( std::stop_token inStopToken );
	TDCLLink( const TDCLLink& inCopy ) = delete;
	TDCLLink& operator = ( const TDCLLink& inCopy ) = delete;

	EState				TransitionFromRunning( void );
	void				SendRawBytes( std::vector<std::uint8_t>&& inBytes );
	void				FlushMNPOutbound( void );
	void				ProcessInboundBytes( const std::vector<std::uint8_t>& inBytes );
	void				EnsureMNPConnected( void );
	void				DrainMNPAndDisconnect( void );
	void				ResetMNPStallTracking( void ) noexcept;
	void				RefreshMNPStallDeadline( void ) noexcept;
	void				HandleMNPStall( void );
	void				CloseSessionTransport( void ) noexcept;

protected:
	enum {
		kClosingTimeout	= 5
	};

	TDCLServer*						mServer;
	TDCLCommLayer*					mCommLayer;
	TDCLApplication*				mApplication;
	std::shared_ptr<TDCLSessionBus>	mSessionBus;
	TDCLSessionId					mSessionId;
	TDCLSessionBus::UpstreamGuard	mUpstreamGuard;
	TDCLEventQueue					mEventQueue;

	bool							mUseMNP;
	bool							mMNPKeepAlive;
	bool							mPeerDisconnected;
	bool							mCloseOnExit;
	long							mTimeout;
	std::chrono::milliseconds		mMNPStallTimeout;
	bool							mMNPStallActive;
	std::uint32_t					mMNPStallProgress;
	std::chrono::steady_clock::time_point	mMNPStallDeadline;
	std::unique_ptr<TMNPProtocol>	mMNPProtocol;
	TDCLCommandFramer				mCommandFramer;
	TDCLChunkStream					mOutputStream;

	mutable std::mutex				mStateMutex;
	std::mutex						mJoinMutex;
	std::atomic<std::thread::id>	mThreadId;
	std::stop_source				mStopSource;
	std::jthread					mThread;
	std::stop_token					mWorkerStopToken;
	EState							mState;
	KUInt32							mType;
	KUInt32							mIdlePeriod;
	Boolean							mIsBusy;

	const KUInt16*					mNewtonName;
	KUInt32							mNewtonID;
	KUInt32							mManufacturer;
	KUInt32							mMachineType;
	KUInt32							mROMVersion;
	KUInt32							mROMStage;
	KUInt32							mRAMSize;
	KUInt32							mScreenHeight;
	KUInt32							mScreenWidth;
	KUInt32							mSystemUpdate;
	KUInt32							mNOSVersion;
	KUInt32							mInternalStoreSig;
	KUInt32							mVertScreenRes;
	KUInt32							mHoriScreenRes;
	KUInt32							mScreenDepth;
};

#endif
// _TDCLLINK_H
