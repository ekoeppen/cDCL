// ==============================
// File:			TDCLServer.h
// Project:			Desktop Connection Library
//
// Created:			14/01/2002
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
// The Original Code is TDCLServer.h.
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
// $Id: TDCLServer.h,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef __TDCLSERVER__
#define __TDCLSERVER__

#include <DCL/Headers/DCLDefinitions.h>

// C++
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLEventQueue.h>

class TDCLLink;

///
/// Abstract base class for servers.
///
/// A server listens on one or more interfaces (pipes) and associates them with
/// a link when the Newton connects.
/// It can then create more interfaces, kill previous interfaces, or tell them
/// to stop listening. All of this depends on the subclass.
///
/// Access to interfaces and links must be done by locking/unlocking the lists.
/// Locking the lists prevents the server from creating new links or new
/// interfaces, so you should unlock them as soon as possible.
///
/// In addition, the server includes a mutex semaphore to protect access to the
/// Start/Stop/Kill methods.
///
/// The server is represented by a thread that generally stays waiting. The loop
/// is:
/// - As long as there is no connection and it does not need to exit, wait.
/// - Do what needs to be done.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	no tests defined.
///
class TDCLServer
{
public:
	///
	/// The constructor is public, but the class is abstract.
	///
	/// \param inApplication	interface with the host system. Used
	///							for the semaphore.
	///
	TDCLServer( TDCLApplication* inApplication );

	///
	/// Destructor.
	/// Kills the server if it is running.
	///
	virtual	~TDCLServer( void );


	///
	/// Server states.
	///
	enum EState {
		kStopped,	///< Initial state.
		kStarting,	///< State right after Start has been called.
		kRunning,	///< State once DoStart has finished.
		kStopping,	///< Stopping.
		kUnknown	///< Unknown state (in case of exception)
	};	

	///
	/// Starts the server.
	/// Does nothing if the server is not stopped.
	/// Changes the state to kStarting if everything succeeds.
	///
	/// \throws	TDCLException if a problem occurred. In that case
	///			the server remains in the stopped state.
	///
	void				Start( void );

	///
	/// Starts a graceful shutdown of the server.
	/// Normally asynchronous, though this may be synchronous.
	/// Does nothing if the server is not running.
	/// Changes the state to kStopping or kStopped depending on whether the
	/// operation was asynchronous or synchronous.
	///
	/// \throws	TDCLException if a problem occurred. In that case
	///			the server is in the unknown state (should not happen).
	///
	void				Stop( void );

	///
	/// Waits for the server worker to finish executing.
	/// Returns immediately with \c false if called from inside the server worker itself.
	///
	bool				Wait( void );
	
	///
	/// Forcefully terminates all connections (synchronous operation).
	/// Does nothing if the server is not running.
	/// Also kills communication interfaces that are currently closing.
	/// Changes the state to kStopped.
	///
	/// \throws	TDCLException if a problem occurred. In that case
	///			the server is in the unknown state (should not happen).
	///
	void				Kill( void );

	///
	/// Retrieves the server state.
	/// Note: this method uses the semaphore like Start/Stop/Kill.
	///
	/// \return the current state of the server
	///
	EState				GetState( void );

	///
	/// Method called by communication layers to indicate that a request is
	/// present. This method sends an event to the server thread.
	///
	/// \param inCommLayer	communication layer that received a request.
	///
	void				RequestPresent( TDCLCommLayer* inCommLayer );

	///
	/// Method called by communication layers to indicate that they are
	/// waiting for a connection. This is when the user should ask the Newton
	/// to connect.
	///
	/// \param inCommLayer	communication layer that is waiting.
	///
	void				WaitingConnection( TDCLCommLayer* inCommLayer );

	///
	/// Method called by links to indicate that they are disconnected
	/// (i.e. communication with the Newton has ended).
	///
	/// \param inLink		link that is disconnected.
	///
	virtual void		Disconnected( TDCLLink* inLink );

