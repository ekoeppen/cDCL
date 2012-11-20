// ==============================
// Fichier:			TDCLStdStream.h
// Projet:			Desktop Connection Library
//
// Créé le:			16/10/2002
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
// The Original Code is TDCLStdStream.h.
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
// $Id: TDCLStdStream.h,v 1.5 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef __TDCLSTDSTREAM__
#define __TDCLSTDSTREAM__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Streams/TDCLStream.h>

///
/// Classe pour un flux sur stdin/stdout.
/// Utilisé en partie pour les tests.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLStdStream
	:
		public TDCLStream
{
public:
	///
	/// Constructeur par défaut.
	///
	TDCLStdStream( void );

	///
	/// Destructeur.
	///
	virtual ~TDCLStdStream( void );

	/// \name interface entrée/sortie

	///
	/// Lit des octets.
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
	/// Récupère l'octet suivant sans avancer le curseur.
	///
	/// \return l'octet lu.
	/// \throws TDCLIOException	si un problème est survenu (y compris si la
	///							fin du fichier est rencontrée)
	///
	virtual	KUInt8		PeekByte( void );
};

#endif
		// __TDCLSTDSTREAM__

// ======================================================================== //
// ... computer hardware progress is so fast.  No other technology since    //
// civilization began has seen six orders of magnitude in performance-price //
// gain in 30 years.                                                        //
//                 -- Fred Brooks                                           //
// ======================================================================== //
