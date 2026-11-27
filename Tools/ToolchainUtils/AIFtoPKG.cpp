// ==============================
// File:            AIFtoPKG.cpp
// Project:         Desktop Connection Library
// Written by:      Eckhart Koeppen (eck@40hz.org)
//
// Tab width:       4 spaces
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
// The Original Code is AIFtoPKG.cpp.
//
// The Initial Developer of the Original Code is Eckhart Koeppen (eck@40hz.org).
// Portions created by the Initial Developer are Copyright (C) 2026
// the Initial Developer. All Rights Reserved.
//
// ***** END LICENSE BLOCK *****
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

#include <DCL/Package/TDCLPackage.h>
#include <DCL/Package/TDCLPkgRelocatablePart.h>

#include "TAIFFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

void
usage( const char* toolName )
{
	(void)::fprintf( stderr, "Generate a Newton package from an ARM AIF file.\n" );
	(void)::fprintf( stderr,
	                 "Syntax: %s -o output.pkg package_name (-protocol|-raw) input.aif [-autoLoad] [-autoRemove] "
	                 "[-autoCopy] [-notify Type HandlerInfo]\n",
	                 toolName );
}

TDCLPkgRelocatablePart*
ParseAIF( TAIFFile& aifFile )
{
	KUInt32 payloadSize = aifFile.GetPayloadSize();
	if ( payloadSize == 0 )
	{
		throw std::runtime_error( "AIF image is empty" );
	}

	void* buffer = ::malloc( payloadSize );
	if ( buffer == NULL )
	{
		throw std::runtime_error( "Out of memory allocating AIF payload" );
	}

	try
	{
		aifFile.ReadPayload( buffer );
	}
	catch ( ... )
	{
		::free( buffer );
		throw;
	}

	KUInt32 baseAddress = aifFile.GetPayloadBaseAddress();
	TDCLPkgRelocatablePart* result = new TDCLPkgRelocatablePart( baseAddress, buffer, payloadSize );
	::free( buffer );

	const std::vector<KUInt32>& relocs = aifFile.GetRelocations();
	if ( !relocs.empty() )
	{
		result->SetRelocations( (KUInt32)relocs.size(), relocs.data(), baseAddress );
	}

	return result;
}

int
main( int argc, char* argv[] )
{
	if ( argc < 6 )
	{
		usage( argv[0] );
		return -1;
	}
	if ( strcmp( argv[1], "-o" ) != 0 )
	{
		usage( argv[0] );
		return -2;
	}

	KUInt32 partType = 'prot';
	KUInt32 flags = TDCLPackage::kPartProtocolPart;
	if ( strcmp( argv[4], "-protocol" ) == 0 )
	{
		partType = 'prot';
		flags = TDCLPackage::kPartProtocolPart;
	}
	else if ( strcmp( argv[4], "-raw" ) == 0 )
	{
		partType = 'raw ';
		flags = TDCLPackage::kPartRawPart;
	}
	else
	{
		usage( argv[0] );
		return -2;
	}

	const char* handlerInfo = NULL;
	int optionIx = 6;
	while ( optionIx < argc )
	{
		if ( strcmp( argv[optionIx], "-autoLoad" ) == 0 )
		{
			flags |= TDCLPackage::kPartAutoLoadFlag;
		}
		else if ( strcmp( argv[optionIx], "-autoCopy" ) == 0 )
		{
			flags |= TDCLPackage::kPartAutoCopyFlag;
		}
		else if ( strcmp( argv[optionIx], "-autoRemove" ) == 0 )
		{
			flags |= TDCLPackage::kPartAutoRemoveFlag;
		}
		else if ( strcmp( argv[optionIx], "-notify" ) == 0 && optionIx + 2 < argc )
		{
			flags |= TDCLPackage::kPartNotifyFlag;
			const char* notifyType = argv[optionIx + 1];
			if ( ::strlen( notifyType ) != 4 )
			{
				usage( argv[0] );
				return -2;
			}
			partType = ( (KUInt32)(KUInt8)notifyType[0] << 24 ) | ( (KUInt32)(KUInt8)notifyType[1] << 16 ) |
			           ( (KUInt32)(KUInt8)notifyType[2] << 8 ) | ( (KUInt32)(KUInt8)notifyType[3] );
			handlerInfo = argv[optionIx + 2];
			optionIx += 2;
		}
		else
		{
			usage( argv[0] );
			return -2;
		}
		optionIx++;
	}

	int theResult = 0;

	try
	{
		TDCLPlatformFiles theFilesIntf;
		TAIFFile aifFile( &theFilesIntf, argv[5] );
		aifFile.Open();

		TDCLPkgRelocatablePart* part = ParseAIF( aifFile );

		TDCLPackage thePackage;
		thePackage.SetNOS1Compatible( false );
		thePackage.SetPackageID( TDCLPackage::kDefaultID );
		thePackage.SetPackageFlags( 0 );
		thePackage.SetPackageVersion( 1 );

		thePackage.SetPackageName( argv[3] );
		thePackage.SetCopyrightString( "" );

		if ( handlerInfo == NULL )
		{
			(void)thePackage.AddPart( partType, flags, part );
		}
		else
		{
			(void)thePackage.AddPart( partType, flags, strlen( handlerInfo ), (const KUInt8*)handlerInfo, part );
		}

		TDCLPlatformFile outputFile( &theFilesIntf, argv[2] );
		outputFile.Create();
		if ( !outputFile.IsOpen() )
		{
			outputFile.Open( false );
		}

		try
		{

			thePackage.WriteToStream( &outputFile );
			outputFile.FlushOutput();
		}
		catch ( ... )
		{
			try
			{
				outputFile.Close();
#ifdef _WIN32
				(void)::remove( argv[2] );
#else
				outputFile.Delete();
#endif
			}
			catch ( ... )
			{
			}
			throw;
		}
	}
	catch ( TDCLException& anException )
	{
		(void)::fprintf( stderr, "A DCL exception occurred (%s, %i, %i)\n", anException.GetExceptionName(),
		                 (int)anException.GetExCode(), (int)anException.GetError() );
		theResult = -1;
	}
	catch ( std::runtime_error& anErr )
	{
		(void)::fprintf( stderr, "%s\n", anErr.what() );
		theResult = -1;
	}
	catch ( std::exception& anException )
	{
		(void)::fprintf( stderr, "A STD exception occurred (%s)\n", anException.what() );
		theResult = -1;
	}
	catch ( ... )
	{
		(void)::fprintf( stderr, "An unknown exception occurred\n" );
		theResult = -1;
	}

	return theResult;
}
