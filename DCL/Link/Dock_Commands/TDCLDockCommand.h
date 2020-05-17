// ==============================
// Fichier:			TDCLDockCommand.h
// Projet:			Desktop Connection Library
//
// Créé le:			31/01/2001
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
// The Original Code is TDCLDockCommand.h.
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
// $Id: TDCLDockCommand.h,v 1.7 2004/11/24 14:09:04 paul Exp $
// ===========

#ifndef __TDCLDOCKCOMMAND__
#define __TDCLDOCKCOMMAND__

#include <DCL/Headers/DCLDefinitions.h>

// DCL
#include <DCL/Communication_Layers/TDCLCommLayer.h>

// Pré-déclarations
class TDCLStream;
class TDCLPipe;

///
/// Type d'une fonction pour savoir l'évolution de l'envoi ou de la réception
/// d'une commande. Cette fonction est appelée à intervalles réguliers (et
/// raisonnables).
///
/// \param inProgress	progression (entre 0 et 1)
///
typedef void (*ProgressFuncPtr)(void* inRefCon, double inProgress);

///
/// Classe de base pour les commandes reçues et envoyées au Newton.
/// La classe TDCLPipe envoie et reçoit les commandes.
///
/// - Envoi d'une commande:
///   N'utilisez pas cette classe mais une classe dérivée. Vous pouvez dériver
///   cette classe si vous avez besoin d'envoyer une commande non définie dans
///   la DCL, vous pouvez aussi utiliser la classe \c TDCLDockCmdGeneric à cet
///   effet.
///   Une fois que la commande est prête, envoyez-là avec la méthode
///   \c SendCommand.
///
/// - Réception d'une commande:
///   Vous obtenez normalement une objet dérivé de \c TDCLDockCommand en
///   appelant la méthode ReceiveCommand. La méthode \c GetCommand
///   devrait vous renseigner sur le type de commande, vous pouvez ensuite
///   prendre un pointeur sur la sous-classe de \c TDCLDockCommand qui
///   correspond.
///
/// L'envoi des données se fait par la méthode \c SendCommand. Les
/// sous-classes ont trois stratégies possibles pour envoyer les données,
/// suivant la façon dont les données sont organisées:
///
/// - dériver les méthodes GetLength et GetData
/// - dériver les méthodes SendHeader et SendBody
/// - dériver la méthode SendCommand
///
/// Si une sous-classe ne suit pas au moins une de ces stratégies, une
/// exception sera lancée.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.7 $
///
class TDCLDockCommand
{
public:
	///
	/// Destructeur.
	virtual	~TDCLDockCommand( void );

	///
	/// Reçoit une commande.
	/// Cette méthode est bloquante tant que la commande n'a pas été lue.
	///
	/// \param inPipe				connexion pour lire la commande.
	/// \param inProgressFuncPtr	fonction appelée pour la progression
	///								(\c nil signifie qu'on ne prévient pas)
	/// \param inRefCon				biscuit pour inProgressFuncPtr
	///
	///
	static	TDCLDockCommand*	ReceiveCommand(
									TDCLStream* inPipe,
									ProgressFuncPtr inProgressFuncPtr = nil,
									void* inRefCon = nil );

	///
	/// Reçoit une commande kDResult et retourne le code d'erreur.
	/// Lance une exception si la commande lue n'est pas une commande kDResult.
	/// Cette méthode est bloquante tant que la commande n'a pas été lue.
	///
	/// \param inPipe				connexion pour lire la commande.
	/// \return le code d'erreur de la commande kDResult qui a été lue
	///
	static	KSInt32				GetResultCmd( TDCLPipe* inPipe );

	///
	/// Envoie la commande.
	/// Par défaut, appelle SendHeader, SendBody et complète pour obtenir un
	/// multiple de 4 octets.
	///
	/// \param inPipe				connexion pour envoyer la commande
	/// \param inProgressFuncPtr	fonction appelée pour la progression
	///								(\c nil signifie qu'on ne prévient pas)
	/// \param inRefCon				biscuit pour inProgressFuncPtr
	///
	virtual	void				SendCommand(
									TDCLStream* inPipe,
									ProgressFuncPtr inProgressFuncPtr = nil,
									void* inRefCon = nil );

