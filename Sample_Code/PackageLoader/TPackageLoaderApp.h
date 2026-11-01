// ==============================
// File: TPackageLoaderApp.h
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#ifndef _TPACKAGELOADERAPP_H
#define _TPACKAGELOADERAPP_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLApplication.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

#include <string>
#include <mutex>
#include <condition_variable>

class TPackageLoaderApp : public TDCLApplication
{
public:
	TPackageLoaderApp( bool inVerbose = false );
	virtual ~TPackageLoaderApp( void ) = default;

	void			SetPackage( TDCLFSItemRef inPackageRef, std::string inPackageName );
	void			WaitForCompletion( void );
	void			SignalCompletion( void );
	bool			IsSuccess( void ) const { return mSuccess; }
	bool			IsVerbose( void ) const { return mVerbose; }
	const std::string&	GetErrorMessage( void ) const { return mErrorMessage; }

	void			Log( const std::string& inMessage );

	/// Current local time formatted as "[hh:mm:ss.mmm] ", used as log prefix.
	static std::string	Timestamp( void );

	// TDCLApplication interface
	virtual void	WaitConnection( TDCLServer* inServer, TDCLCommLayer* inLayer ) override;
	virtual void	Connected( TDCLLink* inLink, TDCLCommLayer* inLayer ) override;
	virtual void	ConnectedToNewtonDevice( TDCLLink* inLink, const KUInt16* inName ) override;
	virtual void	Disconnecting( TDCLLink* inLink ) override;
	virtual void	Disconnected( TDCLLink* inLink ) override;
	virtual void	ServerIsDown( TDCLServer* inServer ) override;
	virtual void	Cancel( TDCLLink* inLink ) override;
	virtual void	Timeout( TDCLLink* inLink ) override;

	virtual void	AppCommandBeingProcessed(
						TDCLLink* inLink,
						TDCLAppCommand* inCommand ) override;
	virtual void	AppCommandProcessed(
						TDCLLink* inLink,
						TDCLAppCommand* inCommand,
						TDCLException* inException ) override;

	virtual void	DockCommandBeingProcessed(
						TDCLLink* inLink,
						TDCLDockCommand* inCommand,
						TDCLLinkEngine* inEngine ) override;
	virtual void	DockCommandProcessed(
						TDCLLink* inLink,
						TDCLLinkEngine* inEngine,
						TDCLException* inException ) override;

	virtual void	HandleServerException(
						TDCLServer* inServer,
						TDCLException* inException ) override;
	virtual void	HandleCommLayerException(
						TDCLCommLayer* inCommLayer,
						TDCLException* inException ) override;
	virtual void	HandleLinkException(
						TDCLLink* inLink,
						TDCLException* inException ) override;
	virtual void	HandleException( TDCLException* inException ) override;

protected:
	virtual IDCLFiles*		CreateFilesIntf( void ) override;
	virtual IDCLThreads*	CreateThreadsIntf( void ) override;

private:
	TDCLFSItemRef			mPackageRef;
	std::string				mPackageName;
	bool				mVerbose;
	bool				mSuccess;
	std::string				mErrorMessage;
	std::mutex				mWaitMutex;
	std::condition_variable	mWaitCond;
	bool				mDone;
};

#endif // _TPACKAGELOADERAPP_H
