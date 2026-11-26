// ==============================
// File:			UTestEventQueue.cpp
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
// The Original Code is UTestEventQueue.cpp.
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
#include <DCL/Interfaces/TDCLEventQueue.h>

#include <doctest/doctest.h>

#include <chrono>
#include <stop_token>
#include <thread>

TEST_SUITE("EventQueue")
{
	TEST_CASE("FIFO and bitmask filtering without losing skipped events")
	{
		TDCLEventQueue queue;
		queue.PostEvent( new TDCLEvent( TDCLEvent::kIdleAppCommand, TDCLEvent::kInstallPackage ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kBytesAvailable ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kIdleAppCommand, TDCLEvent::kSynchronize ) );

		TDCLEvent* ev1 = queue.WaitNextEvent( TDCLEvent::kLinkEvent, std::chrono::milliseconds::zero() );
		REQUIRE( ev1 != nullptr );
		CHECK( ev1->GetEventID() == TDCLEvent::kBytesAvailable );
		delete ev1;

		TDCLEvent* ev2 = queue.WaitNextEvent( TDCLEvent::kLinkEvent, std::chrono::milliseconds::zero() );
		CHECK( ev2 == nullptr );

		TDCLEvent* ev3 = queue.WaitNextEvent( TDCLEvent::kIdleAppCommand, std::chrono::milliseconds::zero() );
		REQUIRE( ev3 != nullptr );
		CHECK( ev3->GetEventID() == TDCLEvent::kInstallPackage );
		delete ev3;

		TDCLEvent* ev4 = queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds::zero() );
		REQUIRE( ev4 != nullptr );
		CHECK( ev4->GetEventID() == TDCLEvent::kSynchronize );
		delete ev4;

		CHECK( queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds::zero() ) == nullptr );
	}

	TEST_CASE("Finite timeout returns nil when no matching event arrives")
	{
		TDCLEventQueue queue;
		queue.PostEvent( new TDCLEvent( TDCLEvent::kIdleAppCommand, TDCLEvent::kInstallPackage ) );
		TDCLEvent* ev = queue.WaitNextEvent( TDCLEvent::kLinkEvent, std::chrono::milliseconds( 15 ) );
		CHECK( ev == nullptr );
	}

	TEST_CASE("Producer wakes blocked WaitNextEvent with matching event")
	{
		TDCLEventQueue queue;
		TDCLEvent* received = nullptr;
		std::jthread consumer( [ & ] {
			received = queue.WaitNextEvent( TDCLEvent::kLinkEvent, TDCLEventQueue::kForever );
		} );

		std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kIdleAppCommand, TDCLEvent::kInstallPackage ) );
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kDisconnectLink ) );
		consumer.join();

		REQUIRE( received != nullptr );
		CHECK( received->GetEventID() == TDCLEvent::kDisconnectLink );
		delete received;
	}

	TEST_CASE("stop_token wakes infinite WaitNextEvent cleanly")
	{
		TDCLEventQueue queue;
		std::stop_source ss;
		TDCLEvent* received = nullptr;
		std::jthread consumer( [ & ] {
			received = queue.WaitNextEvent( TDCLEvent::kAnyEventMask, TDCLEventQueue::kForever, ss.get_token() );
		} );

		std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
		(void)ss.request_stop();
		consumer.join();

		CHECK( received == nullptr );
	}

	TEST_CASE("RemoveEvents deletes only matching mask and data pointer")
	{
		TDCLEventQueue queue;
		int tagA = 1;
		int tagB = 2;
		queue.PostEvent( new TDCLEvent( TDCLEvent::kServerEvent, TDCLEvent::kIncomingRequest, &tagA ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kServerEvent, TDCLEvent::kWaitingConnection, &tagB ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kLinkEvent, TDCLEvent::kBytesAvailable, &tagA ) );
		queue.PostEvent( new TDCLEvent( TDCLEvent::kServerEvent, TDCLEvent::kIncomingRequest, &tagA ) );

		queue.RemoveEvents( TDCLEvent::kServerEventMask, &tagA );

		TDCLEvent* ev1 = queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds::zero() );
		REQUIRE( ev1 != nullptr );
		CHECK( ev1->GetEventID() == TDCLEvent::kWaitingConnection );
		CHECK( ev1->GetEventData() == &tagB );
		delete ev1;

		TDCLEvent* ev2 = queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds::zero() );
		REQUIRE( ev2 != nullptr );
		CHECK( ev2->GetEventID() == TDCLEvent::kBytesAvailable );
		CHECK( ev2->GetEventData() == &tagA );
		delete ev2;

		CHECK( queue.WaitNextEvent( TDCLEvent::kAnyEventMask, std::chrono::milliseconds::zero() ) == nullptr );
	}
}
