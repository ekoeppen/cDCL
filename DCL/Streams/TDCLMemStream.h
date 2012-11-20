// ==============================
// Fichier:			TDCLMemStream.h
// Projet:			Desktop Connection Library
//
// Créé le:			19/10/2002
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
// The Original Code is TDCLMemStream.h.
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
// $Id: TDCLMemStream.h,v 1.6 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef __TDCLMEMSTREAM__
#define __TDCLMEMSTREAM__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLRandomAccessStream.h>

///
/// Classe pour un flux maintenu en mémoire.
/// L'écriture remplit la mémoire tampon.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.6 $
///
/// \test	aucun test défini.
///
class TDCLMemStream
	:
		public TDCLRandomAccessStream
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLMemStream( void );

	///
	/// Constructeur à partir d'une mémoire tampon.
	/// Les données sont copiées.
	///
	/// \param inBuffer		mémoire tampon où se trouvent les données
	///						à copier.
	/// \param inLength		taille en octets de cette mémoire tampon.
	///
	TDCLMemStream( const void* inBuffer, KUInt32 inLength );

	///
	/// Destructeur.
	///
	virtual ~TDCLMemStream( void );

	/// \name interface TDCLRandomAccessStream
	
	///
	/// Détermine la position du curseur dans le flux.
	///
	/// \return la position du curseur par rapport au début du flux.
	///
	virtual KSInt64 GetCursor( void );

	///
	/// Déplace le curseur dans le flux. Peut lancer une exception
	/// pour indiquer que la fin du flux a été atteinte.
	///
	/// \param inPos	nouvelle position du curseur
	/// \param inMode	définit l'origine de \c inPos
	/// \throws TDCLPositionException si la fin du flux (ou le début) est
	///			dépassée.
	///
	virtual void SetCursor( KSInt64 inPos, ECursorMode inMode );

	/// \name interface entrée/sortie

	///
	/// Lit des octets et avance le curseur dans la mémoire tampon.
	///
	/// \param outBuffer	mémoire tampon pour les octets lus.
	/// \param ioCount		nombre d'octets à lire en entrée, lus en sortie.
	///						Cette valeur est mise à jour avant que 
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLIOException	si un problème est survenu (autre que EOF)
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
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Vide la mémoire tampon de sortie.
	///
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual void		FlushOutput( void );

	///
	/// Accesseur sur la mémoire tampon.
	///
	/// \return le pointeur sur la mémoire tampon.
	///
	inline const void*	GetBuffer( void )
		{
			return mBuffer;
		}

	///
	/// Accesseur sur la taille de la mémoire tampon.
	///
	/// \return la taille de la mémoire tampon.
	///
	inline KUInt32		GetBufferSize( void )
		{
			return mBufferSize;
		}

	///
	/// Récupère l'octet suivant sans avancer le curseur.
	///
	/// \return l'octet lu.
	/// \throws TDCLIOException	si un problème est survenu (y compris si la
	///							fin du fichier est rencontrée)
	///
	virtual	KUInt8		PeekByte( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMemStream( const TDCLMemStream& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMemStream& operator = ( const TDCLMemStream& inCopy );

	///
	/// Constantes pour cette classe.
	///
	enum {
		kSizeIncrement	= 256	///< Nombre d'octets pour l'incrément
								///< de la mémoire tampon.
	};
	
	/// \name Variables

	void*		mBuffer;		///< Mémoire tampon
	KUInt32		mBufferSize;	///< Taille de la mémoire tampon
	KUInt32		mAllocatedSize;	///< Taille allouée de la mémoire
								///< tampon
	KUInt32		mCursor;		///< Curseur dans la mémoire tampon
								///< pour la lecture et l'écriture.
};

#endif
		// __TDCLMEMSTREAM__

// ===================================================================== //
// I have never seen anything fill up a vacuum so fast and still suck.   //
//                 -- Rob Pike, on X.                                    //
//                                                                       //
// Steve Jobs said two years ago that X is brain-damaged and it will be  //
// gone in two years.  He was half right.                                //
//                 -- Dennis Ritchie                                     //
//                                                                       //
// Dennis Ritchie is twice as bright as Steve Jobs, and only half wrong. //
//                 -- Jim Gettys                                         //
// ===================================================================== //
