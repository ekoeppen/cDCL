// ==============================
// File: TDCLClientSocket.h
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#ifndef _TDCLCLIENTSOCKET_H
#define _TDCLCLIENTSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <string>

#if TARGET_OS_WIN32
#include <DCL/Communication_Layers/Win32/TDCLWin32Socket.h>
typedef TDCLWin32Socket TDCLPlatformSocket;
#else
#include <DCL/Communication_Layers/POSIX/TDCLBSDSocket.h>
typedef TDCLBSDSocket TDCLPlatformSocket;
#endif

class TDCLClientSocket : public TDCLPlatformSocket
{
public:
	TDCLClientSocket(
		IDCLThreads* inThreadsIntf,
		std::string inHost,
		unsigned short inPort,
		long inTimeout = kDefaultTimeout );

	virtual ~TDCLClientSocket( void );

protected:
	virtual void		DoStartListening( void ) override;
	virtual void		DoStopListening( void ) override;
	virtual Boolean		WaitForIncomingRequest( void ) override;
	virtual TDCLPipe*	DoAccept( void ) override;
	virtual void		DoRefuse( void ) override;

private:
	std::string		mHost;
	unsigned short	mPort;
	dcl_socket_t	mConnectedSocket;
	dcl_socket_t	mInterruptPair[2];
	Boolean			mConnected;
	Boolean			mStopped;
};

#endif // _TDCLCLIENTSOCKET_H
