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
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
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

static void EnsureWinsockInit()
{
	static bool sInitialized = false;
	if (!sInitialized) {
		WSADATA wsaData;
		(void) ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		sInitialized = true;
	}
}

TDCLWin32Socket::TDCLWin32Socket(
		IDCLThreads*	inThreadsIntf,
		unsigned short	inPort /* = kIANAAssignedPort */,
		KUInt32			inMaxPipeCount /* = kDefaultPipeCount */,
		long			inTimeout /* = kDefaultTimeout */ )
	:
		TDCLSocket( inThreadsIntf, inPort, inMaxPipeCount, inTimeout )
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
	EnsureWinsockInit();
	Boolean theResult = false;
	dcl_socket_t tmpsocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (tmpsocket != INVALID_SOCKET)
	{
		theResult = true;
		::closesocket( tmpsocket );
	}
	return theResult;
}

int
TDCLWin32Socket::MakeSocketPair( dcl_socket_t outPair[2] )
{
	EnsureWinsockInit();
	dcl_socket_t listener = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (listener == INVALID_SOCKET) {
		return -1;
	}

	struct sockaddr_in addr;
	(void) ::memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	addr.sin_port = 0; // ephemeral port

	if (::bind( listener, (struct sockaddr*) &addr, sizeof(addr) ) != 0) {
		::closesocket( listener );
		return -1;
	}

	int len = sizeof(addr);
	if (::getsockname( listener, (struct sockaddr*) &addr, &len ) != 0) {
		::closesocket( listener );
		return -1;
	}

	if (::listen( listener, 1 ) != 0) {
		::closesocket( listener );
		return -1;
	}

	outPair[0] = ::socket( AF_INET, SOCK_STREAM, 0 );
	if (outPair[0] == INVALID_SOCKET) {
		::closesocket( listener );
		return -1;
	}

	if (::connect( outPair[0], (struct sockaddr*) &addr, sizeof(addr) ) != 0) {
		::closesocket( outPair[0] );
		::closesocket( listener );
		return -1;
	}

	outPair[1] = ::accept( listener, NULL, NULL );
	if (outPair[1] == INVALID_SOCKET) {
		::closesocket( outPair[0] );
		::closesocket( listener );
		return -1;
	}

	::closesocket( listener );
	return 0;
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
TDCLWin32Socket::SetSocketTimeout( dcl_socket_t inSocket, long inTimeoutInSecs )
{
	DWORD timeoutMs = (DWORD) (inTimeoutInSecs * 1000);
	return ::setsockopt(
				inSocket,
				SOL_SOCKET,
				SO_RCVTIMEO,
				(const char*) &timeoutMs,
				sizeof(timeoutMs) );
}

int
TDCLWin32Socket::GetSocketError( void ) const
{
	return ::WSAGetLastError();
}

void
TDCLWin32Socket::ThrowSocketError( int inErr ) const
{
	switch (inErr)
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
			break;

		case WSAEFAULT:
		case WSAEACCES:
		default:
			throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLWin32Socket::ThrowSelectError( int inErr ) const
{
	switch (inErr)
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
	switch (inErr)
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
	switch (inErr)
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

void
TDCLWin32Socket::ThrowTimeoutError( int inErr ) const
{
	switch (inErr)
	{
		case WSAENOPROTOOPT:
			break;

		case WSAEBADF:
		case WSAENOTSOCK:
			throw DCLPlatformBadParamError( inErr );

		case WSAEINVAL:
		case WSAEISCONN:
			throw DCLPlatformBadStateError( inErr );

		case WSAENOBUFS:
			throw DCLPlatformLimitReachedError( inErr );

		case WSAEFAULT:
		default:
			throw DCLPlatformUnknownError( inErr );
	}
}