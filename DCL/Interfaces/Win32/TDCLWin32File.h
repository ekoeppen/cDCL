// ==============================
// Fichier:			TDCLWin32File.h
// Projet:			Desktop Connection Library
//
// Created:			10/05/2026
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
// The Original Code is TDCLWin32File.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//   Eckhart Koeppen <eck@40hz.org>
//
// ***** END LICENSE BLOCK *****
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
	virtual TDCLFSItemRef	MakeParentFolder( void ) const;

private:
	char*				mPath;
	FILE*				mFile;
	Boolean				mReadOnly;
};

#endif
