// ==============================
// File: TPackageLoaderLink.h
// Project: Desktop Connection Library (PackageLoader)
// ==============================

#ifndef _TPACKAGELOADERLINK_H
#define _TPACKAGELOADERLINK_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLoadPackageLink.h>

#include <string>

class TPackageLoaderApp;

class TPackageLoaderLink : public TDCLLoadPackageLink
{
public:
	TPackageLoaderLink( TPackageLoaderApp* inApplication, bool inVerbose = false );
	virtual ~TPackageLoaderLink( void );

	void			WaitForThreadToStop( unsigned int inTimeoutMs = 1000 );

	virtual void	DoConnect( void ) override;
	virtual void	DoDisconnect( void ) override;

protected:
	virtual void	StartInstallingPackage( void ) override;
	virtual void	InstallationProgress( double inProgress ) override;
	virtual EState	ProcessDockCommand(
						TDCLDockCommand* inCommand,
						Boolean* outProcessed ) override;

private:
	void			Log( const std::string& inMessage );
	void			LogCommand( const char* inDirection, TDCLDockCommand* inCommand );
	static std::string	FormatCmd( KUInt32 inCmd );
	static std::string	ExplainError( KSInt32 inCode );

	TPackageLoaderApp*	mLoaderApp;
	bool				mVerbose;
	int					mLastPercent;
};

#endif // _TPACKAGELOADERLINK_H
