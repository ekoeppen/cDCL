// ==============================
// Fichier:			DCLNSRef.h
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
// The Original Code is DCLNSRef.h.
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
// $Id: DCLNSRef.h,v 1.4 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _DCLNSREF_H
#define _DCLNSREF_H

#import <DCL/Headers/DCLDefinitions.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

#import <objc/objc.h>
#import <Foundation/NSData.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// DCL
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

///
/// Interface ObjC (avec Foundation) pour les références NewtonScript.
/// Il s'agit d'une vue construite autour de TDCLNSRef.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
@interface DCLNSRef : NSObject
{
@private
	// D'après
	// http://developer.apple.com/documentation/ReleaseNotes/Cocoa/Objective-C++.html
	// cet objet C++ ne sera ni construit avec le constructeur par défaut ni
	// détruit par la colle ObjC.
	// (c'est bien ce que CW compile et gcc génère un avertissement)
	// Le constructeur est appelé dans allocWithZone et le destructeur
	// dans dealloc.
	TDCLNSRef			mRef;	///< Référence C++
}

///
/// Allocation de l'objet dans une zone.
/// Appelle la méthode de NSObject.
/// Appelle aussi le constructeur par défaut de TDCLNSRef.
///
+ (id) allocWithZone: (NSZone*) zone;

///
/// Initialise l'objet comme référence sur un entier.
///
- (id) initAsRefToInt: (long) inInt;

///
/// Initialise l'objet comme référence sur un caractère (unicode).
///
- (id) initAsRefToChar: (unsigned short) inChar;

///
/// Initialise l'objet comme référence sur un booléen.
///
- (id) initAsRefToBool: (BOOL) inBool;

///
/// Initialise l'objet comme référence sur un nouveau tableau.
///
- (id) initAsRefToNewArray;

///
/// Initialise l'objet comme une référence sur un nouveau tableau
/// d'une taille donnée.
///
- (id) initAsRefToNewArrayOfSize: (unsigned long) inSize;

///
/// Initialise l'objet comme référence sur un nouveau tableau
/// d'une taille donnée rempli avec une certaine valeur.
///
- (id) initAsRefToNewArrayOfSize: (unsigned long) inSize
			filledWith: (DCLNSRef*) inRef;

///
/// Initialise l'objet comme référence sur une nouvelle structure
/// NewtonScript.
///
- (id) initAsRefToNewFrame;

///
/// Initialise l'objet comme référence sur un nouveau symbole.
///
- (id) initAsRefToNewSymbol: (NSString*) inSymbolString;

///
/// Initialise l'objet comme référence sur une nouvelle chaîne.
///
- (id) initAsRefToNewString: (NSString*) inString;

///
/// Initialise l'objet comme référence sur un nouveau binaire.
///
- (id) initAsRefToNewBinary: (NSData*) inData;

///
/// Initialise l'objet comme référence sur un nouveau binaire
/// d'une classe donnée (sous forme de référence).
///
- (id) initAsRefToNewBinary: (NSData*) inData
			ofClass: (DCLNSRef*) inClass;

///
/// Initialise l'objet comme référence sur un nouveau binaire
/// d'une classe donnée (sous forme de chaîne).
///
- (id) initAsRefToNewBinary: (NSData*) inData
			ofClassAsString: (NSString*) inClass;

///
/// Méthode appelée lorsque l'objet est supprimé.
/// Appelle le destructeur de TDCLNSRef.
///
- (void) dealloc;

///
/// Crée une référence sur un entier.
/// Appelle autorelease.
///
+ (id) refToInt: (long) inInt;

///
/// Crée une référence sur un caractère (unicode).
/// Appelle autorelease.
///
+ (id) refToChar: (unsigned short) inChar;

///
/// Crée une référence sur un booléen.
/// Appelle autorelease.
///
+ (id) refToBool: (BOOL) inBool;

///
/// Crée une référence sur un nouveau tableau.
/// Appelle autorelease.
///
+ (id) refToNewArray;

///
/// Crée une référence sur un nouveau tableau d'une taille donnée.
/// Appelle autorelease.
///
+ (id) refToNewArrayOfSize: (unsigned long) inSize;

///
/// Crée une référence sur un nouveau tableau d'une taille donnée rempli
/// avec une certaine valeur.
/// Appelle autorelease.
///
+ (id) refToNewArrayOfSize: (unsigned long) inSize
			filledWith: (DCLNSRef*) inRef;

///
/// Crée une référence sur une nouvelle structure NewtonScript.
/// Appelle autorelease.
///
+ (id) refToNewFrame;

///
/// Crée une référence sur un nouveau symbole.
/// Appelle autorelease.
///
+ (id) refToNewSymbol: (NSString*) inSymbolString;

///
/// Crée une référence sur une nouvelle chaîne.
/// Appelle autorelease.
///
+ (id) refToNewString: (NSString*) inString;

///
/// Crée une référence sur un nouveau binaire.
/// Appelle autorelease.
///
+ (id) refToNewBinary: (NSData*) inData;

///
/// Crée une référence sur un nouveau binaire d'une classe donnée.
/// Appelle autorelease.
///
+ (id) refToNewBinary: (NSData*) inData ofClass: (DCLNSRef*) inClass;

///
/// Crée une référence sur un nouveau binaire d'une classe donnée (chaîne).
/// Appelle autorelease.
///
+ (id) refToNewBinary: (NSData*) inData
			ofClassAsString: (NSString*) inClass;

@end

#endif
		// _DCLNSREF_H

// ========================================================================= //
//         There was once a programmer who was attached to the court of the  //
// warlord of Wu.  The warlord asked the programmer: "Which is easier to     //
// design: an accounting package or an operating system?"                    //
//         "An operating system," replied the programmer.                    //
//         The warlord uttered an exclamation of disbelief.  "Surely an      //
// accounting package is trivial next to the complexity of an operating      //
// system," he said.                                                         //
//         "Not so," said the programmer, "when designing an accounting      //
// package, the programmer operates as a mediator between people having      //
// different ideas: how it must operate, how its reports must appear, and    //
// how it must conform to the tax laws.  By contrast, an operating system is //
// not limited by outside appearances.  When designing an operating system,  //
// the programmer seeks the simplest harmony between machine and ideas.      //
// This is why an operating system is easier to design."                     //
//         The warlord of Wu nodded and smiled.  "That is all good and well, //
// but which is easier to debug?"                                            //
//         The programmer made no reply.                                     //
//                 -- Geoffrey James, "The Tao of Programming"               //
// ========================================================================= //
