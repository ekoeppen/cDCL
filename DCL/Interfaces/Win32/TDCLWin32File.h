// ==============================
// Fichier:			TDCLWin32File.h
// Projet:			Desktop Connection Library
//
// Créé le:			10/05/2026
// ===========

#ifndef __TDCLWIN32FILE__
#define __TDCLWIN32FILE__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Interfaces/TDCLFile.h>
#include <DCL/Interfaces/Win32/TDCLWin32Files.h>
#include <stdio.h>

class TDCLWin32File
	:
		public TDCLFile
{
public:
	TDCLWin32File( IDCLFiles* inFilesIntf, const char* inPath );
	virtual ~TDCLWin32File( void );

	virtual void		Open( Boolean inReadOnly );	
	virtual void		Create( void );	
	virtual void		Delete( void );	
	virtual Boolean		IsOpen( void ) const;
	virtual Boolean		IsReadOnly( void ) const;
	virtual KUInt64		GetLength( void );
	virtual void		Close( void );
	virtual void		Read( void* outBuffer, KUInt32* ioCount );
	virtual void		Write( const void* inBuffer, KUInt32* ioCount );
	virtual void		FlushOutput( void );
	virtual KUInt64		GetPosition( void );
	virtual void		SetPosition( KUInt64 inPosition );
	virtual KSInt64		GetCursor( void );
	virtual void		SetCursor( KSInt64 inOffset, ECursorMode inMode );
	virtual TDCLNSRef	GetKind( void ) const;
	virtual KUInt32		GetCreationDate( void ) const;
	virtual KUInt32		GetModificationDate( void ) const;
	virtual TDCLNSRef	GetStringPath( void ) const;
	virtual const KUInt16* GetName( void ) const;
	virtual KUInt16*	MakeName( void ) const;

private:
	char*				mPath;
	FILE*				mFile;
	Boolean				mReadOnly;
};

#endif
