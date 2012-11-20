// ==============================
// Fichier:			DCLNSRef.mm
// Projet:			Desktop Connection Library
// 
// Créé le:			31/12/2003
// Tabulation:		4 espaces
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
// The Original Code is DCLNSRef.mm.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2003-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: DCLNSRef.mm,v 1.3 2004/11/24 14:09:08 paul Exp $
// ===========

#import <DCL/Headers/DCLDefinitions.h>
#import <DCL/NS_Objects/Views/ObjC/DCLNSRef.h>

// C++
#include <new>

@implementation DCLNSRef

// ------------------------------------------------------------------------- //
//  * allocWithZone
// ------------------------------------------------------------------------- //
+ (id) allocWithZone: (NSZone*) zone;
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theResult = [super allocWithZone: zone];
	new (&theResult->mRef) TDCLNSRef();
	return theResult;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToInt: (long)
// ------------------------------------------------------------------------- //
- (id) initAsRefToInt: (long) inInt
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeInt( inInt );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToChar: (unsigned short)
// ------------------------------------------------------------------------- //
- (id) initAsRefToChar: (unsigned short) inChar
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeChar( (unsigned short) inChar );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToBool: (BOOL)
// ------------------------------------------------------------------------- //
- (id) initAsRefToBool: (BOOL) inBool
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeBoolean( (inBool == YES) );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewArray
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewArray
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeArray();
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewArrayOfSize: (unsigned long) inSize
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewArrayOfSize: (unsigned long) inSize
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeArray( inSize );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewArrayOfSize: (unsigned long) filledWith: (DCLNSRef*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewArrayOfSize: (unsigned long) inSize
			filledWith: (DCLNSRef*) inRef
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeArray( inSize, inRef->mRef );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewFrame
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewFrame
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeFrame();
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewSymbol: (NSString*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewSymbol: (NSString*) inSymbolString
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Extraction des caractères en UTF-16.
	// (ceci peut merdouiller sur x86 si c'est du UTF-16 LE).
	unsigned int theLength = [inSymbolString length] + 1;
	KUInt16* theBuffer = (KUInt16*) ::malloc( sizeof( KUInt16 ) * theLength );
	theBuffer[theLength - 1] = '\0';
	[inSymbolString getCharacters: theBuffer];
	mRef = TDCLNSRef::MakeSymbol( theBuffer );
	
	// Ménage.
	::free( theBuffer );
	
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewString: (NSString*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewString: (NSString*) inString
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Extraction des caractères en UTF-16.
	// (ceci peut merdouiller sur x86 si c'est du UTF-16 LE).
	unsigned int theLength = [inString length] + 1;
	KUInt16* theBuffer = (KUInt16*) ::malloc( sizeof( KUInt16 ) * theLength );
	theBuffer[theLength - 1] = '\0';
	[inString getCharacters: theBuffer];
	mRef = TDCLNSRef::MakeString( theBuffer );
	
	// Ménage.
	::free( theBuffer );
	
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewBinary: (NSData*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewBinary: (NSData*) inData
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeBinary( [inData bytes], [inData length] );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewBinary: (NSData*) ofClass: (DCLNSRef*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewBinary: (NSData*) inData
		ofClass: (DCLNSRef*) inClass
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef = TDCLNSRef::MakeBinary(
						[inData bytes],
						[inData length],
						inClass->mRef );
	return self;
}

// ------------------------------------------------------------------------- //
//  * initAsRefToNewBinary: (NSData*) ofClassAsString: (NSString*)
// ------------------------------------------------------------------------- //
- (id) initAsRefToNewBinary: (NSData*) inData
		ofClassAsString: (NSString*) inClass
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	// Extraction des caractères en UTF-16.
	// (ceci peut merdouiller sur x86 si c'est du UTF-16 LE).
	unsigned int theLength = [inClass length] + 1;
	KUInt16* theBuffer = (KUInt16*) ::malloc( sizeof( KUInt16 ) * theLength );
	theBuffer[theLength - 1] = '\0';
	[inClass getCharacters: theBuffer];
	TDCLNSRef theClassRef = TDCLNSRef::MakeString( theBuffer );
	
	// Ménage.
	::free( theBuffer );

	mRef = TDCLNSRef::MakeBinary(
						[inData bytes],
						[inData length],
						theClassRef );
	return self;
}

// ------------------------------------------------------------------------- //
//  * dealloc
// ------------------------------------------------------------------------- //
- (void) dealloc
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	mRef.TDCLNSRef::~TDCLNSRef();

	[super dealloc];
}

// ------------------------------------------------------------------------- //
//  * refToInt: (long)
// ------------------------------------------------------------------------- //
+ (id) refToInt: (long) inInt
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc] initAsRefToInt: inInt];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToChar: (unsigned short)
// ------------------------------------------------------------------------- //
+ (id) refToChar: (unsigned short) inChar
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc] initAsRefToChar: inChar];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToBool: (BOOL)
// ------------------------------------------------------------------------- //
+ (id) refToBool: (BOOL) inBool
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc] initAsRefToBool: inBool];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewArray
// ------------------------------------------------------------------------- //
+ (id) refToNewArray
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc] initAsRefToNewArray];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewArrayOfSize: (unsigned long)
// ------------------------------------------------------------------------- //
+ (id) refToNewArrayOfSize: (unsigned long) inSize
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc] initAsRefToNewArrayOfSize: inSize];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewArrayOfSize: (unsigned long) filledWith: (DCLNSRef*)
// ------------------------------------------------------------------------- //
+ (id) refToNewArrayOfSize: (unsigned long) inSize
			filledWith: (DCLNSRef*) inRef
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewArrayOfSize: inSize
							filledWith: inRef ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewFrame
// ------------------------------------------------------------------------- //
+ (id) refToNewFrame
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewFrame ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewSymbol: (NSString*)
// ------------------------------------------------------------------------- //
+ (id) refToNewSymbol: (NSString*) inSymbolString
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewSymbol: inSymbolString ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewString: (NSString*)
// ------------------------------------------------------------------------- //
+ (id) refToNewString: (NSString*) inString
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewString: inString ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewBinary: (NSData*)
// ------------------------------------------------------------------------- //
+ (id) refToNewBinary: (NSData*) inData
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewBinary: inData ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewBinary: (NSData*) ofClass: (DCLNSRef*)
// ------------------------------------------------------------------------- //
+ (id) refToNewBinary: (NSData*) inData ofClass: (DCLNSRef*) inClass
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewBinary: inData
									ofClass: inClass ];
	return [theRef autorelease];
}

// ------------------------------------------------------------------------- //
//  * refToNewBinary: (NSData*) ofClassAsString: (NSString*)
// ------------------------------------------------------------------------- //
+ (id) refToNewBinary: (NSData*) inData ofClassAsString: (NSString*) inClass
{
#ifdef __MWERKS__
	#pragma unused ( _cmd )
#endif

	DCLNSRef* theRef = [[DCLNSRef alloc]
							initAsRefToNewBinary: inData
									ofClassAsString: inClass ];
	return [theRef autorelease];
}

@end

// ======================================================================== //
// Congratulations!  You are the one-millionth user to log into our system. //
// If there's anything special we can do for you, anything at all, don't    //
// hesitate to ask!                                                         //
// ======================================================================== //
