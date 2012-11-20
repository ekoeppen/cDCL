// ==============================
// Fichier:			TDCLPicture.h
// Projet:			Desktop Connection Library
// 
// Créé le:			20/3/2004
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
// The Original Code is TDCLPicture.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPicture.h,v 1.3 2004/11/24 14:08:56 paul Exp $
// ===========

#ifndef _TDCLPICTURE_H
#define _TDCLPICTURE_H

#include <DCL/Headers/DCLDefinitions.h>
#include <K/Misc/TRefCounter.h>

///
/// Interface pour une image.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLPicture
	:
		private TRefCounter
{
	/// La classe qui s'occupe des références est une amie.
	friend class TDCLStyleRef;

public:
	///
	/// Destructeur.
	///
	virtual ~TDCLPicture( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPicture( const TDCLPicture& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPicture& operator = ( const TDCLPicture& inCopy );

	/// \name Variables

};

#endif
		// _TDCLPICTURE_H

// ======================================================================== //
// Our informal mission is to improve the love life of operators worldwide. //
//                 -- Peter Behrendt, president of Exabyte                  //
// ======================================================================== //