	///
	/// Constantes pour cette classe.
	///
	enum {

	// Command types (I have a couple of them)
	// (special thanks to Sascha Bendinger)

	// Remote query
	//	All of the commands in this section are based on the NewtonScript query
	//  functions.
	//	Please see the Newton Programmer's Guide for details about the
	//  functions performed by the commands. The query command returns a long
	//  representing the queries cursor.
	//	Each of the other commands take this cursor as a parameter. Entries are
	//  returned with the kDEntry command.

		// Newton -> Desktop Commands
		kDLongData					= FOUR_CHAR_CODE('ldta'),
		kDRefResult					= FOUR_CHAR_CODE('ref '),

		// Desktop -> Newton

		kDQuery						= FOUR_CHAR_CODE('qury'),
		kDCursorGotoKey				= FOUR_CHAR_CODE('goto'),
		kDCursorMap					= FOUR_CHAR_CODE('cmap'),
		kDCursorEntry				= FOUR_CHAR_CODE('crsr'),
		kDCursorMove				= FOUR_CHAR_CODE('move'),
		kDCursorNext				= FOUR_CHAR_CODE('next'),
		kDCursorPrev				= FOUR_CHAR_CODE('prev'),
		kDCursorReset				= FOUR_CHAR_CODE('rset'),
		kDCursorResetToEnd			= FOUR_CHAR_CODE('rend'),
		kDCursorCountEntries		= FOUR_CHAR_CODE('cnt '),
		kDCursorWhichEnd			= FOUR_CHAR_CODE('whch'),
		kDCursorFree				= FOUR_CHAR_CODE('cfre'),

	// Keyboard Passthrough

		// Desktop -> Newton

		kDKeyboardChar				= FOUR_CHAR_CODE('kbdc'),
		kDKeyboardString			= FOUR_CHAR_CODE('kbds'),

		// Desktop -> Newton or Newton -> Desktop
		kDStartKeyboardPassthrough	= FOUR_CHAR_CODE('kybd'),

	// Misc additions

		// Newton -> Desktop Commands

		kDDefaultStore				= FOUR_CHAR_CODE('dfst'),
		kDAppNames					= FOUR_CHAR_CODE('appn'),
		kDImportParameterSlipResult = FOUR_CHAR_CODE('islr'),
		kDPackageInfo 				= FOUR_CHAR_CODE('pinf'),
		kDSetBaseID					= FOUR_CHAR_CODE('base'),
		kDBackupIDs					= FOUR_CHAR_CODE('bids'),
		kDBackupSoupDone 			= FOUR_CHAR_CODE('bsdn'),
		kDSoupNotDirty				= FOUR_CHAR_CODE('ndir'),
		kDSynchronize				= FOUR_CHAR_CODE('sync'),
		kDCallResult				= FOUR_CHAR_CODE('cres'),

		// Desktop -> Newton

		kDRemovePackage				= FOUR_CHAR_CODE('rmvp'),
		kDResultString				= FOUR_CHAR_CODE('ress'),
		kDSourceVersion				= FOUR_CHAR_CODE('sver'),
		kDAddEntryWithUniqueID		= FOUR_CHAR_CODE('auni'),
		kDGetPackageInfo			= FOUR_CHAR_CODE('gpin'),
		kDGetDefaultStore			= FOUR_CHAR_CODE('gdfs'),
		kDCreateDefaultSoup		 	= FOUR_CHAR_CODE('cdsp'),
		kDGetAppNames				= FOUR_CHAR_CODE('gapp'),
		kDRegProtocolExtension		= FOUR_CHAR_CODE('pext'),
		kDRemoveProtocolExtension	= FOUR_CHAR_CODE('rpex'),
		kDSetStoreSignature			= FOUR_CHAR_CODE('ssig'),
		kDSetSoupSignature			= FOUR_CHAR_CODE('ssos'),
		kDImportParametersSlip		= FOUR_CHAR_CODE('islp'),
		kDGetPassword				= FOUR_CHAR_CODE('gpwd'),
		kDSendSoup 					= FOUR_CHAR_CODE('snds'),
		kDBackupSoup				= FOUR_CHAR_CODE('bksp'),
		kDSetStoreName				= FOUR_CHAR_CODE('ssna'),
		kDCallGlobalFunction 		= FOUR_CHAR_CODE('cgfn'),
		kDCallRootMethod			= FOUR_CHAR_CODE('crmd'),
		kDSetVBOCompression		 	= FOUR_CHAR_CODE('cvbo'),
		kDRestorePatch				= FOUR_CHAR_CODE('rpat'),

