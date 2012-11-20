// ==============================
// Fichier:			TDCLNSObject.cp
// Projet:			Desktop Connection Library
//
// CrŽŽ le:			06/08/2002
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
// The Original Code is TDCLNSObject.cp.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vac’k
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2002-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vac’k <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSObject.h>

#warning TODO: handle weak references.

// ------------------------------------------------------------------------- //
//  * AddWeakRef( void )
// ------------------------------------------------------------------------- //
void
TDCLNSObject::AddWeakRef( TDCLNSWeakRef* /* inWeakRef */ )
{
}

// ------------------------------------------------------------------------- //
//  * RemoveWeakRef( void )
// ------------------------------------------------------------------------- //
void
TDCLNSObject::RemoveWeakRef( TDCLNSWeakRef* /* inWeakRef */ )
{
}

// ------------------------------------------------------------------------- //
//  * IsFrame( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsFrame( void ) const
{
	return false;
}

// ------------------------------------------------------------------------- //
//  * IsFunction( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsFunction( void ) const
{
	return false;
}

// ------------------------------------------------------------------------- //
//  * IsSymbol( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsSymbol( void ) const
{
	return false;
}

// ------------------------------------------------------------------------- //
//  * IsArray( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsArray( void ) const
{
	return false;
}

// ------------------------------------------------------------------------- //
//  * IsBinary( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsBinary( void ) const
{
	return false;
}

// ------------------------------------------------------------------------- //
//  * IsString( void ) const
// ------------------------------------------------------------------------- //
Boolean
TDCLNSObject::IsString( void ) const
{
	return false;
}

// =============================================================================== //
//         Several students were asked to prove that all odd integers are prime.   //
//         The first student to try to do this was a math student.  "Hmmm...       //
// Well, 1 is prime, 3 is prime, 5 is prime, and by induction, we have that all    //
// the odd integers are prime."                                                    //
//         The second student to try was a man of physics who commented, "I'm not  //
// sure of the validity of your proof, but I think I'll try to prove it by         //
// experiment."  He continues, "Well, 1 is prime, 3 is prime, 5 is prime, 7 is     //
// prime, 9 is...  uh, 9 is... uh, 9 is an experimental error, 11 is prime, 13     //
// is prime...  Well, it seems that you're right."                                 //
//         The third student to try it was the engineering student, who responded, //
// "Well, to be honest, actually, I'm not sure of your answer either.  Let's       //
// see...  1 is prime, 3 is prime, 5 is prime, 7 is prime, 9 is... uh, 9 is...     //
// well, if you approximate, 9 is prime, 11 is prime, 13 is prime...  Well, it     //
// does seem right."                                                               //
//         Not to be outdone, the computer science student comes along and says    //
// "Well, you two sort've got the right idea, but you'll end up taking too long!   //
// I've just whipped up a program to REALLY go and prove it."  He goes over to     //
// his terminal and runs his program.  Reading the output on the screen he says,   //
// "1 is prime, 1 is prime, 1 is prime, 1 is prime..."                             //
// =============================================================================== //
