// ==============================
// File:			CommLayerStartStopTests.cpp
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
// The Original Code is CommLayerStartStopTests.cpp.
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

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Server/TDCLServer.h>

#if TARGET_OS_WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
typedef TDCLWin32Socket TDCLPlatformSocket;
#else
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
typedef TDCLBSDSocket TDCLPlatformSocket;
#endif

#include "TTestApplication.h"

#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

#if TARGET_OS_COMPAT_POSIX || TARGET_OS_WIN32
namespace
{
class TSASTApp : public TTestApplication
{
public:
	TSASTApp( void )
		: TTestApplication()
	{
	}

	virtual void WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer ) override
	{
		TDCLLogApplication::WaitConnection( inServer, inLayer );
		++mWaitCount;
		inServer->Stop();
	}

	virtual void ServerIsDown( TDCLServer* inServer ) override
	{
		TDCLLogApplication::ServerIsDown( inServer );
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mDownCount;
			mDown = true;
		}
		mCV.notify_all();
	}

	bool WaitUntilServerIsDownFor( std::chrono::milliseconds inTimeout )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		const bool ok = mCV.wait_for( lock, inTimeout, [this] { return mDown; } );
		mDown = false;
		return ok;
	}

	std::atomic<int> mWaitCount{ 0 };
	std::atomic<int> mDownCount{ 0 };
	std::atomic<int> mExceptionCount{ 0 };

protected:
	virtual void ExceptionOccurred( void ) override
	{
		++mExceptionCount;
		{
			std::lock_guard<std::mutex> lock( mMutex );
			mDown = true;
		}
		mCV.notify_all();
	}

private:
	std::mutex mMutex;
	std::condition_variable mCV;
	bool mDown = false;
};

class TSASTServer : public TDCLServer
{
public:
	TSASTServer( TDCLApplication* inApplication, TDCLCommLayer* inCommLayer )
		: TDCLServer( inApplication ), mCommLayer( inCommLayer )
	{
	}

	std::atomic<int> mStartCount{ 0 };
	std::atomic<int> mStopCount{ 0 };

protected:
	virtual void DoStartListening( void ) override
	{
		++mStartCount;
		mCommLayer->StartListening( this );
	}

	virtual void DoStop( void ) override
	{
		++mStopCount;
		mCommLayer->StopListening();
	}

	virtual void DoKill( void ) override
	{
		mCommLayer->StopListening();
	}

	virtual EState HandleIncomingConnection( TDCLCommLayer* /* inCommLayer */ ) override
	{
		return kRunning;
	}

	virtual EState HandleLinkDisconnection( TDCLLink* /* inLink */ ) override
	{
		return kRunning;
	}

	virtual void DoIdle( void ) override
	{
		mCommLayer->Idle();
	}

private:
	TDCLCommLayer* mCommLayer;
};
} // namespace

TEST_SUITE("CommLayerStartStop")
{
	TEST_CASE("Platform socket starts and stops twice cleanly")
	{
		TSASTApp theApp;
		TDCLPlatformSocket theCommLayer( 0 /* ephemeral */ );
		TSASTServer theServer( &theApp, &theCommLayer );

		theServer.Start();
		REQUIRE( theApp.WaitUntilServerIsDownFor( std::chrono::milliseconds( 5000 ) ) );
		CHECK( theServer.Wait() );
		CHECK( theServer.GetState() == TDCLServer::kStopped );
		CHECK( theServer.mStartCount == 1 );
		CHECK( theApp.mWaitCount == 1 );
		CHECK( theServer.mStopCount == 1 );
		CHECK( theApp.mDownCount == 1 );
		CHECK( theApp.mExceptionCount == 0 );

		theServer.Start();
		REQUIRE( theApp.WaitUntilServerIsDownFor( std::chrono::milliseconds( 5000 ) ) );
		CHECK( theServer.Wait() );
		CHECK( theServer.GetState() == TDCLServer::kStopped );
		CHECK( theServer.mStartCount == 2 );
		CHECK( theApp.mWaitCount == 2 );
		CHECK( theServer.mStopCount == 2 );
		CHECK( theApp.mDownCount == 2 );
		CHECK( theApp.mExceptionCount == 0 );
	}
}
#endif
