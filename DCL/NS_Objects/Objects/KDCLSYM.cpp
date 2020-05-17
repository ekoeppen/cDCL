// ==============================
// Fichier:			KDCLSYM.cp
// Projet:			Desktop Connection Library
//
// Créé le:			23/10/2002
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
// The Original Code is KDCLSYM.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: KDCLSYM.cp,v 1.7 2004/11/24 14:09:07 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>

// ------------------------------------------------------------------------- //
//  * Constantes
// ------------------------------------------------------------------------- //
const TDCLNSRef KDCLSYM::kSYM_proto		= TDCLNSRef::MakeSymbol( "_proto" );
const TDCLNSRef KDCLSYM::kSYMargFrame	= TDCLNSRef::MakeSymbol( "argFrame" );
const TDCLNSRef KDCLSYM::kSYMarray		= TDCLNSRef::MakeSymbol( "array" );
const TDCLNSRef KDCLSYM::kSYMbinary		= TDCLNSRef::MakeSymbol( "binary" );
const TDCLNSRef KDCLSYM::kSYMboolean	= TDCLNSRef::MakeSymbol( "boolean" );
const TDCLNSRef KDCLSYM::kSYMbottom		= TDCLNSRef::MakeSymbol( "bottom" );
const TDCLNSRef KDCLSYM::kSYMchar		= TDCLNSRef::MakeSymbol( "char" );
const TDCLNSRef KDCLSYM::kSYMclass		= TDCLNSRef::MakeSymbol( "class" );
const TDCLNSRef KDCLSYM::kSYMCodeBlock	= TDCLNSRef::MakeSymbol( "CodeBlock" );
const TDCLNSRef KDCLSYM::kSYMdebug		= TDCLNSRef::MakeSymbol( "debug" );
const TDCLNSRef KDCLSYM::kSYMespy		= TDCLNSRef::MakeSymbol( "espy" );
const TDCLNSRef KDCLSYM::kSYMface		= TDCLNSRef::MakeSymbol( "face" );
const TDCLNSRef KDCLSYM::kSYMfamily		= TDCLNSRef::MakeSymbol( "family" );
const TDCLNSRef KDCLSYM::kSYMframe		= TDCLNSRef::MakeSymbol( "frame" );
const TDCLNSRef KDCLSYM::kSYMgeneva		= TDCLNSRef::MakeSymbol( "geneva" );
const TDCLNSRef KDCLSYM::kSYMhandwriting
										= TDCLNSRef::MakeSymbol( "handwriting" );
const TDCLNSRef KDCLSYM::kSYMimmediate	= TDCLNSRef::MakeSymbol( "immediate" );
const TDCLNSRef KDCLSYM::kSYMinstructions
										= TDCLNSRef::MakeSymbol( "instructions" );
const TDCLNSRef KDCLSYM::kSYMint		= TDCLNSRef::MakeSymbol( "int" );
const TDCLNSRef KDCLSYM::kSYMleft		= TDCLNSRef::MakeSymbol( "left" );
const TDCLNSRef KDCLSYM::kSYMliterals	= TDCLNSRef::MakeSymbol( "literals" );
const TDCLNSRef KDCLSYM::kSYMname		= TDCLNSRef::MakeSymbol( "name" );
const TDCLNSRef KDCLSYM::kSYMnewYork	= TDCLNSRef::MakeSymbol( "newYork" );
const TDCLNSRef KDCLSYM::kSYMnumArgs	= TDCLNSRef::MakeSymbol( "numArgs" );
const TDCLNSRef KDCLSYM::kSYMright		= TDCLNSRef::MakeSymbol( "right" );
const TDCLNSRef KDCLSYM::kSYMsize		= TDCLNSRef::MakeSymbol( "size" );
const TDCLNSRef KDCLSYM::kSYMstring		= TDCLNSRef::MakeSymbol( "string" );
const TDCLNSRef KDCLSYM::kSYMstring_email
										= TDCLNSRef::MakeSymbol( "string.email" );
const TDCLNSRef KDCLSYM::kSYMstyles		= TDCLNSRef::MakeSymbol( "styles" );
const TDCLNSRef KDCLSYM::kSYMsymbol		= TDCLNSRef::MakeSymbol( "symbol" );
const TDCLNSRef KDCLSYM::kSYMtabs		= TDCLNSRef::MakeSymbol( "tabs" );
const TDCLNSRef KDCLSYM::kSYMtext		= TDCLNSRef::MakeSymbol( "text" );
const TDCLNSRef KDCLSYM::kSYMtextFlags	= TDCLNSRef::MakeSymbol( "textFlags" );
const TDCLNSRef KDCLSYM::kSYMtop		= TDCLNSRef::MakeSymbol( "top" );
const TDCLNSRef KDCLSYM::kSYMviewBounds	= TDCLNSRef::MakeSymbol( "viewBounds" );
const TDCLNSRef KDCLSYM::kSYMviewClass	= TDCLNSRef::MakeSymbol( "viewClass" );
const TDCLNSRef KDCLSYM::kSYMviewFlags	= TDCLNSRef::MakeSymbol( "viewFlags" );
const TDCLNSRef KDCLSYM::kSYMviewFont	= TDCLNSRef::MakeSymbol( "viewFont" );
const TDCLNSRef KDCLSYM::kSYMviewFormat	= TDCLNSRef::MakeSymbol( "viewFormat" );
const TDCLNSRef KDCLSYM::kSYMviewJustify
										= TDCLNSRef::MakeSymbol( "viewJustify" );
const TDCLNSRef KDCLSYM::kSYMviewStationery
										= TDCLNSRef::MakeSymbol( "kSYMviewStationery" );
const TDCLNSRef KDCLSYM::kSYMweird_immediate
										= TDCLNSRef::MakeSymbol( "weird_immediate" );

// ========================================================================== //
// The primary purpose of the DATA statement is to give names to constants;   //
// instead of referring to pi as 3.141592653589793 at every appearance, the   //
// variable PI can be given that value with a DATA statement and used instead //
// of the longer form of the constant.  This also simplifies modifying the    //
// program, should the value of pi change.                                    //
//                 -- FORTRAN manual for Xerox Computers                      //
// ========================================================================== //
