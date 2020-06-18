// ==============================
// Fichier:			TDCLBufferedPipe.h
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
// The Original Code is TDCLBufferedPipe.h.
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
// $Id: TDCLBufferedPipe.h,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#ifndef __TDCLBUFFEREDPIPE__
#define __TDCLBUFFEREDPIPE__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipeProxy.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>

///
/// Classe pour une connexion avec mémoire tampon. Utilisable surtout
/// pour les communications par paquets.
/// La mémoire tampon ne sert qu'en sortie.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLBufferedPipe
	:
		public TDCLPipeProxy
{
public:
	///
	/// Constantes pour cette classe.
	///
	enum {
		kDefaultBufferCapacity	=	1024	///< 1 KB
	};
	
	///
	/// Constructeur à partir d'une connexion
	/// et d'une taille de mémoire tampon.
	///
	/// \param inSubPipe		connexion pour laquelle cette mémoire tampon sert
	/// \param inBufferCapacity	taille de la mémoire tampon.
	///
	TDCLBufferedPipe(
			TDCLPipe* inSubPipe,
			KUInt32 inBufferCapacity = kDefaultBufferCapacity );

	///
	/// Destructeur.
	///
	virtual ~TDCLBufferedPipe( void );

	///
	/// Lit des octets.
	///
	/// \param outBuffer	mémoire tampon pour les octets lus.
	/// \param ioCount		nombre d'octets à lire en entrée, lus en sortie.
	///						Cette valeur est mise à jour avant que 
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLException	si un problème est survenu (autre que EOF)
	///
	virtual	void		Read( void* outBuffer, KUInt32* ioCount );

	///
	/// Ecrit des octets.
	///
	/// \param inBuffer		mémoire tampon pour les octets à écrire.
	/// \param ioCount		nombre d'octets à écrire en entrée, écris en
	///						sortie. Cette valeur est mise à jour avant que
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLException	si un problème est survenu.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Vide la mémoire tampon de sortie.
	///
	/// \throws TDCLException	si un problème est survenu.
	///
	virtual void		FlushOutput( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLBufferedPipe( const TDCLBufferedPipe& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLBufferedPipe& operator = ( const TDCLBufferedPipe& inCopy );

	/// \name Variables
	void*				mBuffer;			///< Mémoire tampon
	KUInt32				mBufferSize;		///< Utilisation de la mémoire
											///< tampon
	KUInt32				mBufferCapacity;	///< Taille de la mémoire tampon
};

#endif
		// __TDCLBUFFEREDPIPE__

// =========================================================================== //
// Our documentation manager was showing her 2 year old son around the office. //
// He was introduced to me, at which time he pointed out that we were both     //
// holding bags of popcorn.  We were both holding bottles of juice.  But only  //
// *__he* had a lollipop.                                                    //
//         He asked his mother, "Why doesn't HE have a lollipop?"              //
//         Her reply: "He can have a lollipop any time he wants to.  That's    //
// what it means to be a programmer."                                          //
// =========================================================================== //
