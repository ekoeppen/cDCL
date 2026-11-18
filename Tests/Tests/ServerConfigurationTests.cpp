// ==============================
// File:			ServerConfigurationTests.cpp
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
// The Original Code is ServerConfigurationTests.cpp.
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

#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Communication_Layers/TDCLPipe.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Server/TDCLOneLinkServer.h>
#include <DCL/Server/TDCLSimpleServer.h>

#include "TTestApplication.h"
#include "TTestDockLink.h"

#include <doctest/doctest.h>

#include <condition_variable>
#include <memory>
#include <mutex>

namespace
{

class TConfigTestCommLayer : public TDCLCommLayer
{
public:
	TConfigTestCommLayer( void )
		: TDCLCommLayer( false, 0 ), mStartCount( 0 ), mStopCount( 0 ), mTimeout( kDefaultTimeout )
	{
	}

	virtual std::unique_ptr<TDCLPipe> Accept( void ) override { return nullptr; }
	virtual void Refuse( void ) override {}
	virtual void SetTimeout( long inTimeout ) override { mTimeout = inTimeout; }
	virtual long GetTimeout( void ) override { return mTimeout; }
	virtual void Disconnected( TDCLPipe* /* inPipe */ ) override {}

	int GetStartCount( void ) const { return mStartCount; }
	int GetStopCount( void ) const { return mStopCount; }

protected:
	virtual void DoStartListening( void ) override
	{
		++mStartCount;
	}

	virtual void DoStopListening( void ) override
	{
		++mStopCount;
	}

private:
	int mStartCount;
	int mStopCount;
	long mTimeout;
};

class TConfigTestApplication : public TTestApplication
{
public:
	TConfigTestApplication( void )
		: TTestApplication(),
		  mWaitingCount( 0 ),
		  mDownCount( 0 )
	{
	}

	virtual void WaitConnection( TDCLServer* /* inServer */, TDCLCommLayer* /* inLayer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mWaitingCount;
		}
		mCV.notify_all();
	}

	virtual void ServerIsDown( TDCLServer* /* inServer */ ) override
	{
		{
			std::lock_guard<std::mutex> lock( mMutex );
			++mDownCount;
		}
		mCV.notify_all();
	}

	void WaitForListening( int inExpectedCount = 1 )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [&]() { return mWaitingCount >= inExpectedCount; } );
		mWaitingCount -= inExpectedCount;
	}

	void WaitUntilServerIsDown( void )
	{
		std::unique_lock<std::mutex> lock( mMutex );
		mCV.wait( lock, [&]() { return mDownCount >= 1; } );
		--mDownCount;
	}

private:
	std::mutex mMutex;
	std::condition_variable mCV;
	int mWaitingCount;
	int mDownCount;
};

template <typename Fn>
static bool
ExpectBadState( Fn&& inFn )
{
	try
	{
		inFn();
		return false;
	}
	catch ( const TDCLBadStateError& )
	{
		return true;
	}
	catch ( const TDCLException& anException )
	{
		return anException.GetExCode() == TDCLException::exBadStateError;
	}
}

template <typename Fn>
static bool
ExpectBadParam( Fn&& inFn )
{
	try
	{
		inFn();
		return false;
	}
	catch ( const TDCLBadParamError& )
	{
		return true;
	}
	catch ( const TDCLException& anException )
	{
		return anException.GetExCode() == TDCLException::exBadParamError;
	}
}

} // namespace

TEST_SUITE("ServerConfiguration")
{
	TEST_CASE("TDCLSimpleServer SetLink lifecycle constraints")
	{
		TConfigTestApplication theApp;
		auto layer = std::make_unique<TConfigTestCommLayer>();
		auto* layerPtr = layer.get();
		TDCLSimpleServer theServer( &theApp, std::move( layer ) );

		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		CHECK( layerPtr->GetStartCount() == 0 );

		theServer.Start();
		theApp.WaitForListening();
		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );

		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		(void)theServer.Wait();
		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );

		theServer.Start();
		theApp.WaitForListening();
		theServer.Kill();
		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );
	}

	TEST_CASE("TDCLOneLinkServer SetLink and AddCommLayer lifecycle constraints")
	{
		TConfigTestApplication theApp;
		TDCLOneLinkServer theServer( &theApp );

		CHECK( ExpectBadParam( [&]() { (void)theServer.AddCommLayer( nullptr ); } ) );

		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		(void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) );
		auto layer1 = std::make_unique<TConfigTestCommLayer>();
		auto layer2 = std::make_unique<TConfigTestCommLayer>();
		auto* layer1Ptr = layer1.get();
		auto* layer2Ptr = layer2.get();
		(void)theServer.AddCommLayer( std::move( layer1 ) );
		(void)theServer.AddCommLayer( std::move( layer2 ) );
		CHECK( layer1Ptr->GetStartCount() == 0 );
		CHECK( layer2Ptr->GetStartCount() == 0 );

		theServer.Start();
		theApp.WaitForListening( 2 );
		CHECK( layer1Ptr->GetStartCount() == 1 );
		CHECK( layer2Ptr->GetStartCount() == 1 );

		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );
		CHECK( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ) );

		theServer.Stop();
		theApp.WaitUntilServerIsDown();
		(void)theServer.Wait();
		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );
		CHECK( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ) );

		theServer.Start();
		theApp.WaitForListening( 2 );
		CHECK( layer1Ptr->GetStartCount() == 2 );
		CHECK( layer2Ptr->GetStartCount() == 2 );
		theServer.Kill();
		CHECK( ExpectBadState( [&]() { (void)theServer.SetLink( std::make_unique<TTestDockLink>( &theApp ) ); } ) );
		CHECK( ExpectBadState( [&]() { (void)theServer.AddCommLayer( std::make_unique<TConfigTestCommLayer>() ); } ) );
	}
}