		// Desktop -> Newton or Newton -> Desktop

		kDOperationDone				= FOUR_CHAR_CODE('opdn'),
		kDOperationCanceled			= FOUR_CHAR_CODE('opcn'),
		kDOpCanceledAck				= FOUR_CHAR_CODE('ocaa'),
		kDRefTest					= FOUR_CHAR_CODE('rtst'),
		kDUnknownCommand			= FOUR_CHAR_CODE('unkn'),

	// Starting a session
	//	2.0 Newton supports a new set of protocols to enhance the connection
	//  capabilities. However, since it's desirable to also support package
	//  downloading from NPI, NTK 1.0 and Connection 2.0 the ROMs will also
	//  support the old protocol for downloading packages. To make this work
	//  the 2.0 ROMs will pretend that they are talking the old protocol when
	//  they send the kDRequestToDock message. If a new connection (or other
	//	app) is on the other end the protocol will be negotiated up to the
	//  current version. Only package loading is supported with the old
	//  protocol.

		// Newton -> Desktop Commands

		/// data = encrypted key
		kDPassword					= FOUR_CHAR_CODE('pass'),
		kDNewtonName				= FOUR_CHAR_CODE('name'),
		kDNewtonInfo				= FOUR_CHAR_CODE('ninf'),

		// Desktop -> Newton Commands

		/// Ask Newton to start docking process
		kDInitiateDocking			= FOUR_CHAR_CODE('dock'),
		/// Info from the desktop (application & so on)
		kDDesktopInfo				= FOUR_CHAR_CODE('dinf'),
		/// Optional to define which icons are shown
		kDWhichIcons				= FOUR_CHAR_CODE('wicn'),

	// Sync and Selective Sync

		// Newton -> Desktop Commands

		kDRequestToSync				= FOUR_CHAR_CODE('ssyn'),
		kDSyncOptions				= FOUR_CHAR_CODE('sopt'),

		// Desktop -> Newton Commands

		kDGetSyncOptions			= FOUR_CHAR_CODE('gsyn'),
		kDSyncResults				= FOUR_CHAR_CODE('sres'),
		kDSetStoreGetNames			= FOUR_CHAR_CODE('ssgn'),
		kDSetSoupGetInfo			= FOUR_CHAR_CODE('ssgi'),
		kDGetChangedIndex			= FOUR_CHAR_CODE('cidx'),
		kDGetChangedInfo			= FOUR_CHAR_CODE('cinf'),

	// File browsing
	//	File browsing will use the same protocol described above with the
	//  following additions. For synchronize, the process is completely driven
	//  from the desktop side. For file browsing/importing, however, the
	//  process is driven from the Newton.

		// Newton -> Desktop Commands

		kDRequestToBrowse			= FOUR_CHAR_CODE('rtbr'),
		/// Windows only
		kDGetDevices				= FOUR_CHAR_CODE('gdev'),
		/// Get the starting path
		kDGetDefaultPath			= FOUR_CHAR_CODE('dpth'),
		/// Ask the desktop for files and folders
		kDGetFilesAndFolders		= FOUR_CHAR_CODE('gfil'),
		kDSetPath					= FOUR_CHAR_CODE('spth'),
		kDGetFileInfo				= FOUR_CHAR_CODE('gfin'),
		kDInternalStore				= FOUR_CHAR_CODE('isto'),
		kDResolveAlias				= FOUR_CHAR_CODE('rali'),
		/// Windows only
		kDGetFilters				= FOUR_CHAR_CODE('gflt'),
		/// Windows only
		kDSetFilter					= FOUR_CHAR_CODE('sflt'),
		/// Windows only
		kDSetDrive					= FOUR_CHAR_CODE('sdrv'),

