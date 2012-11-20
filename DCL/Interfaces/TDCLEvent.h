// ==============================
// Fichier:			TDCLEvent.h
// Projet:			Desktop Connection Library
//
// Créé le:			28/3/2003
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
// The Original Code is TDCLEvent.h.
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
// $Id: TDCLEvent.h,v 1.3 2004/11/24 14:08:59 paul Exp $
// ===========

#ifndef _TDCLEVENT_H
#define _TDCLEVENT_H

#include <DCL/Headers/DCLDefinitions.h>

///
/// Classe pour les événements envoyés entre processus légers.
/// Un processus léger dort normalement tant qu'un tel événement
/// n'a pas été envoyé.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLEvent
{
public:
	///
	/// Types d'événements et masques pour filtrer avec WaitNextEvent.
	///
	enum EEventKind {
		kServerEvent		= 0x00000001,
		kServerEventMask	= kServerEvent,
		
		kLinkEvent			= 0x00000002,	///< Événements pour le lien (autre que les commandes)
		kIdleAppCommand		= 0x00000004,	///< Messages traités lorsque
											///< le lien est inactif
		kKbdAppCommand		= 0x00000008,	///< Messages traités lorsque
											///< le lien est en mode clavier
		kAnyAppCommandMask	= kIdleAppCommand | kKbdAppCommand,
		kAnyLinkEventMask	= kLinkEvent | kAnyAppCommandMask,
		
		kAnyEventMask		= 0xFFFFFFFF
	};

	///
	/// Identifiants pour les événements.	
	/// Les identifiants en minuscule sont réservés.
	///
	enum EEventID {
		// Messages reçus par le serveur.
		kStopServer			= FOUR_CHAR_CODE('stop'),	///< Il faut arrêter le serveur.
		kWaitingConnection	= FOUR_CHAR_CODE('wait'),	///< Ayé, on attend le Newton (inData: l'objet TDCLCommLayer)
		kIncomingRequest	= FOUR_CHAR_CODE('toc '), 	///< Une connexion vient d'arriver (inData: l'objet TDCLCommLayer)
		kDisconnected		= FOUR_CHAR_CODE('disc'), 	///< Le lien a été déconnecté (inData: l'objet TDCLLink)

		// Messages reçus par le lien.
		kDisconnectLink		= FOUR_CHAR_CODE('disl'), 	///< Il faut déconnecter le lien.
		kCloseLink			= FOUR_CHAR_CODE('clsl'),	///< Il faut fermer le lien (il a été déconnecté).
		kBytesAvailable		= FOUR_CHAR_CODE('byta'),	///< Des octets sont disponibles.

		// Commandes
		kInstallPackage 	= FOUR_CHAR_CODE('inst'),	///< Commande pour installer un paquet.
										///< Le paramètre est un TFile.
										///< L'objet doit être un TDCLAppCmdFile.
		kStartUsingKeyboard = FOUR_CHAR_CODE('skbd'),	///< Commande pour commencer à utiliser le clavier.
		kEndUsingKeyboard 	= FOUR_CHAR_CODE('ekbd'),	///< Commande pour arrêter d'utiliser le clavier.
		kSendCharacter 		= FOUR_CHAR_CODE('kbdc'),	///< Commande pour envoyer un caractère.
										///< Le paramètre est constitué de deux mots de 16 bits.
										///< L'objet doit être un TDCLAppKbdChar
		kSendString 		= FOUR_CHAR_CODE('kbds'),	///< Commande pour envoyer une chaîne.
										///< Le paramètre est une chaîne.
										///< L'objet doit être un TDCLAppKbdString
		kSynchronize	 	= FOUR_CHAR_CODE('sync')	///< Commande pour synchroniser.
	};

	///
	/// Constructeur à partir d'un type, d'un ID et de données associées.
	///
	/// \param inEventKind	type (pour filtrer avec WaitNextEvent)
	/// \param inEventID	ID de l'événement
	/// \param inData		données associées à l'événement.
	///
	TDCLEvent( EEventKind inEventKind, EEventID inEventID, void* inData = nil );

	///
	/// Constructeur par copie.
	/// Ne copie pas les pointeurs sur les autres événements.
	///
	/// \param inCopy		objet à copier
	///
	TDCLEvent( const TDCLEvent& inCopy );

	///
	/// Destructeur.
	///
	virtual ~TDCLEvent( void );

	///
	/// Accesseur sur le type de l'événement
	///
	/// \return	le type de cet événement
	///
	inline EEventKind	GetEventKind( void ) const
		{
			return mEventKind;
		}

	///
	/// Accesseur sur l'ID de l'événement
	///
	/// \return	l'ID de cet événement
	///
	inline EEventID		GetEventID( void ) const
		{
			return mEventID;
		}

	///
	/// Accesseur sur les données de l'événement
	///
	/// \return	les données de cet événement
	///
	inline void*		GetEventData( void )
		{
			return mEventData;
		}

protected:
	///
	/// Sélecteur sur les données de l'événement
	///
	/// \param inEventData	nouvelles données de l'événement.
	///
	inline void			SetEventData( void* inEventData )
		{
			mEventData = inEventData;
		}

private:
	///
	/// La classe TDCLThread est une amie.
	/// C'est elle qui gère la liste d'événements.
	///
	friend class TDCLThread;
	
	///
	/// Accesseur sur l'événement suivant.
	///
	/// \return	le pointeur sur l'événement suivant
	///
	inline TDCLEvent*	GetNextEvent( void ) const
		{
			return mNextEvent;
		}

	///
	/// Sélecteur pour l'événement suivant.
	///
	/// \param	inNextEvent	événement suivant.
	///
	inline void	SetNextEvent( TDCLEvent* inNextEvent )
		{
			mNextEvent = inNextEvent;
		}

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLEvent& operator = ( const TDCLEvent& inCopy );

	/// \name Variables
	EEventKind		mEventKind;	///< Type de l'événement.
	EEventID		mEventID;	///< ID de l'événement.
	void*			mEventData;	///< Données associées à l'événement.
	TDCLEvent*		mNextEvent;	///< Pointeur sur l'événement suivant
};

#endif
		// _TDCLEVENT_H

// =================================================== //
// You can tune a piano, but you can't tuna fish.      //
//                                                     //
// You can tune a filesystem, but you can't tuna fish. //
//                 -- from the tunefs(8) man page      //
// =================================================== //
