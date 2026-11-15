// ==============================
// Fichier:			TDCLException.h
// Projet:			Desktop Connection Library
//
// Créé le:			11/02/2001
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
// The Original Code is TDCLException.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 2001-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLException.h,v 1.5 2006/02/12 01:23:56 pguyot Exp $
// ===========

#ifndef _TDCLEXCEPTION_H
#define _TDCLEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Headers/DCLErrorMessages.h>

///
/// Classe de base pour les exceptions dans la DCL.
///
/// Stratégie pour les exceptions:
/// Il existe plusieurs classes d'exception, chaque classe correspondant à un
/// code qui peut-être obtenu ici.
///
/// Toutes les exceptions peuvent aussi avoir un code d'erreur venant de la
/// plateforme.
///
/// Suivant le drapeau \c kDCLDebugFILEandLINE, les exceptions comprennent
/// le nom du fichier et la ligne (il y a deux constructeurs).
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.5 $
///
/// \test	aucun test défini.
///
class TDCLException
{
public:

	///
	/// Codes pour les classes d'exception.
	///
	enum ExCode {
		/// \name Errors
		exBadParamError,		///< TDCLBadParamError
		exBadStateError,		///< TDCLBadStateError
		exLimitReachedError,	///< TDCLLimitReachedError
		exMemError,				///< TDCLMemError
		exNotImplementedError,	///< TDCLNotImplementedError
		exUnknownError,			///< TDCLUnknownError

		/// \name IO Exceptions
		
		exDoesntExist,			///< TDCLDoesntExistException
		exEOF,					///< TDCLEOFException
		exExistsAlready,		///< TDCLExistsAlreadyException
		exFull,					///< TDCLFullException
		exPermission,			///< TDCLPermissionException
		exPosition,				///< TDCLPositionException
		exReadOnly,				///< TDCLReadOnlyException
		exTimeout,				///< TDCLTimeoutException
		
		/// \name Link Exceptions
		exBadAppCmd,			///< TDCLBadAppCmdException
		exBadDockCmd,			///< TDCLBadDockCmdException
		exUnexpDockCmd,			///< TDCLUnexpDockCmdException
		
		/// \name Thread Exceptions
		exSemaphoreBusy,		///< TDCLSemaphoreBusyException
		exThreadActive,			///< TDCLThreadActiveException
		exThreadStopped,		///< TDCLThreadStoppedException
		exThreadDeadLock,		///< TDCLThreadDeadLockException
		
		/// \name Other Exceptions
		exClassCast,			///< TDCLClassCastException
		exNewton,				///< TDCLNewtonException
		exNotAvailable,			///< TDCLNotAvailableException
		exNS,					///< TDCLNSException
		exUserCancelled,		///< TDCLUserCancelledException

		/// \name Inspector Exceptions
		exBadInspectorCmd
	};

	///
	/// Accesseur sur le code d'exception
	///
	/// \return	le code d'exception.
	///
	inline	ExCode			GetExCode( void ) const
		{
			return mExCode;
		}
	
	///
	/// Accesseur sur le code d'erreur associé avec l'exception.
	/// Ce code peut être zéro si aucun code d'erreur n'est associé.
	///
	/// \return	le code d'erreur.
	///
	inline	KSInt32			GetError( void ) const
		{
			return mError;
		}

	///
	/// Détermine le nom de l'exception.
	/// Par défaut, utilise le code.
	///
	virtual const char*		GetExceptionName( void ) const;

#if kDCLDebugFILEandLINE

	///
	/// Accesseur sur le nom du fichier où l'exception a été
	/// lancée.
	///
	/// \return	le nom du fichier. Celui-ci est normalement une constante
	///			générée par le compilateur.
	///
	inline	const char*		GetFileName( void ) const
		{
			return mFileName;
		}

	///
	/// Accesseur sur le numéro de ligne où l'exception a été
	/// lancée.
	///
	/// \return	le numéro de ligne.
	///
	inline	KUInt32			GetLine( void ) const
		{
			return mLine;
		}
#endif

#if kDCLDebugFILEandLINE
	///
	/// Constructeur par copie (si kDCLDebugFILEandLINE n'est pas à 1,
	/// on utilise le constructeur par copie par défaut).
	///
	/// \param inCopy		objet à copier.
	///
	TDCLException( const TDCLException& inCopy );
#endif

	///
	/// Destructor.
	///
	virtual ~TDCLException( void ) {};

protected:
#if kDCLDebugFILEandLINE
	///
	/// Constructeur à partir du code de l'exception, d'un code d'erreur, du
	/// nom de fichier et du numéro de ligne.
	///
	/// \param inExCode		code de l'exception.
	/// \param inError		code d'erreur.
	/// \param inFileName	nom du fichier. N'est pas copié.
	/// \param inLine		numéro de ligne.
	///
	TDCLException(
			ExCode inExCode,
			KSInt32 inError,
			const char* inFileName,
			KUInt32 inLine );
#else
	///
	/// Constructeur à partir du code de l'exception et de l'erreur seuls.
	///
	/// \param inExCode		code de l'exception.
	/// \param inError		code d'erreur
	///
	TDCLException( ExCode inExCode, KSInt32 inError );
#endif

private:
	///
	/// Constructeur par défaut non disponible.
	///
	TDCLException( void );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLException& operator = ( const TDCLException& inCopy );

	/// \name Variables

	ExCode					mExCode;		///< Code de l'exception
	KSInt32					mError;			///< Erreur liée à l'exception
#if kDCLDebugFILEandLINE
	const char*				mFileName;		///< Nom du fichier où l'exception
											///< a été lancée
	KUInt32					mLine;			///< Numéro de la ligne.
#endif
};

#endif
		// _TDCLEXCEPTION_H

// =============================================================================== //
// Real Programmers don't write in FORTRAN.  FORTRAN is for pipe stress freaks and //
// crystallography weenies.  FORTRAN is for wimp engineers who wear white socks.   //
// =============================================================================== //
