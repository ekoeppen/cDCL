// ==============================
// Fichier:			TDCLNSOFDecoder.h
// Projet:			Desktop Connection Library
//
// Créé le:			22/10/2002
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
// The Original Code is TDCLNSOFDecoder.h.
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
// $Id: TDCLNSOFDecoder.h,v 1.3 2004/11/24 14:09:05 paul Exp $
// ===========

#ifndef __TDCLNSOFDECODER__
#define __TDCLNSOFDECODER__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Exchange/TDCLNSDecoder.h>
#include <DCL/NS_Objects/Exchange/KDCLNSOF.h>

///
/// Classe pour le décodage d'objets NS au format NSOF.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	UTestNSOF
///
class TDCLNSOFDecoder
			:
				public TDCLNSDecoder,
				public KDCLNSOF
{
public:
	///
	/// Constructeur à partir d'un flux.
	/// Commence à lire le flux (la version).
	///
	/// \param inStream		flux où lire les données.
	///
	TDCLNSOFDecoder( TDCLStream* inStream );

	///
	/// Destructeur.
	///
	virtual ~TDCLNSOFDecoder( void );

	///
	/// Crée l'objet suivant.
	///
	/// \return une référence vers l'objet suivant.
	///
	virtual TDCLNSRef	GetNextObject( void );
};

#endif
		// __TDCLNSOFDECODER__

// ========================================================================== //
// There are two ways to write error-free programs; only the third one works. //
// ========================================================================== //
