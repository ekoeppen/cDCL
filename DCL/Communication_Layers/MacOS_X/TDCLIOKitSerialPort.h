// ==============================
// Fichier:			TDCLIOKitSerialPort.h
// Projet:			Desktop Connection Library
//
// Créé le:			4/4/2003
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
// The Original Code is TDCLIOKitSerialPort.h.
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
// $Id: TDCLIOKitSerialPort.h,v 1.3 2004/11/24 14:08:55 paul Exp $
// ===========

#ifndef _TDCLIOKITSERIALPORT_H
#define _TDCLIOKITSERIALPORT_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/POSIX/TDCLFDSerialPort.h>

// ANSI C
#include <stdlib.h>

// MacOS X
#include <CoreFoundation/CFString.h>
#include <IOKit/IOKitLib.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Exceptions/TDCLNotAvailableException.h>

///
/// Classe pour des ports série sous MacOS X dont on obtient le nom
/// avec l'IOKit. Cette classe ne fait en fait pas grand chose sinon itérer sur les ports
/// série disponibles.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.3 $
///
/// \test	aucun test défini.
///
class TDCLIOKitSerialPort
	:
		public TDCLFDSerialPort
{
public:
	///
	/// Constructeur à partir d'un indice.
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inIndex			indice du port
	/// \param inBaud			vitesse du port
	///
	TDCLIOKitSerialPort(
				IDCLThreads* inThreadsIntf,
				KUInt32 inIndex,
				long inBaud = 38400 );

	///
	/// Constructeur à partir d'un chemin BSD sous forme de chaîne CFStringRef.
	/// C'est le constructeur préféré (stocker l'indice du port n'est pas tip top).
	///
	/// \param inThreadsIntf	interface pour les processus légers.
	/// \param inDevPath		chemin du port
	/// \param inBaud			vitesse du port
	///
	TDCLIOKitSerialPort(
				IDCLThreads* inThreadsIntf,
				CFStringRef inDevPath,
				long inBaud = 38400 );

	///
	/// Destructeur.
	///
	virtual ~TDCLIOKitSerialPort( void );

	///
	/// Détermine le nom d'un port série. Utile pour être sympa avec l'utilisateur.
	///
	/// \param inIndex			index du port série (base 0)
	/// \return le nom du port ou \c nil si le port n'existe pas.
	///
	static	CFStringRef	GetSerialPortName( KUInt32 inIndex );

	///
	/// Détermine le nom d'un port série avec son nom. Utile pour être sympa avec
	/// l'utilisateur. Cette méthode retourne \c nil si le port n'existe pas.
	/// Remarque: cette méthode compare avec la base I/O Kit.
	///
	/// \param inDevPath		chemin BSD du port série.
	/// \return le nom du port ou \c nil si le port n'existe pas.
	///
	static	CFStringRef	GetSerialPortName( CFStringRef inDevPath );

	///
	/// Détermine si un port série existe à partir de son chemin.
	/// Remarque: cette méthode compare avec la base I/O Kit.
	///
	/// \param inDevPath		chemin BSD du port série.
	/// \return \c true si le port existe, \c false sinon.
	///
	static	Boolean		IsAvailable( CFStringRef inDevPath );

	///
	/// Détermine le chemin BSD d'un port série.
	///
	/// \param inIndex			index du port série (base 0)
	/// \return le chemin BSD du port ou \c nil si le port n'existe pas.
	///
	static	CFStringRef	GetSerialPortPath( KUInt32 inIndex );

	///
	/// Détermine le nombre de ports série disponibles.
	///
	/// \return le nombre de ports série (n) sur cette machine. L'index du port
	///			pour les autres méthodes doit être entre 0 et n - 1.
	///
	static	KUInt32	CountSerialPorts( void );

private:
	///
	/// Récupère un itérateur sur les ports série.
	///
	/// \param outIterator	en sortie, itérateur sur les ports série. L'itérateur
	///						doit être libéré par l'appelant. Il n'est
	///						pas alloué si une exception est lancée.
	/// \throws TDCLNotAvailableException s'il n'y a pas de port série.
	///
	static void GetSerialPortsIterator( io_iterator_t* outIterator );

	///
	/// Transforme une chaîne CFStringRef en chaîne US-ASCII allouée avec malloc.
	///
	/// \param inStringRef	chaîne CFStringRef.
	/// \return la chaîne en US-ASCII allouée avec malloc
	///
	static	char*	CFStringRefToCStr( CFStringRef inStringRef );
};

#endif
		// _TDCLIOKITSERIALPORT_H

// ==================================================================== //
// Real Users are afraid they'll break the machine -- but they're never //
// afraid to break your face.                                           //
// ==================================================================== //