// ==============================
// Fichier:			TDCLBSDSocket.cpp
// Projet:			Desktop Connection Library
//
// Créé le:			29/07/2002
// Tabulation:		4 espaces
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
// The Original Code is TDCLBSDSocket.cp.
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
// $Id: TDCLBSDSocket.cp,v 1.13 2004/11/24 18:11:12 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <fcntl.h>

#include <K/Tests/KDebug.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>
#include <DCL/Exceptions/Errors/TDCLBadStateError.h>
#include <DCL/Exceptions/Errors/TDCLLimitReachedError.h>
#include <DCL/Exceptions/Errors/TDCLMemError.h>
#include <DCL/Exceptions/Errors/TDCLUnknownError.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLEOFException.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLTimeoutException.h>

TDCLBSDSocket::TDCLBSDSocket( IDCLThreads* inThreadsIntf, unsigned short inPort /* = kIANAAssignedPort */,
                              KUInt32 inMaxPipeCount /* = kDefaultPipeCount */, long inTimeout /* = kDefaultTimeout */ )
    : TDCLSocket( inThreadsIntf, inPort, inMaxPipeCount, inTimeout )
{
}

TDCLBSDSocket::~TDCLBSDSocket( void )
{
	Destroy();
}

Boolean
TDCLBSDSocket::IsAvailable( void )
{
	Boolean theResult = false;
	int tmpsocket = ::socket( AF_INET, SOCK_STREAM, 0 );
	if ( tmpsocket >= 0 )
	{
		theResult = true;
		(void)::close( tmpsocket );
	}
	return theResult;
}

int
TDCLBSDSocket::MakeSocketPair( dcl_socket_t outPair[2] )
{
	return ::socketpair( AF_UNIX, SOCK_STREAM, PF_UNSPEC, outPair );
}

void
TDCLBSDSocket::CloseSocket( dcl_socket_t inSocket )
{
	(void)::close( inSocket );
}

int
TDCLBSDSocket::SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking )
{
	int flags = ::fcntl( inSocket, F_GETFL, 0 );
	if ( flags < 0 )
	{
		return flags;
	}
	if ( inNonBlocking )
	{
		flags |= O_NONBLOCK;
	}
	else
	{
		flags &= ~O_NONBLOCK;
	}
	return ::fcntl( inSocket, F_SETFL, flags );
}

int
TDCLBSDSocket::SetSocketTimeout( dcl_socket_t inSocket, long inTimeoutInSecs )
{
	struct timeval theTimeoutStructure;
	theTimeoutStructure.tv_sec = inTimeoutInSecs;
	theTimeoutStructure.tv_usec = 0;
	int theResult = ::setsockopt( inSocket, SOL_SOCKET, SO_RCVTIMEO, &theTimeoutStructure, sizeof( theTimeoutStructure ) );
	if ( theResult < 0 )
	{
		return theResult;
	}
	return ::setsockopt( inSocket, SOL_SOCKET, SO_SNDTIMEO, &theTimeoutStructure, sizeof( theTimeoutStructure ) );
}

int
TDCLBSDSocket::GetSocketError( void ) const
{
	return errno;
}

void
TDCLBSDSocket::ThrowSocketError( int inErr ) const
{
	switch ( inErr )
	{
	case EDESTADDRREQ:
	case EADDRNOTAVAIL:
	case EAFNOSUPPORT:
	case EBADF:
	case ENOTSOCK:
	case EOPNOTSUPP:
	case EINVAL:
	case ENAMETOOLONG:
		throw DCLPlatformBadParamError( inErr );

	case EISCONN:
		throw DCLPlatformBadStateError( inErr );

	case EPROTONOSUPPORT:
	case EPROTOTYPE:
		throw DCLPlatformNotAvailable( inErr );

	case EMFILE:
	case ENFILE:
	case ENOBUFS:
#ifdef ENOSR
	case ENOSR:
#endif
		throw DCLPlatformLimitReachedError( inErr );

	case EADDRINUSE:
		KDEBUG( "Address is in use" );
		throw DCLPlatformUnknownError( inErr );

	case EFAULT:
	case EACCES:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLBSDSocket::ThrowSelectError( int inErr ) const
{
	switch ( inErr )
	{
	case EINVAL:
		throw DCLPlatformBadParamError( inErr );

	case EBADF:
	case EINTR:
		break;

	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLBSDSocket::ThrowRecvError( int inErr ) const
{
	switch ( inErr )
	{
	case EINTR:
	case EAGAIN:
#if ( EWOULDBLOCK != EAGAIN )
	case EWOULDBLOCK:
#endif
		KDEBUG( "Looping in socket code!" );
		break;

	case ECONNRESET:
		throw DCLPlatformEOF( inErr );

	case ENOTCONN:
		throw DCLPlatformBadStateError( inErr );

	case EBADF:
	case EINVAL:
	case ENOTSOCK:
		throw DCLPlatformBadParamError( inErr );

	case EOPNOTSUPP:
		throw DCLPlatformNotAvailable( inErr );

	case ETIMEDOUT:
		throw DCLPlatformTimeout( inErr );

	case ENOMEM:
		throw DCLPlatformMemError( inErr );

	case ENOBUFS:
#ifdef ENOSR
	case ENOSR:
#endif
		throw DCLPlatformLimitReachedError( inErr );

	case EFAULT:
	case EIO:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLBSDSocket::ThrowSendError( int inErr ) const
{
	switch ( inErr )
	{
	case EINTR:
	case EAGAIN:
#if ( EWOULDBLOCK != EAGAIN )
	case EWOULDBLOCK:
#endif
		KDEBUG( "Looping in socket code!" );
		break;

	case ECONNRESET:
	case EPIPE:
		throw DCLPlatformEOF( inErr );

	case ENOTCONN:
		throw DCLPlatformBadStateError( inErr );

	case EBADF:
	case EINVAL:
	case ENOTSOCK:
		throw DCLPlatformBadParamError( inErr );

	case EOPNOTSUPP:
		throw DCLPlatformNotAvailable( inErr );

	case ETIMEDOUT:
		throw DCLPlatformTimeout( inErr );

	case ENOMEM:
		throw DCLPlatformMemError( inErr );

	case ENOBUFS:
#ifdef ENOSR
	case ENOSR:
#endif
		throw DCLPlatformLimitReachedError( inErr );

	case EFAULT:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}

void
TDCLBSDSocket::ThrowTimeoutError( int inErr ) const
{
	switch ( inErr )
	{
	case ENOPROTOOPT:
		break;

	case EBADF:
	case EDOM:
	case ENOTSOCK:
		throw DCLPlatformBadParamError( inErr );

	case EINVAL:
	case EISCONN:
		throw DCLPlatformBadStateError( inErr );

	case ENOMEM:
		throw DCLPlatformMemError( inErr );

	case ENOBUFS:
#ifdef ENOSR
	case ENOSR:
#endif
		throw DCLPlatformLimitReachedError( inErr );

	case EFAULT:
	default:
		throw DCLPlatformUnknownError( inErr );
	}
}