		// Desktop -> Newton

		/// Windows only
		kDDevices					= FOUR_CHAR_CODE('devs'),
		/// Windows only
		kDFilters					= FOUR_CHAR_CODE('filt'),
		kDPath						= FOUR_CHAR_CODE('path'),
		/// Frame of info about files and folders
		kDFilesAndFolders			= FOUR_CHAR_CODE('file'),
		kDFileInfo					= FOUR_CHAR_CODE('finf'),
		kDGetInternalStore	 		= FOUR_CHAR_CODE('gist'),
		kDAliasResolved				= FOUR_CHAR_CODE('alir'),

		/// No data (?) undocumented.
		/// Newton -> Dock
		/// Réponse: ocaa
		kDOperationCanceled2		= FOUR_CHAR_CODE('opca'),
	// File importing
	//	File importing uses the file browsing interface described above. After
	//  the user taps the import button, the following commands are used.

		// Newton -> Desktop Commands

		kDImportFile				= FOUR_CHAR_CODE('impt'),
		kDSetTranslator				= FOUR_CHAR_CODE('tran'),

		// Desktop -> Newton

		kDTranslatorList			= FOUR_CHAR_CODE('trnl'),
		kDImporting					= FOUR_CHAR_CODE('dimp'),
		kDSoupsChanged				= FOUR_CHAR_CODE('schg'),
		kDSetStoreToDefault			= FOUR_CHAR_CODE('sdef'),

	// Package loading
	//	Package loading uses the file browsing interface described above. After
	//  the user taps the load package button, the following commands are used.

		// Newton -> Desktop Commands

		kDLoadPackageFile			= FOUR_CHAR_CODE('lpfl'),

	// Restore originated on Newton
	//	Restore uses the file browsing interface described above. After the
	//  user taps the restore button, the following commands are used.

		// Newton -> Desktop Commands

		kDRestoreFile				= FOUR_CHAR_CODE('rsfl'),
		kDGetRestoreOptions			= FOUR_CHAR_CODE('grop'),
		kDRestoreAll				= FOUR_CHAR_CODE('rall'),

		// Desktop -> Newton or Newton -> Desktop

		kDRestoreOptions			= FOUR_CHAR_CODE('ropt'),
		kDRestorePackage			= FOUR_CHAR_CODE('rpkg'),

	// Desktop Initiated Functions while connected
	//	With the advent of the new protocol, the Newton and the desktop can be
	//  connected, but with no command specified. A command can be requested by
	//  the user on either the Newton or the Desktop. Commands requested by the
	//  newton user are discussed above. This section describes the commands
	//  sent from the Desktop to the Newton in response to a user request on
	//  the desktop.

		// Desktop  -> Newton Commands

//		kDRequestToSync				= FOUR_CHAR_CODE('ssyn'),
		kDRequestToRestore			= FOUR_CHAR_CODE('rrst'),
		kDRequestToInstall			= FOUR_CHAR_CODE('rins'),

	// 1.0 Newton ROM Support

		// Newton -> Desktop Commands

		/// Ask PC to start docking process
		kDRequestToDock				= FOUR_CHAR_CODE('rtdk'),
		// The name of the newton
//		kDNewtonName				= FOUR_CHAR_CODE('name'),
		/// The current time on the Newton
		kDCurrentTime				= FOUR_CHAR_CODE('time'),
		/// data = array of store names & signatures
		kDStoreNames				= FOUR_CHAR_CODE('stor'),
		/// data =  array of soup names & signatures
		kDSoupNames					= FOUR_CHAR_CODE('soup'),
		/// data = array of ids for the soup
		kDSoupIDs					= FOUR_CHAR_CODE('sids'),
		/// data = array of ids
		kDChangedIDs				= FOUR_CHAR_CODE('cids'),
		/// data = command & result (error)
		kDResult					= FOUR_CHAR_CODE('dres'),
		/// data = the id of the added entry
		kDAddedID					= FOUR_CHAR_CODE('adid'),
		/// data = entry being returned
		kDEntry						= FOUR_CHAR_CODE('entr'),
		/// data = list of package ids
		kDPackageIDList				= FOUR_CHAR_CODE('pids'),
		/// data = package
		kDPackage					= FOUR_CHAR_CODE('apkg'),
		/// data = index description array
		kDIndexDescription			= FOUR_CHAR_CODE('indx'),
		/// data = array of class, supperclass pairs
		kDInheritance				= FOUR_CHAR_CODE('dinh'),
		/// no data
		kDPatches					= FOUR_CHAR_CODE('patc'),

