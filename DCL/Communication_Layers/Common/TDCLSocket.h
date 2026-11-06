// ==============================
// Fichier:			TDCLSocket.h
// Projet:			Desktop Connection Library
//
// Créé le:			29/3/2003
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
// The Original Code is TDCLSocket.h.
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
// $Id: TDCLSocket.h,v 1.7 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef _TDCLSOCKET_H
#define _TDCLSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLSyncCommLayer.h>

#if TARGET_OS_WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef SOCKET dcl_socket_t;
	#define kInvalidSocket INVALID_SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	typedef int dcl_socket_t;
	#define kInvalidSocket ((dcl_socket_t) -1)
#endif

class TDCLSocket
	:
		public TDCLSyncCommLayer
{
public:
	enum {
		kMaxSocketBackLog		= 128,
		kIANAAssignedPort		= 3679,
		kSocketDefaultTimeout	= 30
	};

	TDCLSocket(
			IDCLThreads*	inThreadsIntf,
			unsigned short	inPort = kIANAAssignedPort,
			KUInt32			inMaxPipeCount = kDefaultPipeCount,
			long			inTimeout = kDefaultTimeout );

	virtual ~TDCLSocket( void );

	void				Destroy( void );

	virtual void		SetTimeout( long inTimeout ) override;
	virtual long		GetTimeout( void ) override;
	virtual void		Disconnected( TDCLPipe* inPipe ) override;

	inline dcl_socket_t	GetServerSocket( void ) const
	{
		return mServerSocket;
	}

	inline unsigned short	GetPort( void ) const
	{
		return mPort;
	}

protected:
	virtual void		DoStartListening( void ) override;
	virtual void		DoStopListening( void ) override;
	virtual Boolean		WaitForIncomingRequest( void ) override;
	virtual TDCLPipe*	DoAccept( void ) override;
	virtual void		DoRefuse( void ) override;

	virtual dcl_socket_t	SocketBindAndListen( void );
	virtual TDCLPipe*	CreateSocketPipe( dcl_socket_t inSocket, long inTimeout );

	virtual int			MakeSocketPair( dcl_socket_t outPair[2] ) = 0;
	virtual void		CloseSocket( dcl_socket_t inSocket ) = 0;
	virtual int			SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking ) = 0;
	virtual int			SetSocketTimeout( dcl_socket_t inSocket, long inTimeoutInSecs ) = 0;
	virtual int			GetSocketError( void ) const = 0;
	virtual void		ThrowSocketError( int inErr ) const = 0;
	virtual void		ThrowSelectError( int inErr ) const = 0;
	virtual void		ThrowRecvError( int inErr ) const = 0;
	virtual void		ThrowSendError( int inErr ) const = 0;
	virtual void		ThrowTimeoutError( int inErr ) const = 0;

	class TSocketPipe;
	friend class TSocketPipe;

	class TSocketPipe
		:
			public TSyncPipe
	{
	public:
		friend class TDCLSocket;

		TSocketPipe(
				TDCLSocket*		inCommLayer,
				dcl_socket_t	inClientSocket,
				long			inTimeout );

		virtual ~TSocketPipe( void );

		virtual Boolean	BytesAvailable( void ) override;
		virtual long	GetTimeout( void ) override;
		virtual void	SetTimeout( long inTimeout ) override;
		virtual Boolean	WaitForIncomingData( void ) override;
		virtual void	DoDisconnect( void ) override;
		virtual void	Read( void* outBuffer, KUInt32* ioCount ) override;
		virtual void	Write( const void* inBuffer, KUInt32* ioCount ) override;

		inline dcl_socket_t	GetClientSocket( void ) const
		{
			return mClientSocket;
		}

	private:
		TDCLSocket*		mSocketCommLayer;
		dcl_socket_t	mClientSocket;
		long			mTimeout;
		dcl_socket_t	mPublicPairMember;
		dcl_socket_t	mPrivatePairMember;
		Boolean			mClientSocketIsClosed;
	};

private:
	TDCLSocket( const TDCLSocket& inCopy );
	TDCLSocket& operator = ( const TDCLSocket& inCopy );

	dcl_socket_t	mServerSocket;
	KUInt32			mCurrentPipeCount;
	KUInt32			mBackLogSize;
	long			mTimeout;
	dcl_socket_t	mPublicPairMember;
	dcl_socket_t	mPrivatePairMember;
	Boolean			mSocketsAreClosed;
	unsigned short	mPort;
};

#endif // _TDCLSOCKET_H