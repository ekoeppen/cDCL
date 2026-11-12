// ==============================
// File: TDCLClientSocket.h
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#ifndef _TDCLCLIENTSOCKET_H
#define _TDCLCLIENTSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
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
		std::string inHost,
		unsigned short inPort,
		long inTimeout = kDefaultTimeout );

	virtual ~TDCLClientSocket( void );

	virtual TDCLSessionId	AcceptSession( void ) override;
	virtual void			Refuse( void ) override;

protected:
	virtual void			DoStartListening( void ) override;
	virtual void			DoStopListening( void ) override;
	virtual Boolean			WaitForIncomingRequest( void ) override;

private:
	std::string		mHost;
	unsigned short	mPort;
	dcl_socket_t	mConnectedSocket;
	TDCLWakeableWait	mWakeWait;
	Boolean			mConnected;
	Boolean			mStopped;
};

#endif // _TDCLCLIENTSOCKET_H