		// Desktop -> Newton

// data = session type
//		kDInitiateDocking			= FOUR_CHAR_CODE('dock'),
		/// The time of the last sync
		kDLastSyncTime				= FOUR_CHAR_CODE('stme'),
		/// no data
		kDGetStoreNames				= FOUR_CHAR_CODE('gsto'),
		/// no data
		kDGetSoupNames				= FOUR_CHAR_CODE('gets'),
		/// data = store frane
		kDSetCurrentStore			= FOUR_CHAR_CODE('ssto'),
		/// data = soup name
		kDSetCurrentSoup			= FOUR_CHAR_CODE('ssou'),
		/// no data
		kDGetSoupIDs				= FOUR_CHAR_CODE('gids'),
		/// no date
		kDGetChangedIDs				= FOUR_CHAR_CODE('gcid'),
		/// data = list of IDs
		kDDeleteEntries				= FOUR_CHAR_CODE('dele'),
		/// data = flattened entry
		kDAddEntry					= FOUR_CHAR_CODE('adde'),
		/// data = ID to return
		kDReturnEntry				= FOUR_CHAR_CODE('rete'),
		/// data = ID to return
		kDReturnChangedEntry		= FOUR_CHAR_CODE('rcen'),
		/// no data
		kDEmptySoup					= FOUR_CHAR_CODE('esou'),
		/// no data
		kDDeleteSoup				= FOUR_CHAR_CODE('dsou'),
		/// data = package
		kDLoadPackage				= FOUR_CHAR_CODE('lpkg'),
		/// no data
		kDGetPackageIDs				= FOUR_CHAR_CODE('gpid'),
		/// no data
		kDBackupPackages			= FOUR_CHAR_CODE('bpkg'),
		/// no data
		kDDisconnect				= FOUR_CHAR_CODE('disc'),
		/// no data
		kDDeleteAllPackages			= FOUR_CHAR_CODE('dpkg'),
		/// no data
		kDGetIndexDescription		= FOUR_CHAR_CODE('gind'),
		/// data = name + index description
		kDCreateSoup				= FOUR_CHAR_CODE('csop'),
		/// no data
		kDGetInheritance			= FOUR_CHAR_CODE('ginh'),
		/// data = # of seconds
		kDSetTimeout				= FOUR_CHAR_CODE('stim'),
		/// no data
		kDGetPatches				= FOUR_CHAR_CODE('gpat'),
		/// no data
		kDDeletePkgDir				= FOUR_CHAR_CODE('dpkd'),
		/// no data
		kDGetSoupInfo				= FOUR_CHAR_CODE('gsin'),

		// Desktop -> Newton or Newton -> Desktop

		/// data = entry being returned
		kDChangedEntry				= FOUR_CHAR_CODE('cent'),
		/// variable length data
		kDTest						= FOUR_CHAR_CODE('test'),
		/// no data
		kDHello						= FOUR_CHAR_CODE('helo'),
		/// data = soup info frame
		kDSoupInfo					= FOUR_CHAR_CODE('sinf'),

		// Unknown commands.
		// Desktop -> Newton
		/// Ouvre la fenêtre de progression.
		/// Pas de données.
		kDUnknown_dsnc				= FOUR_CHAR_CODE('dsnc'),

