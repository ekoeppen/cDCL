// ==============================
// Fichier:			KDCLSYM.h
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
// The Original Code is KDCLSYM.h.
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
// $Id: KDCLSYM.h,v 1.7 2004/11/24 14:09:07 paul Exp $
// ===========

#ifndef __KDCLSYM__
#define __KDCLSYM__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Classe pour les différents symboles utilisés dans la DCL.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
/// \test	aucun test défini.
///
class KDCLSYM
{
public:
	/// '_proto
	static const TDCLNSRef	kSYM_proto;

	/// 'argFrame
	static const TDCLNSRef	kSYMargFrame;

	/// 'array
	static const TDCLNSRef	kSYMarray;

	/// 'binary
	static const TDCLNSRef	kSYMbinary;

	/// 'boolean
	static const TDCLNSRef	kSYMboolean;

	/// 'bottom
	static const TDCLNSRef	kSYMbottom;

	/// 'char
	static const TDCLNSRef	kSYMchar;

	/// 'class
	static const TDCLNSRef	kSYMclass;

	/// 'CodeBlock
	static const TDCLNSRef	kSYMCodeBlock;

	/// 'debug
	static const TDCLNSRef	kSYMdebug;

	/// 'espy
	static const TDCLNSRef	kSYMespy;

	/// 'face
	static const TDCLNSRef	kSYMface;

	/// 'family
	static const TDCLNSRef	kSYMfamily;

	/// 'frame
	static const TDCLNSRef	kSYMframe;

	/// 'geneva
	static const TDCLNSRef	kSYMgeneva;

	/// 'handwriting
	static const TDCLNSRef	kSYMhandwriting;

	/// 'immediate
	static const TDCLNSRef	kSYMimmediate;

	/// 'instructions
	static const TDCLNSRef	kSYMinstructions;

	/// 'int
	static const TDCLNSRef	kSYMint;

	/// 'left
	static const TDCLNSRef	kSYMleft;

	/// 'literals
	static const TDCLNSRef	kSYMliterals;

	/// 'name
	static const TDCLNSRef	kSYMname;

	/// 'newYork
	static const TDCLNSRef	kSYMnewYork;

	/// 'numArgs
	static const TDCLNSRef	kSYMnumArgs;

	/// 'right
	static const TDCLNSRef	kSYMright;

	/// 'size
	static const TDCLNSRef	kSYMsize;
	
	/// 'string
	static const TDCLNSRef	kSYMstring;
	
	/// 'string.email
	static const TDCLNSRef	kSYMstring_email;

	/// 'styles
	static const TDCLNSRef	kSYMstyles;

	/// 'symbol
	static const TDCLNSRef	kSYMsymbol;

	/// 'tabs
	static const TDCLNSRef	kSYMtabs;

	/// 'text
	static const TDCLNSRef	kSYMtext;

	/// 'textFlags
	static const TDCLNSRef	kSYMtextFlags;

	/// 'top
	static const TDCLNSRef	kSYMtop;

	/// 'viewBounds
	static const TDCLNSRef	kSYMviewBounds;

	/// 'viewClass
	static const TDCLNSRef	kSYMviewClass;

	/// 'viewFlags
	static const TDCLNSRef	kSYMviewFlags;

	/// 'viewFont
	static const TDCLNSRef	kSYMviewFont;

	/// 'viewFormat
	static const TDCLNSRef	kSYMviewFormat;

	/// 'viewJustify
	static const TDCLNSRef	kSYMviewJustify;

	/// 'viewStationery
	static const TDCLNSRef	kSYMviewStationery;

	/// 'weird_immediate
	static const TDCLNSRef	kSYMweird_immediate;
};

#endif
		// __KDCLSYM__

// ============================================================================== //
// This "brain-damaged" epithet is getting sorely overworked.  When we can        //
// speak of someone or something being flawed, impaired, marred, spoiled;         //
// batty, bedlamite, bonkers, buggy, cracked, crazed, cuckoo, daft, demented,     //
// deranged, loco, lunatic, mad, maniac, mindless, non compos mentis, nuts,       //
// Reaganite, screwy, teched, unbalanced, unsound, witless, wrong;  senseless,    //
// spastic, spasmodic, convulsive; doped, spaced-out, stoned, zonked;  {beef,     //
// beetle,block,dung,thick}headed, dense, doltish, dull, duncical, numskulled,    //
// pinhead;  asinine, fatuous, foolish, silly, simple;  brute, lumbering, oafish; //
// half-assed, incompetent; backward, retarded, imbecilic, moronic; when we have  //
// a whole precisely nuanced vocabulary of intellectual abuse to draw upon,       //
// individually and in combination, isn't it a little <fill in the blank> to be   //
// limited to a single, now quite trite, adjective?                               //
// ============================================================================== //
