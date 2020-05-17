// ==============================
// Fichier:			TDCLFileImportEngine.h
// Projet:			Desktop Connection Library
//
// Créé le:			17/4/2003
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
// The Original Code is TDCLFileImportEngine.h.
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
// $Id: TDCLFileImportEngine.h,v 1.3 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef _TDCLFILEIMPORTENGINE_H
#define _TDCLFILEIMPORTENGINE_H

///
/// Classe pour .
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLFileImportEngine
{
public:
	/// \name Constantes publiques

	/// \name Constructeurs et destructeur

	///
	/// Constructeur par défaut.
	///
	TDCLFileImportEngine( void );

	///
	/// Destructeur.
	///
	virtual ~TDCLFileImportEngine( void );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFileImportEngine( const TDCLFileImportEngine& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLFileImportEngine& operator = ( const TDCLFileImportEngine& inCopy );

	/// \name Variables

};

#endif
		// _TDCLFILEIMPORTENGINE_H

// ============================================================================== //
// Dear Emily:                                                                    //
//         I recently read an article that said, "reply by mail, I'll summarize." //
// What should I do?                                                              //
//                 -- Doubtful                                                    //
//                                                                                //
// Dear Doubtful:                                                                 //
//         Post your response to the whole net.  That request applies only to     //
// dumb people who don't have something interesting to say.  Your postings are    //
// much more worthwhile than other people's, so it would be a waste to reply by   //
// mail.                                                                          //
//                 -- Emily Postnews Answers Your Questions on Netiquette         //
// ============================================================================== //
