// ==============================
// Fichier:			TDCLBSDSocket.h
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
// The Original Code is TDCLBSDSocket.h.
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
// $Id: TDCLBSDSocket.h,v 1.9 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef __TDCLBSDSOCKET__
#define __TDCLBSDSOCKET__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>

#if !TARGET_OS_COMPAT_POSIX
#error TDCLBSDSocket requires POSIX compatibility
#endif

class TDCLBSDSocket : public TDCLSocket
{
public:
	enum
	{
		kBSDSocketMaxBackLogSize = kMaxSocketBackLog,
		kIANAAssignedPort = TDCLSocket::kIANAAssignedPort,
		kBSDSocketDefaultTimeout = kSocketDefaultTimeout
	};

	TDCLBSDSocket( IDCLThreads* inThreadsIntf, unsigned short inPort = kIANAAssignedPort,
	               KUInt32 inMaxPipeCount = kDefaultPipeCount, long inTimeout = kDefaultTimeout );

	virtual ~TDCLBSDSocket( void );

	static Boolean IsAvailable( void );

protected:
	virtual int MakeSocketPair( dcl_socket_t outPair[2] ) override;
	virtual void CloseSocket( dcl_socket_t inSocket ) override;
	virtual int SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking ) override;
	virtual int SetSocketTimeout( dcl_socket_t inSocket, long inTimeoutInSecs ) override;
	virtual int GetSocketError( void ) const override;
	virtual void ThrowSocketError( int inErr ) const override;
	virtual void ThrowSelectError( int inErr ) const override;
	virtual void ThrowRecvError( int inErr ) const override;
	virtual void ThrowSendError( int inErr ) const override;
	virtual void ThrowTimeoutError( int inErr ) const override;

private:
	TDCLBSDSocket( const TDCLBSDSocket& inCopy );
	TDCLBSDSocket& operator=( const TDCLBSDSocket& inCopy );
};

#endif // __TDCLBSDSOCKET__