// ==============================
// File: TDCLClientSocket.h
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#ifndef _TDCLCLIENTSOCKET_H
#define _TDCLCLIENTSOCKET_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/Common/TDCLWakeableWait.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#ifdef _WIN32
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

protected:
	virtual void						DoStartListening( void ) override;
	virtual void						DoStopListening( void ) override;
	virtual bool						WaitForIncomingRequest( void ) override;
	virtual std::unique_ptr<TDCLPipe>	DoAccept( void ) override;
	virtual void						DoRefuse( void ) override;

private:
	std::string		mHost;
	unsigned short	mPort;
	std::mutex		mSocketMutex;
	dcl_socket_t	mConnectedSocket;
	TDCLWakeableWait	mWakeWait;
	bool			mConnected;
	std::atomic<bool>	mStopped;
};

#endif // _TDCLCLIENTSOCKET_H