	// Session types
		kNone_SType					= 0,
		kSettingUp_SType			= 1,
		kSynchronize_SType			= 2,
		kRestore_SType				= 3,
		kLoadPackage_SType			= 4,
		kTestComm_SType				= 5,
		kLoadPatch_SType			= 6,
		kUpdatingStores_SType		= 7,

	// GetAppNames options

		kNamesAndSoupsAllStores		= 0,
		kNamesAndSoupsCurrentStore	= 1,
		kJustNamesAllStores			= 2,
		kJustNamesCurrentStore		= 3,

	// Protocol version: for Dante Dock application
		kProtocolVersion			= 10,
	// Note: there is a bug in the Dante documentation. If you say 9, the
	// Newton will answer -28011!

	// Desktop Types
		kMac_DType					= 0,
		kWin_DType					= 1,

	// This is used for file browsing, so you'd better say Win on non MacOS
	// machines.

#if TARGET_OS_MAC
		kThis_DType					= kMac_DType,
#else
		kThis_DType					= kWin_DType,
#endif

	// FS Item types
		kDesktop_FSType				= 0,	// MacOS only
		kFile_FSType				= 1,	// Mac & Windoze
		kFolder_FSType				= 2,	// Mac & Windoze
		kDisk_FSType				= 3,	// MacOS only

	// Dock Protocol Errors
		kBadPassword_Err			= -28022,
		kRetryPassword_Err			= -28023

	};

	///
	/// Récupère le code de cette commande.
	///
	/// \return l'identifiant de cette commande.
	///
	inline	KUInt32			GetCommand( void ) const
		{
			return mCommand;
		};

protected:
	///
	/// Constructeur par défaut.
	/// Ne règle pas l'identifiant de la commande.
	///
	TDCLDockCommand( void );

	///
	/// Constructeur à partir de l'identifiant de la commande.
	///
	/// \param inCommand identifiant de la commande
	///
	TDCLDockCommand( KUInt32 inCommand );

	///
	/// Change l'identifiant de la commande.
	///
	/// \param inCommand 	nouvel identifiant de la commande.
	///
	inline	void			SetCommand( KUInt32 inCommand )
		{
			mCommand = inCommand;
		};

	///
	/// Méthode appelée pour récupèrer la taille de la commande avant de
	/// l'envoyer.
	///
	/// \return la taille de la commande
	///
	virtual	KUInt32			GetLength( void ) const;

	///
	/// Méthode appelée pour récupèrer les données à envoyer.
	/// Cette méthode n'est pas const parce que la sous-classe pour changer
	/// l'objet pour préparer les données à envoyer.
	/// Le pointeur appartient à la sous-classe qui doit le libérer s'il a été
	/// alloué.
	///
	/// \return un pointeur sur les données à envoyer.
	///
	virtual const void*		GetData( void );

	///
	/// Envoie l'entête de la commande,
	/// i.e. 'newt', 'dock', identifiant et longueur.
	/// Par défaut, récupère la taille avec \c GetLength et envoie les 4 longs.
	///
	/// \param inPipe				connexion pour écrire la commande.
	/// \return la taille de la commande.
	///
	virtual	KUInt32			SendHeader( TDCLStream* inPipe );

	///
	/// Envoie le corps de la commande.
	/// Par défaut, envoie les données obtenues par GetData.
	///
	/// \param inPipe				connexion pour écrire la commande.
	/// \param inLength				taille des données à envoyer
	/// \param inProgressFuncPtr	fonction appelée pour la progression
	///								(\c nil signifie qu'on ne prévient pas)
	///								Cette méthode doit tenir compte du fait
	///								que 16 octets ont déjà été envoyés.
	/// \param inRefCon				biscuit pour inProgressFuncPtr
	///
	virtual	void			SendBody(
									TDCLStream* inPipe,
									KUInt32 inLength,
									ProgressFuncPtr inProgressFuncPtr,
									void* inRefCon );


private:
	KUInt32					mCommand;	///< Identifiant de la commande.
};

#endif
		// __TDCLDOCKCOMMAND__

// ============================================================== //
// As of next Thursday, UNIX will be flushed in favor of TOPS-10. //
// Please update your programs.                                   //
// ============================================================== //
