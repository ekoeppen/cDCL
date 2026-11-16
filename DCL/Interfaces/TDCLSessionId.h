// ==============================
// File:			TDCLSessionId.h
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
// The Original Code is TDCLSessionId.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2026 the Initial Developers. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLSESSIONID_H
#define _TDCLSESSIONID_H

#include <atomic>
#include <compare>
#include <cstdint>
#include <functional>

///
/// Monotonic process-unique identifier for an active transport session.
/// Zero represents an invalid/unassigned session.
///
struct TDCLSessionId
{
	std::uint64_t value = 0;

	constexpr bool IsValid( void ) const noexcept
	{
		return value != 0;
	}

	constexpr explicit operator bool( void ) const noexcept
	{
		return value != 0;
	}

	friend auto operator<=>( const TDCLSessionId&, const TDCLSessionId& ) = default;

	static TDCLSessionId Next( void ) noexcept
	{
		static std::atomic<std::uint64_t> sNextId{ 1 };
		return TDCLSessionId{ sNextId.fetch_add( 1, std::memory_order_relaxed ) };
	}
};

namespace std
{
template <>
struct hash<TDCLSessionId>
{
	std::size_t operator()( const TDCLSessionId& inId ) const noexcept
	{
		return std::hash<std::uint64_t>{}( inId.value );
	}
};
} // namespace std

#endif
// _TDCLSESSIONID_H