	///
	/// Method called when an exception is uncaught in a communication layer's
	/// thread.
	/// This method notifies the application.
	///
	/// \param inCommLayer	layer that crashed.
	/// \param inException	the exception if it is a TDCLException,
	///			\c nil otherwise.
	///
	virtual void 		HandleCommLayerException(
							TDCLCommLayer* inCommLayer,
							TDCLException* inException );

protected:
	///
	/// Method called when an event is received.
	/// This method can be overridden if a server needs to receive unexpected
	/// events.
	/// By default
	///
	/// \param 	inEvent	the received event.
	/// \param 	outProcessed on output, \c true if the event was handled,
	///			unchanged otherwise. If the event is not handled, an exception
	///			will be thrown.
	/// \return the new state of the server.
	///
	virtual EState		ProcessEvent(
							TDCLEvent* inEvent,
							bool* outProcessed );

	///
	/// Method called when an exception is uncaught.
	/// When this method returns, the thread has finished.
	///
	/// \param inException	the exception if it is a TDCLException,
	///			\c nil otherwise.
	///
	virtual void 		HandleException( TDCLException* inException );

	/// \name Server execution

	///
	/// Server main loop.
	/// This loop terminates when an exception occurs or when the server is
	/// stopped.
	///
	virtual void		Run( void );

	/// \name TDCLServer interface
	
	///
	/// Starts listening.
	/// This method is called when the server is started (from the server
	/// thread).
	///
	virtual	void		DoStartListening( void ) = 0;

	///
	/// Stops the server.
	///
	virtual	void		DoStop( void ) = 0;
	
	///
	/// Requests cooperative stop of objects managed by the subclass
	/// before joining the server thread.
	///
	virtual void		DoRequestStop( void );

	///
	/// Returns \c true if called from a worker thread that Kill() joins.
	///
	virtual bool		IsShutdownWorkerThread( void ) const;

	///
	/// Forcefully closes all connections.
	///
	virtual	void		DoKill( void ) = 0;

	///
	/// Handles the arrival of a new connection.
	///
	/// \param inCommLayer	communication interface that received the request.
	/// \return the new state of the server.
	/// \throws TDCLException if a problem occurred.
	///
	virtual EState		HandleIncomingConnection(
								TDCLCommLayer* inCommLayer ) = 0;

	///
	/// Method called on the server thread to indicate that a link has been
	/// disconnected and the communication interface must be closed.
	///
	/// \param inLink	the link that was disconnected.
	/// \return the new state of the server.
	/// \throws TDCLException if a problem occurred.
	///
	virtual	EState		HandleLinkDisconnection( TDCLLink* inLink ) = 0;

	///
	/// Method called by the server when nothing is happening.
	/// It allows calling Idle on communication interfaces that need it.
	///
	virtual	void		DoIdle( void ) = 0;

	///
	/// Acquires the server state mutex and checks that Start() has never been
	/// called and the server is completely stopped.
	///
	/// \return RAII lock holding the server state mutex.
	/// \throws TDCLBadStateError if Start() has already been called or the
	///			server is not stopped.
	///
	[[nodiscard]] std::unique_lock<std::mutex>	AcquireConfigurationLock( void );

	inline	TDCLApplication*	GetApplication( void )
			{
				return mApplication;
			}
	
private:
	///
	/// Copy constructor intentionally unavailable.
	///
	/// \param inCopy		object to copy
	///
	TDCLServer( const TDCLServer& inCopy );

	///
	/// Assignment operator intentionally unavailable.
	///
	/// \param inCopy		object to copy
	///
	TDCLServer& operator = ( const TDCLServer& inCopy );

	void				ThreadEntry( std::stop_token inStopToken );

	/// \name Member variables

	TDCLApplication*			mApplication;	///< Reference to the
											///< application.
	TDCLEventQueue				mEventQueue;	///< Server event queue.
	std::mutex					mStateMutex;	///< Mutex protecting the state.
	std::condition_variable		mKillCV;		///< Start/Kill synchronization.
	std::mutex					mJoinMutex;		///< Mutex for Wait().
	std::atomic<std::thread::id> mThreadId;		///< Active thread identifier.
	std::thread::id				mKillThreadId;	///< Thread executing Kill().
	std::jthread				mThread;		///< Server thread.
	std::stop_token				mWorkerStopToken; ///< Current worker token.
	EState						mState;			///< State.
	bool						mHasStarted;	///< Whether Start() has been called.
};

#endif
		// __TDCLSERVER__

// ============================================================= //
// APL is a natural extension of assembler language programming; //
// ...and is best for educational purposes.                      //
//                 -- A. Perlis                                  //
// ============================================================= //
