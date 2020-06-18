// ==============================
// Fichier:			TDCLOTSerial.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/01/2002
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
// The Original Code is TDCLOTSerial.h.
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
// $Id: TDCLOTSerial.h,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef __TDCLOTSERIAL__
#define __TDCLOTSERIAL__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/MacOS/TDCLOpenTransport.h>

///
/// Classe pour la couche de communication port série avec OpenTransport
/// (avec compression MNP fournie par TDCLMNPPipe).
/// 
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
/// \todo	à terminer (il faut surcharger la méthode qui crée les EP et
///			avoir notre propre classe d'EP)
///
class TDCLOTSerial
	:
		public TDCLOpenTransport
{
public:
	///
	/// Constructeur.
	///
	/// \param inThreadsIntf	interface pour les threads (utilisée pour Yield).
	/// \param inPort			index du port série (base 0)
	/// \param inBaud			vitesse du port série (par défaut, 38400, la
	///							vitesse par défaut sur le Newton)
	/// \param inTimeout		délai de temporisation, format CommLayer.
	///
	TDCLOTSerial(
			IDCLThreads* inThreadsIntf,
			unsigned long inPort,
			long inBaud = 38400,
			long inTimeout = kDefaultTimeout
		);

	///
	/// Vérifie que le service est disponible.
	///
	/// \return \c true si le service est disponible, \c false sinon.
	///
	static	Boolean		IsAvailable( void );

	///
	/// Détermine le nom d'un port série. Utile pour être sympa avec l'utilisateur.
	/// Retourne une chaîne allouée avec malloc.
	///
	/// \param inIndex			index du port série (base 0)
	/// \return une chaîne allouée avec malloc ou nil si le port n'existe pas.
	///
	static	char*		GetSerialPortName( unsigned long inIndex );

	///
	/// Détermine le nombre de ports série disponibles.
	///
	/// \return le nombre de ports série (n) sur cette machine. L'index du port
	///			pour les autres méthodes doit être entre 0 et n - 1.
	///
	static	unsigned long	CountSerialPorts( void );

	///
	/// Retourne un objet connexion représentant le canal de communication
	/// avec le client ou \c nil s'il n'y a pas de requête de connexion
	/// présente.
	/// Ce doit être un objet créé avec new. L'appelant devra le supprimer.
	///
	/// \return	un nouvel objet TDCLPipe créé avec new représentant la connexion
	///			ou \c nil.
	///
	virtual	TDCLPipe*	Accept( void );

private:
	
	///
	/// Détermine la chaîne de configuration pour un port série donné et
	/// le nom à montrer à l'utilisateur. Si le port n'existe pas, aucun
	/// des paramètres n'est modifié.
	///
	/// \param inIndex			index du port série (base 0)
	/// \param outConfigStr		chaîne (allouée par l'appelant) pour la
	///							configuration. Si elle vaut \c nil, elle
	///							n'est pas remplie.
	/// \param outNameStr		pointeur vers une chaîne allouée par cette
	///							méthode avec malloc qui contient le nom
	///							du port série. Rien n'est alloué si ce
	///							pointeur vaut nil.
	/// \return \c true si le port a été trouvé, \c false sinon.
	///
	static	Boolean		GetSerialPortInfos(
										unsigned long inIndex,
										char outConfigStr[36],
										char** outNameStr );

	/// \name variables privées
	long				mBaud;	///< vitesse du port série
};

#endif
		// __TDCLOTSERIAL__

// ====================================================================== //
// Documentation is like sex: when it is good, it is very, very good; and //
// when it is bad, it is better than nothing.                             //
//                 -- Dick Brandon                                        //
// ====================================================================== //