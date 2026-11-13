// ==============================
// Fichier:			TDCLWin32Socket.h
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
// The Original Code is TDCLWin32Socket.h.
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

#ifndef __TDCLWIN32SOCKET__
#define __TDCLWIN32SOCKET__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLSocket.h>

#if !TARGET_OS_WIN32
#error TDCLWin32Socket requires Win32
#endif

class TDCLWin32Socket : public TDCLSocket
{
public:
	enum
	{
		kWin32SocketMaxBackLogSize = kMaxSocketBackLog,
		kIANAAssignedPort = TDCLSocket::kIANAAssignedPort,
		kWin32SocketDefaultTimeout = kSocketDefaultTimeout
	};

	TDCLWin32Socket( IDCLThreads* inThreadsIntf, unsigned short inPort = kIANAAssignedPort,
	                 KUInt32 inMaxPipeCount = kDefaultPipeCount, long inTimeout = kDefaultTimeout );

	virtual ~TDCLWin32Socket( void );

	static Boolean IsAvailable( void );

protected:
	virtual int SetListenSocketOptions( dcl_socket_t inSocket ) override;
	virtual void CloseSocket( dcl_socket_t inSocket ) override;
	virtual int SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking ) override;
	virtual int GetSocketError( void ) const override;
	virtual bool IsWouldBlockError( int inErr ) const override;
	virtual bool IsInterruptedError( int inErr ) const override;
	virtual bool IsAbortedAcceptError( int inErr ) const override;
	virtual void ThrowSocketError( int inErr ) const override;
	virtual void ThrowSelectError( int inErr ) const override;
	virtual void ThrowRecvError( int inErr ) const override;
	virtual void ThrowSendError( int inErr ) const override;

private:
	TDCLWin32Socket( const TDCLWin32Socket& inCopy );
	TDCLWin32Socket& operator=( const TDCLWin32Socket& inCopy );
};

#endif // __TDCLWIN32SOCKET__
