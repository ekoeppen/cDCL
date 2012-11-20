// ==============================
// Fichier:			TDCLPipeProxy.h
// Projet:			Desktop Connection Library
//
// Créé le:			5/4/2003
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
// The Original Code is TDCLPipeProxy.h.
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
// $Id: TDCLPipeProxy.h,v 1.3 2004/11/24 14:08:54 paul Exp $
// ===========

#ifndef _TDCLPIPEPROXY_H
#define _TDCLPIPEPROXY_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipe.h>

///
/// Classe pour un mandataire sur une connexion. Utilisée pour la compression
/// MNP et pour l'utilisation d'une mémoire tampon.
///
/// Les méthodes de lecture et d'écriture doivent être mises en places par
/// les sous-classes. Pareil pour les méthodes BytesAvailable et FlushOutput.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLPipeProxy
	:
		public TDCLPipe
{
public:
	///
	/// Constructeur à partir d'une connexion.
	/// Cette connexion sera détruite par le destructeur de cette classe.
	///
	/// \param inSubPipe	connexion dont cette classe est le mandataire.
	///
	TDCLPipeProxy( TDCLPipe* inSubPipe );

	///
	/// Destructeur.
	/// Détruit la connexion dont cette classe est le mandataire.
	///
	virtual ~TDCLPipeProxy( void );
	
	///
	/// Détermine si des octets sont disponibles dans la mémoire tampon d'entrée.
	/// Appelle la méthode de la couche de connexion dont
	/// cette classe est le mandataire.
	///
	/// \return \c true s'il y a des octets disponibles, \c false sinon.
	///
	virtual	Boolean		BytesAvailable( void );

	///
	/// Déconnecte le canal de communication avec le client.
	///
	/// \remark	une fois que cette méthode est appelée, l'objet connexion est inutilisable.
	/// 		Il peut être supprimé par l'appelant à tout instant.
	///
	virtual	void		Disconnect( void );

	///
	/// Méthode appelée pour indiquer que c'est le moment de perdre un peu
	/// de temps.
	///
	/// Certaines couches de communication ont besoin de ceci.
	///
	virtual	void		Idle( void );

	///
	/// Récupère la temporisation pour cette connexion.
	/// La temporisation est exprimée en secondes ou c'est une des constantes
	/// définies plus haut.
	///
	/// \return la temporisation au format TDCLCommLayer.
	///
	virtual	long		GetTimeout( void );

	///
	/// Change la temporisation pour cette connexion.
	/// La temporisation est exprimée en secondes ou c'est une des constantes
	/// définies plus haut.
	///
	/// \param inTimeout	temporisation au format TDCLCommLayer.
	///
	virtual	void		SetTimeout( long inTimeout );

	///
	/// Récupère l'octet suivant sans avancer le curseur.
	///
	/// \return l'octet lu.
	/// \throws TDCLException si l'opération n'est pas possible.
	///
	virtual	KUInt8		PeekByte( void );

	///
	/// Méthode appelée par le lien pour dire qu'il est connecté
	/// via cette connexion. Cette méthode est appelée juste avant
	/// que des données soient échangées.
	///
	/// Par défaut règle mLink.
	///
	/// \param inLink	lien qui est connecté.
	/// \return l'interface de communication.
	///
	virtual	TDCLCommLayer*	Connected( TDCLLink* inLink );

	///
	/// Méthode appelée par le lien pour dire qu'il souhaite
	/// être prévenu dès que des données sont présentes.
	/// Au départ, l'interface de communication ne prévient
	/// pas le lien de la présence de données.
	///
	virtual	void		ClearDataPresent( void );

protected:
	///
	/// Accesseur sur la connexion dont cette classe est le
	/// mandataire.
	///
	/// \return le pointeur sur l'objet connexion.
	///
	inline TDCLPipe*	GetSubPipe( void )
		{
			return mSubPipe;
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPipeProxy( const TDCLPipeProxy& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPipeProxy& operator = ( const TDCLPipeProxy& inCopy );

	/// \name Variables.
	
	TDCLPipe*		mSubPipe;		///< Connexion dont cette classe est le mandataire.
};

#endif
		// _TDCLPIPEPROXY_H

// ================================================ //
// You will have a head crash on your private pack. //
// ================================================ //
