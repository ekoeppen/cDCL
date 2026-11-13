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

#include <DCL/Headers/DCLDefinitions.h>

#include <stop_token>
#include <utility>

///
/// Mixin for objects that can receive a cooperative stop request
/// via std::stop_source / std::stop_token.
///
class TStoppable
{
public:
	///
	/// Destructor.
	///
	virtual ~TStoppable( void ) = default;

	///
	/// Requests a cooperative stop of the object.
	///
	/// \return \c true if this call triggered the stop,
	///			\c false if a stop had already been requested.
	///
	virtual bool
	RequestStop( void )
	{
		return mStopSource.request_stop();
	}

	///
	/// Returns the stop token.
	///
	/// \return the associated std::stop_token.
	///
	inline std::stop_token
	GetStopToken( void ) const noexcept
	{
		return mStopSource.get_token();
	}

	///
	/// Returns whether a stop has been requested.
	///
	/// \return \c true if RequestStop() has been called.
	///
	inline bool
	StopRequested( void ) const noexcept
	{
		return mStopSource.stop_requested();
	}

protected:
	///
	/// Resets the stop source (e.g. when restarting).
	///
	inline void
	ResetStopSource( void )
	{
		mStopSource = std::stop_source{};
	}

private:
	std::stop_source mStopSource; ///< Stop token source.
};

///
/// RAII forwarder of a stop request from a parent std::stop_token to
/// a target TStoppable object.
///
class TStopForwarder
{
public:
	TStopForwarder( std::stop_token inParentToken, TStoppable& inTarget )
	    : mCallback( std::move( inParentToken ), TForward{ &inTarget } )
	{
	}

	TStopForwarder( const TStopForwarder& ) = delete;
	TStopForwarder& operator=( const TStopForwarder& ) = delete;

private:
	struct TForward
	{
		TStoppable* mTarget;
		void
		operator()( void ) const
		{
			mTarget->RequestStop();
		}
	};

	std::stop_callback<TForward> mCallback;
};

#endif
// _TSTOPPABLE_H
