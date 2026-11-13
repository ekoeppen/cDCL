// ==============================
// Fichier:			TDCLWin32Socket.cpp
// Projet:			Desktop Connection Library
//
// Created:			07/11/2026
// Tab width:		4 spaces
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
// The Original Code is TDCLWin32Socket.cpp.
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
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <K/Tests/KDebug.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

namespace
{
class TWinsockInit
{
public:
	TWinsockInit( void )
	{
		WSADATA wsaData;
		int theErr = ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
		if ( theErr != 0 )
		{
			throw DCLPlatformNotAvailable( theErr );
		}
	}

	~TWinsockInit( void )
	{
		(void)::WSACleanup();
	}
};
} // namespace

static void
EnsureWinsockInit( void )
{
	static TWinsockInit sWinsockInit;
}

TDCLWin32Socket::TDCLWin32Socket( IDCLThreads* inThreadsIntf, unsigned short inPort /* = kIANAAssignedPort */,
                                  KUInt32 inMaxPipeCount /* = kDefaultPipeCount */,
                                  long inTimeout /* = kDefaultTimeout */ )
    : TDCLSocket( inThreadsIntf, inPort, inMaxPipeCount, inTimeout )
{
	EnsureWinsockInit();
}

TDCLWin32Socket::~TDCLWin32Socket( void )
{
	Destroy();
}

Boolean
TDCLWin32Socket::IsAvailable( void )
{
	try
	{
		EnsureWinsockInit();
	}
	catch ( ... )
	{
		return false;
	}
	Boolean theResult = false;
	dcl_socket_t tmpsocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( tmpsocket != INVALID_SOCKET )
	{
		theResult = true;
		::closesocket( tmpsocket );
	}
	return theResult;
}

int
TDCLWin32Socket::SetListenSocketOptions( dcl_socket_t inSocket )
{
	// On Windows, SO_REUSEADDR allows another socket to bind the same port
	// while we are listening. Use SO_EXCLUSIVEADDRUSE so that a second
	// server fails with WSAEADDRINUSE instead of stealing connections.
	int theExclusiveFlag = 1;
	return ::setsockopt( inSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&theExclusiveFlag,
	                     sizeof( theExclusiveFlag ) );
}

void
TDCLWin32Socket::CloseSocket( dcl_socket_t inSocket )
{
	::closesocket( inSocket );
}

int
TDCLWin32Socket::SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking )
{
	u_long mode = inNonBlocking ? 1 : 0;
	return ::ioctlsocket( inSocket, FIONBIO, &mode );
}

int
TDCLWin32Socket::GetSocketError( void ) const
{
	return ::WSAGetLastError();
}

bool
TDCLWin32Socket::IsWouldBlockError( int inErr ) const
{
	return ( inErr == WSAEINTR ) || ( inErr == WSAEWOULDBLOCK );
}

bool
TDCLWin32Socket::IsInterruptedError( int inErr ) const
{
	return ( inErr == WSAEINTR );
}

bool
TDCLWin32Socket::IsAbortedAcceptError( int inErr ) const
{
	// The client closed the connection while it was queued.
	return ( inErr == WSAECONNRESET ) || ( inErr == WSAECONNABORTED );
}

void
TDCLWin32Socket::ThrowSocketError( int inErr ) const
{
	switch ( inErr )
	{
	case WSAEDESTADDRREQ:
	case WSAEADDRNOTAVAIL:
	case WSAEAFNOSUPPORT:
	case WSAEBADF:
	case WSAENOTSOCK:
	case WSAEOPNOTSUPP:
	case WSAEINVAL:
		throw DCLPlatformBadParamError( inErr );

	case WSAEISCONN:
		throw DCLPlatformBadStateError( inErr );

	case WSAEPROTONOSUPPORT:
	case WSAEPROTOTYPE:
		throw DCLPlatformNotAvailable( inErr );

	case WSAEMFILE:
	case WSAENOBUFS:
		throw DCLPlatformLimitReachedError( inErr );

	case WSAEADDRINUSE:
		KDEBUG( "Address is in use" );
		throw DCLPlatformUnknownError( inErr );

	case WSAEFAULT:
	case WSAEACCES:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLWin32Socket::ThrowSelectError( int inErr ) const
{
	switch ( inErr )
	{
	case WSAEINVAL:
		throw DCLPlatformBadParamError( inErr );

	case WSAEBADF:
	case WSAENOTSOCK:
	case WSAEINTR:
		break;

	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLWin32Socket::ThrowRecvError( int inErr ) const
{
	switch ( inErr )
	{
	case WSAEINTR:
	case WSAEWOULDBLOCK:
		KDEBUG( "Looping in socket code!" );
		break;

	case WSAECONNRESET:
		throw DCLPlatformEOF( inErr );

	case WSAENOTCONN:
		throw DCLPlatformBadStateError( inErr );

	case WSAEBADF:
	case WSAEINVAL:
	case WSAENOTSOCK:
		throw DCLPlatformBadParamError( inErr );

	case WSAEOPNOTSUPP:
		throw DCLPlatformNotAvailable( inErr );

	case WSAETIMEDOUT:
		throw DCLPlatformTimeout( inErr );

	case WSAENOBUFS:
		throw DCLPlatformLimitReachedError( inErr );

	case WSAEACCES:
	case WSAEFAULT:
	case WSAENETDOWN:
	case WSAENETUNREACH:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLWin32Socket::ThrowSendError( int inErr ) const
{
	switch ( inErr )
	{
	case WSAEINTR:
	case WSAEWOULDBLOCK:
		KDEBUG( "Looping in socket code!" );
		break;

	case WSAECONNRESET:
	case WSAESHUTDOWN:
		throw DCLPlatformEOF( inErr );

	case WSAENOTCONN:
		throw DCLPlatformBadStateError( inErr );

	case WSAEBADF:
	case WSAEMSGSIZE:
	case WSAEINVAL:
	case WSAENOTSOCK:
		throw DCLPlatformBadParamError( inErr );

	case WSAEOPNOTSUPP:
		throw DCLPlatformNotAvailable( inErr );

	case WSAETIMEDOUT:
		throw DCLPlatformTimeout( inErr );

	case WSAENOBUFS:
		throw DCLPlatformLimitReachedError( inErr );

	case WSAEACCES:
	case WSAEFAULT:
	case WSAENETDOWN:
	case WSAENETUNREACH:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}
