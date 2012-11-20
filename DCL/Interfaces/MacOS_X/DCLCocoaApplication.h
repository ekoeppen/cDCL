// ==============================
// Fichier:			DCLCocoaApplication.h
// Projet:			Desktop Connection Library
//
// Créé le:			30/3/2003
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
// The Original Code is DCLCocoaApplication.h.
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
// $Id: DCLCocoaApplication.h,v 1.6 2004/11/24 14:09:01 paul Exp $
// ===========

#ifndef _DCLCOCOAAPPLICATION_H
#define _DCLCOCOAAPPLICATION_H

#import <DCL/Headers/DCLDefinitions.h>
#import <DCL/Interfaces/MacOS_X/IDCLObjCApplication.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	off
	#pragma	warn_extracomma		off
#endif

#import <Cocoa/Cocoa.h>

#ifdef __MWERKS__
	#pragma	warn_implicitconv	reset
	#pragma	warn_extracomma		reset
#endif

// Pré-déclarations.
class TDCLObjCApplication;
class IDCLFiles;
class IDCLThreads;

///
/// Classe pour une application Cocoa utilisant la DCL.
///
/// Cet objet gère les événements asynchrones (tout sauf les créateurs des
/// interfaces) en utilisant des objets de la classe NSTimer envoyés sur
/// la boucle de l'application (la boucle courante lorsque la méthode
/// initWithController: est appelée).
///
/// Par ailleurs, en Cocoa, si on fait du multitâche préemptif avec mettons
/// les pthreads, il faut créer au moins un objet de type NSTask pour que
/// certains éléments de Cocoa soient initialisés. Cette classe crée un objet
/// NSTask bidon.
///
/// Pour utiliser cette classe, il faut créer un contrôleur qui adopte le
/// protocole IDCLObjCApplication et initialiser cette classe avec ce
/// contrôleur.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
@interface DCLCocoaApplication : NSObject < IDCLObjCApplication >
{
	NSRunLoop*					mRunLoop;		///< Boucle qui gère les événements.
	id < IDCLObjCApplication >	mController;	///< Référence sur le contrôleur.
	TDCLObjCApplication* 		mDCLAppl;		///< Référence sur l'application DCL.
	IDCLFiles*					mFilesIntf;		///< Interface pour les fichiers.
	IDCLThreads*				mThreadsIntf;	///< Interface pour les processus légers.
}

///
/// Initialisation de l'objet.
///
/// \param inContoller	contrôleur associé.
/// \return \c this
///
- (id) initWithController: (id < IDCLObjCApplication >) inController;

///
/// Crée une invocation pour le mandataire avec le sélecteur donné.
///
/// \param inSelector	sélecteur considéré
/// \return une invocation pour ce sélecteur
///
- (NSInvocation*) CreateInvocationWithSelector: (SEL) inSelector;

///
/// Envoie l'invocation sur la boucle.
///
/// \param inInvocation	invocation considérée.
///
- (void) SendInvocation: (NSInvocation*) inInvocation;

///
/// Méthode du processus léger bidon.
/// Ne sert à rien (enfin, sert uniquement à dire à Cocoa qu'on fait du
/// multi-tâche préemptif)
///
/// \param ignoredArgs	arguments ignorés.
///
- (void) DummyThreadEntryPoint: (id) ignoredArgs;

///
/// Accesseur sur l'application DCL
///
/// \return une référence sur l'application DCL.
///
- (TDCLObjCApplication*) GetDCLAppl;

@end

#endif
		// _DCLCOCOAAPPLICATION_H

// ========================================================================= //
// The net is like a vast sea of lutefisk with tiny dinosaur brains embedded //
// in it here and there. Any given spoonful will likely have an IQ of 1, but //
// occasional spoonfuls may have an IQ more than six times that!             //
//         -- James 'Kibo' Parry                                             //
// ========================================================================= //