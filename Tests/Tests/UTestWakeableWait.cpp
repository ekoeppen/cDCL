// ==============================
// File:			UTestWakeableWait.cpp
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
// The Original Code is UTestWakeableWait.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>

#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <stop_token>
#include <thread>

#if TARGET_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#endif

namespace
{
#if !TARGET_OS_WIN32
struct TPipePair
{
	int fds[2];
	TPipePair( void )
	{
		fds[0] = -1;
		fds[1] = -1;
		(void)::pipe( fds );
	}
	~TPipePair( void )
	{
		if ( fds[0] >= 0 )
		{
			(void)::close( fds[0] );
		}
		if ( fds[1] >= 0 )
		{
			(void)::close( fds[1] );
		}
	}
};
#endif
} // namespace

TEST_SUITE("WakeableWait")
{
#if TARGET_OS_WIN32
	TEST_CASE("Invalid socket returns kError")
	{
		int err = 0;
		CHECK( TDCLWakeableWait::PollNow( INVALID_SOCKET, TDCLWakeableWait::EDirection::kRead, &err ) ==
		       TDCLWakeableWait::EResult::kError );
	}
#else
	TEST_CASE("Invalid handle returns kError")
	{
		int err = 0;
		CHECK( TDCLWakeableWait::PollNow( -1, TDCLWakeableWait::EDirection::kRead, &err ) ==
		       TDCLWakeableWait::EResult::kError );
		std::stop_source ss;
		TDCLWakeableWait wait;
		CHECK( wait.WaitReady( -1, TDCLWakeableWait::EDirection::kRead, std::chrono::milliseconds( 50 ),
		                       ss.get_token(), &err ) == TDCLWakeableWait::EResult::kError );
	}

	TEST_CASE("PollNow distinguishes ready vs timeout")
	{
		TPipePair p;
		REQUIRE( p.fds[0] >= 0 );
		REQUIRE( p.fds[1] >= 0 );
		CHECK( TDCLWakeableWait::PollNow( p.fds[0], TDCLWakeableWait::EDirection::kRead ) ==
		       TDCLWakeableWait::EResult::kTimeout );
		CHECK( TDCLWakeableWait::PollNow( p.fds[1], TDCLWakeableWait::EDirection::kWrite ) ==
		       TDCLWakeableWait::EResult::kReady );
		const char b = 'x';
		REQUIRE( ::write( p.fds[1], &b, 1 ) == 1 );
		CHECK( TDCLWakeableWait::PollNow( p.fds[0], TDCLWakeableWait::EDirection::kRead ) ==
		       TDCLWakeableWait::EResult::kReady );
	}

	TEST_CASE("Pre-cancelled stop_token returns immediately without blocking")
	{
		TPipePair p;
		REQUIRE( p.fds[0] >= 0 );
		REQUIRE( p.fds[1] >= 0 );
		std::stop_source ss;
		ss.request_stop();
		TDCLWakeableWait wait;
		const auto t0 = std::chrono::steady_clock::now();
		CHECK( wait.WaitReady( p.fds[0], TDCLWakeableWait::EDirection::kRead, std::chrono::milliseconds( 2000 ),
		                       ss.get_token() ) == TDCLWakeableWait::EResult::kCancelled );
		const auto elapsed = std::chrono::steady_clock::now() - t0;
		CHECK( elapsed < std::chrono::milliseconds( 500 ) );
	}

	TEST_CASE("Timeout returns kTimeout")
	{
		TPipePair p;
		REQUIRE( p.fds[0] >= 0 );
		REQUIRE( p.fds[1] >= 0 );
		std::stop_source ss;
		TDCLWakeableWait wait;
		CHECK( wait.WaitReady( p.fds[0], TDCLWakeableWait::EDirection::kRead, std::chrono::milliseconds( 20 ),
		                       ss.get_token() ) == TDCLWakeableWait::EResult::kTimeout );
	}

	TEST_CASE("Concurrent request_stop wakes infinite WaitReady promptly")
	{
		TPipePair p;
		REQUIRE( p.fds[0] >= 0 );
		REQUIRE( p.fds[1] >= 0 );
		std::stop_source ss;
		TDCLWakeableWait wait;
		std::atomic<TDCLWakeableWait::EResult> result{ TDCLWakeableWait::EResult::kTimeout };

		std::jthread waiter( [ & ] {
			result.store( wait.WaitReady( p.fds[0], TDCLWakeableWait::EDirection::kRead,
			                              std::chrono::milliseconds( -1 ), ss.get_token() ) );
		} );

		std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
		ss.request_stop();
		waiter.join();
		CHECK( result.load() == TDCLWakeableWait::EResult::kCancelled );
	}

	TEST_CASE("Manual Trigger drains and continues waiting without spurious cancellation")
	{
		TPipePair p;
		REQUIRE( p.fds[0] >= 0 );
		REQUIRE( p.fds[1] >= 0 );
		std::stop_source ss;
		TDCLWakeableWait wait;
		REQUIRE( wait.EnsureOpen() == 0 );
		std::atomic<TDCLWakeableWait::EResult> result{ TDCLWakeableWait::EResult::kTimeout };

		std::jthread waiter( [ & ] {
			result.store( wait.WaitReady( p.fds[0], TDCLWakeableWait::EDirection::kRead,
			                              std::chrono::milliseconds( 2000 ), ss.get_token() ) );
		} );

		std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
		wait.Trigger();
		std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
		const char b = 'y';
		REQUIRE( ::write( p.fds[1], &b, 1 ) == 1 );
		waiter.join();
		CHECK( result.load() == TDCLWakeableWait::EResult::kReady );
	}
#endif
}
