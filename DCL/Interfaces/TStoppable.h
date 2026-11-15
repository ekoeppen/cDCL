// ==============================
// File:			TStoppable.h
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
// The Original Code is TStoppable.h.
//
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
//
// ***** END LICENSE BLOCK *****

#ifndef _TSTOPPABLE_H
#define _TSTOPPABLE_H

#include <mutex>
#include <stop_token>

///
/// Cooperative cancellation mixin based on std::stop_source / std::stop_token.
///
class TStoppable
{
public:
	TStoppable( void ) = default;
	virtual ~TStoppable( void ) = default;

	TStoppable( const TStoppable& ) = delete;
	TStoppable& operator=( const TStoppable& ) = delete;

	///
	/// Requests cooperative stop.
	///
	/// \return \c true if this call triggered the stop, \c false if already requested.
	///
	virtual bool
	RequestStop( void )
	{
		std::stop_source theSource;
		{
			std::lock_guard<std::mutex> lock( mStopMutex );
			theSource = mStopSource;
		}
		return theSource.request_stop();
	}

	///
	/// Returns whether cooperative stop has been requested.
	///
	inline bool
	StopRequested( void ) const noexcept
	{
		std::lock_guard<std::mutex> lock( mStopMutex );
		return mStopSource.stop_requested();
	}

	///
	/// Returns a std::stop_token observing this stop source.
	///
	inline std::stop_token
	GetStopToken( void ) const noexcept
	{
		std::lock_guard<std::mutex> lock( mStopMutex );
		return mStopSource.get_token();
	}

protected:
	///
	/// Resets the stop source for a new active session.
	///
	inline void
	ResetStopSource( void )
	{
		std::lock_guard<std::mutex> lock( mStopMutex );
		mStopSource = std::stop_source{};
	}

private:
	mutable std::mutex mStopMutex;
	std::stop_source mStopSource;
};

///
/// RAII helper that forwards stop requests from a parent std::stop_token
/// to a target TStoppable for the lifetime of the guard.
///
class TStopForwarder
{
public:
	TStopForwarder( const std::stop_token& inParentToken, TStoppable& inTarget )
	    : mCallback( inParentToken, TCallback{ &inTarget } )
	{
		if ( inParentToken.stop_requested() )
		{
			(void)inTarget.RequestStop();
		}
	}

	~TStopForwarder( void ) = default;

	TStopForwarder( const TStopForwarder& ) = delete;
	TStopForwarder& operator=( const TStopForwarder& ) = delete;

private:
	struct TCallback
	{
		TStoppable* mTarget;
		void
		operator()( void ) const
		{
			(void)mTarget->RequestStop();
		}
	};

	std::stop_callback<TCallback> mCallback;
};

#endif
// _TSTOPPABLE_H
