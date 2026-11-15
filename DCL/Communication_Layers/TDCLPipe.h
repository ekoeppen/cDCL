// ==============================
// Fichier:			TDCLPipe.h
// Projet:			Desktop Connection Library
//
// Créé le:			26/10/2002
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
// The Original Code is TDCLPipe.h.
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
// $Id: TDCLPipe.h,v 1.4 2004/11/24 14:08:53 paul Exp $
// ===========

#ifndef __TDCLPIPE__
#define __TDCLPIPE__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStream.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>

// Pré-déclarations.
class TDCLPipeProxy;
class TDCLLink;

///
/// Classe pour une connexion physique avec le Newton.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.4 $
///
/// \test	aucun test défini.
///
class TDCLPipe
	:
		public TDCLStream
{
public:
	///
	/// Constructeur.
	/// Remplit les champs privés.
	///
	/// \param inCommLayer	couche de communication à laquelle est
	///						rattachée la connexion.
	///
	TDCLPipe( TDCLCommLayer* inCommLayer );

	///
	/// Destructeur.
	/// Celui-ci ne fait rien. Le destructeur des sous-classes doit fermer
	/// la connexion si ceci n'est pas déjà fait.
	///
	virtual	~TDCLPipe( void ) {}

	///
	/// Détermine si des octets sont disponibles dans la mémoire tampon
	/// d'entrée.
	///
	/// \return \c true s'il y a des octets disponibles, \c false sinon.
	///
	virtual	Boolean		BytesAvailable( void ) = 0;
	
	///
	/// Vide la mémoire tampon de sortie et retourne une fois que toutes les
	/// données ont été envoyées.
	///
	/// Utile pour s'assurer que tout a été envoyé.
	/// L'implémentation par défaut ne fait rien.
	///
	/// \throws TDCLException	si un problème est survenu.
	///
	virtual	void		FlushOutput( void );
	
	///
	/// Déconnecte le canal de communication avec le client.
	///
	/// L'implémentation par défaut appelle la méthode
	/// \c Disconnected( TDCLPipe* ) de TDCLCommLayer.
	///
	/// \remark	une fois que cette méthode est appelée, l'objet connexion est
	///			inutilisable. Il peut être supprimé par l'appelant à tout
	///			instant.
	///
	virtual	void		Disconnect( void );

	///
	/// Détermine si cette connexion sait vider la mémoire tampon de sortie.
	///
	/// Certaines couches de communication ne savent pas vider la mémoire tampon
	/// de sortie. Dans ce cas, l'appelant devra attendre un petit peu avant
	/// d'appeler Disconnect.
	/// \remark il faut quand même appeler FlushOutput. En effet, si les données
	///			sont mise dans une mémoire tampon, il faut vider cette mémoire
	///			tampon.
	///
	/// \return	\c true si la couche de communication sait vider la mémoire
	///			tampon de sortie, \c false sinon.
	///
	inline	Boolean		FlushAvailable( void )
		{
			return mCommLayer->FlushAvailable();
		}

	///
	/// Méthode appelée pour indiquer que c'est le moment de perdre un peu
	/// de temps.
	///
	/// Certaines couches de communication ont besoin de ceci. L'implémentation
	/// par défaut appelle la méthode Idle( void ) de la couche de
	/// communication.
	///
	virtual	void		Idle( void );

	///
	/// Récupère la temporisation pour cette connexion.
	/// La temporisation est exprimée en secondes ou c'est une des constantes
	/// définies plus haut.
	///
	/// \return la temporisation au format TDCLCommLayer.
	///
	virtual	long		GetTimeout( void ) = 0;

	///
	/// Change la temporisation pour cette connexion.
	/// La temporisation est exprimée en secondes ou c'est une des constantes
	/// définies plus haut.
	///
	/// \param inTimeout	temporisation au format TDCLCommLayer.
	///
	virtual	void		SetTimeout( long inTimeout ) = 0;

	///
	/// Récupère l'octet suivant sans avancer le curseur.
	/// Par défaut, ceci n'est pas possible. Renvoie donc une exception.
	///
	/// \return l'octet lu.
	/// \throws TDCLException
	///
	virtual	KUInt8		PeekByte( void );

	///
	/// Méthode appelée par le lien pour dire qu'il est connecté via cette
	/// connexion. Cette méthode est appelée juste avant que des données soient
	/// échangées. Cette méthode retourne l'interface de communication comme ça
	/// le lien peut la transmettre à l'application.
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
	/// Remarque: chaque appel à cette méthode induit (au moins) un message
	/// envoyé au lien. Comme les messages sont asynchrones, le lien doit
	/// vérifier lorsqu'il a un tel message que des données sont présentes
	/// (en appelant BytesAvailable).
	///
	virtual	void		ClearDataPresent( void ) = 0;

protected:
	///
	/// Le constructeur de la classe mandataire a besoin d'accéder à la couche
	/// de communication.
	///
	friend class TDCLPipeProxy;

	///
	/// Accesseur sur la couche de communication.
	///
	/// Il est protégé parce que les appelant n'ont pas besoin d'y avoir accès.
	///
	/// \return	la couche de communication associée à cette connexion.
	///
	inline	TDCLCommLayer*	GetCommLayer( void )
		{
			return mCommLayer;
		}

	///
	/// Accesseur sur le lien associé (\c nil lorsqu'on n'est pas connecté).
	///
	/// \return le lien associé à cette connexion.
	///
	inline	TDCLLink*	GetLink( void ) const
		{
			return mLink;
		}
		
	///
	/// Sélecteur sur le lien associé.
	///
	/// \param inLink	nouveau lien
	///
	inline	void		SetLink( TDCLLink* inLink )
		{
			mLink = inLink;
		}

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPipe( const TDCLPipe& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPipe& operator = ( const TDCLPipe& inCopy );

	/// \name Variables.
	
	TDCLCommLayer*	mCommLayer;	///< Couche de communication associée à cette
								///< connexion.
	TDCLLink*		mLink;		///< Lien associé à cette connexion.
};

#endif
		// __TDCLPIPE__

// =================================================================== //
// "One basic notion underlying Usenet is that it is a cooperative."   //
//                                                                     //
// Having been on USENET for going on ten years, I disagree with this. //
// The basic notion underlying USENET is the flame.                    //
//                 -- Chuq Von Rospach                                 //
// =================================================================== //
