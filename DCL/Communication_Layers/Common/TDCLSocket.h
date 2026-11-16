// ==============================
// File:			TDCLSocket.h
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
// The Original Code is TDCLSocket.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacik
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacik <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****

#ifndef _TDCLSOCKET_H
#define _TDCLSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <DCL/Interfaces/TDCLEventBus.h>

// ISO C++
#include <atomic>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

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
#define kInvalidSocket ( (dcl_socket_t)-1 )
#endif

///
/// TCP socket communication layer using push-based per-session worker threads
/// registered on TDCLSessionBus (no TDCLPipe or sidecar watcher threads).
///
class TDCLSocket : public TDCLCommLayer
{
public:
	enum
	{
		kMaxSocketBackLog = 128,
		kIANAAssignedPort = 3679,
		kSocketDefaultTimeout = 30
	};

	struct TSocketOps
	{
		void ( *closeSocket )( dcl_socket_t );
		int ( *getSocketError )( void );
		bool ( *isWouldBlock )( int );
		bool ( *isInterrupted )( int );
		std::exception_ptr ( *makeRecvError )( int );
		std::exception_ptr ( *makeSendError )( int );
	};

	class TSocketSession;

	TDCLSocket( unsigned short inPort = kIANAAssignedPort,
	            KUInt32 inMaxPipeCount = kDefaultPipeCount,
	            long inTimeout = kDefaultTimeout );

	virtual ~TDCLSocket( void );

	void Destroy( void );

	virtual void StopListening( void ) override;
	virtual TDCLSessionId AcceptSession( void ) override;
	virtual void Refuse( void ) override;
	virtual bool IsCurrentThread( void ) const noexcept override;
	virtual void SetTimeout( long inTimeout ) override;
	virtual long GetTimeout( void ) override;

	inline dcl_socket_t GetServerSocket( void ) const { return mServerSocket; }
	inline unsigned short GetPort( void ) const { return mPort; }

protected:
	virtual void DoStartListening( void ) override;
	virtual void DoStopListening( void ) override;

	virtual Boolean WaitForIncomingRequest( void );
	virtual dcl_socket_t SocketBindAndListen( void );
	virtual int SetListenSocketOptions( dcl_socket_t inSocket );

	/// Creates a push-based TSocketSession for an already-connected socket
	/// (used by both AcceptSession and outbound client sockets like TDCLClientSocket).
	TDCLSessionId CreateSessionFromSocket( dcl_socket_t inSocket );

	/// Starts/stops the listener worker thread (for subclasses such as TDCLClientSocket).
	void StartListenThread( void );
	void ClearRequestPending( void );

	/// Stops and joins all active sessions created by this socket layer.
	void CloseAllSessions( void );

	virtual TSocketOps GetSocketOps( void ) const = 0;
	virtual void CloseSocket( dcl_socket_t inSocket ) = 0;
	virtual int SetNonBlocking( dcl_socket_t inSocket, bool inNonBlocking ) = 0;
	virtual int GetSocketError( void ) const = 0;
	virtual bool IsWouldBlockError( int inErr ) const = 0;
	virtual bool IsInterruptedError( int inErr ) const = 0;
	virtual bool IsAbortedAcceptError( int inErr ) const = 0;
	virtual void ThrowSocketError( int inErr ) const = 0;
	virtual void ThrowSelectError( int inErr ) const = 0;

private:
	TDCLSocket( const TDCLSocket& inCopy ) = delete;
	TDCLSocket& operator=( const TDCLSocket& inCopy ) = delete;

	void ListenWorkerEntry( std::stop_token inStopToken );
	void CloseListenSockets( void );
	void PruneFinishedSessionsLocked( void );

	dcl_socket_t mServerSocket;
	std::shared_ptr<std::atomic<KUInt32>> mActiveSessionCount;
	KUInt32 mBackLogSize;
	long mTimeout;
	TDCLWakeableWait mWakeWait;
	Boolean mSocketsAreClosed;
	unsigned short mPort;

	std::mutex mListenMutex;
	std::condition_variable mListenCV;
	bool mRequestPending;
	std::jthread mListenThread;
	std::atomic<std::thread::id> mListenThreadId;

	std::mutex mSessionsMutex;
	std::vector<std::shared_ptr<TSocketSession>> mSessions;
};

#endif // _TDCLSOCKET_H
