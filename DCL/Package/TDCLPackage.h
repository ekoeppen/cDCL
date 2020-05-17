// ==============================
// Fichier:			TDCLPackage.h
// Projet:			Desktop Connection Library
// 
// Créé le:			29/10/2003
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
// The Original Code is TDCLPackage.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík
// and Nicolas Zinovieff. Portions created by the Initial Developers are
// Copyright (C) 1999-2004 the Initial Developers. All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========
// $Id: TDCLPackage.h,v 1.16 2004/11/24 14:09:08 paul Exp $
// ===========

#ifndef _TDCLPACKAGE_H
#define _TDCLPACKAGE_H

#include <DCL/Headers/DCLDefinitions.h>

class TDCLFile;
class TDCLStream;
class TDCLPkgPart;

///
/// Classe pour un paquet Newton.
/// Dérivé de TNewtonPackage (interne).
///
/// Cette classe gère essentiellement le catalogue des paquets. Lorsqu'un objet
/// de cette classe est créé à partir d'une mémoire tampon ou d'un fichier, les
/// données des parties sont copiées. Elles ne sont analysées que la première
/// fois qu'elles sont accédées.
///
/// \author Paul Guyot <pguyot@kallisys.net>
/// \version $Revision: 1.16 $
///
/// \test	aucun test défini.
///
class TDCLPackage
{
public:
	///
	/// Drapeaux pour SPackageDirectory
	/// Les autres bits sont réservés et doivent être à zéro.
	///
	enum {
		/// Les parties du paquet doivent être supprimées immédiatement
		/// après l'installation. Lorsqu'un paquet avec ce drapeau à un est
		/// activé, le système active ses parties puis ensuite les désactive
		/// sans procéder à la désactivation normale des parties (comme
		/// appeler removeScript). Ceci n'est recommandé que pour les paquets
		/// en une seule partie de type 'auto'.
		///
		/// Néanmoins, les mise à jour système ont ce bit à un, même si elles
		/// sont composées de plusieurs parties.
		kDirAutoRemoveFlag				= 0x80000000,
		
		/// Marque le paquet comme protégé pour la copie. Ce drapeau ne sert
		/// que parce que les logiciels qui copient les paquets le respectent.
		/// Ce n'est pas un verrou absolu contre la copie.
		/// Le système refuse d'envoyer par infra-rouge ou par courrier
		/// électronique les paquets avec ce bit à un. Un tel paquet peut
		/// néanmoins être sauvegardé sur une carte ou synchronisé avec un
		/// ordinateur de bureau, donc les utilisateurs peuvent le copier. Ils
		/// peuvent aussi utiliser EEUnprotect.
		kDirCopyProtectFlag				= 0x40000000,
		
		/// Indique au système qu'il ne doit pas compresser le paquet (par
		/// défaut les paquets sont compressés).
		kDirNoCompressionFlag			= 0x10000000,

		/// Indique que le paquet contient des informations de relogement.
		/// Ce drapeau n'est valide que pour les paquets de type "package1".	
		kDirRelocationFlag				= 0x04000000,

		/// Spécifie que le paquet doit être compressé avec une méthode plus
		/// rapide mais moins efficace. Ce drapeau n'est valide que pour les
		/// paquets de type "package1" et n'est pris en compte que si le
		/// drapeau kDirNoCompressionFlag n'est pas à un.
		kDirUseFasterCompressionFlag	= 0x02000000,
		
		/// Indique que le paquet a une signature numérique qui certifie
		/// qu'il fonctionne sur Watson.
		/// La signature est dans une partie de type raw et fait 32 bits.
		/// C'est tout ce que je sais pour le moment.
		kWatsonSignaturePresentFlag		= 0x01000000
	};
	
	/// Signature pour un paquet sans information de relogement.
	static const KUInt8	kNOS1CompatibleSignature[8];
	
	/// Signature pour un paquet qui ne fonctionne que sur NewtonOS 2.x.
	static const KUInt8 kNOS1IncompatibleSignature[8];

	///
	/// Drapeaux pour SPartEntry.
	/// Les autres bits sont réservés et doivent être à zéro.
	///
	enum {
		/// La partie est un protocole.
		kPartProtocolPart	= 0x00000000,
		
		/// La partie est composée d'objets NewtonScript (au format Pkg)
		kPartNOSPart		= 0x00000001,
		
		/// La partie est composée de données brutes.
		kPartRawPart		= 0x00000002,
		
		/// Masque pour le type de partie.
		kPartTypeMask		= 0x00000003,
		
		/// Uniquement pour les protocoles: précise que le protocole doit être
		/// enregistré lorsque le paquet est activé.
		kPartAutoLoadFlag	= 0x00000010,
		
		/// Uniquement pour les protocoles: précise que le protocole doit être
		/// désenregistré lorsque le paquet est désactivé. Normalement à un
		/// lorsque kPartAutoLoadFlag est à un.
		kPartAutoRemoveFlag	= 0x00000020,
		
		/// Spécifie que le gestionnaire dans le système correspondant au type
		/// de la partie doit être appelé. Normalement à un sauf si les
		/// spécifications pour le type de partie indiquent le contraire.
		kPartNotifyFlag		= 0x00000080,
		
		/// Spécifie que la partie doit être copiée en mémoire avant d'être
		/// activée. Ne doit être utilisé que pour les parties (normalement des
		/// protocoles) qui ne tolèrent pas le mécanisme de gestion de mémoire
		/// virtuelle.
		kPartAutoCopyFlag	= 0x00000100
	};
	
	///
	/// Types des parties.
	///
	enum {
		kFormPart	= FOUR_CHAR_CODE('form'),	///< Application
		kBookPart	= FOUR_CHAR_CODE('book'),	///< Livre
		kAutoPart	= FOUR_CHAR_CODE('auto'),	///< Applications invisibles
		kFontPart	= FOUR_CHAR_CODE('font'),	///< Polices supplémentaires
		kDictPart	= FOUR_CHAR_CODE('dict'),	///< Dictionnaires
												///< supplémentaires
		kStorePart	= FOUR_CHAR_CODE('soup')	///< Magasin avec des soupes en
												///< lecture seule.
	};
	
	///
	/// Identifiants des paquets (non utilisé par NTK, probablement non
	/// utilisés par le système).
	///
	enum {
		kDefaultID		= FOUR_CHAR_CODE('xxxx'),	///< Identifiant par défaut.
		kSystemUpdateID	= FOUR_CHAR_CODE('ptch'),	///< Identifiant utilisé par
													///< les rustines.
		kSoundCodecID	= FOUR_CHAR_CODE('codt')	///< Identifiant utilisé par
													///< les Codec.
	};
	
	enum {
		// La taille maximale autorisée par NTK est 28 caractères.
		// (apparemment, NewtonOS en autorise 31).
		kPackageNameMaxLength	= 28,	///< Taille maximale, en caractères,
										///< du nom.
		kCopyrightMaxLength		= 63	///< Taille maximale, en caractères,
										///< de la chaîne avec le copyright.
	};
	
	///
	/// Les structures 'InfoRef' permettent de faire référence à des données de
	/// taille variable dans la partie données du catalogue.
	///
	struct SInfoRef {
		KUInt16		fOffset;	///< Décalage depuis le début de la zone des
								///< données.
		KUInt16		fLength;	///< Taille des données (en octets).
	};
	
	///
	/// Catalogue des paquets.
	///
	struct SPackageDirectory {
		KUInt8		fSignature[8];	///< Signature (format du paquet)
		KUInt32		fPackageID;		///< Type (e.g. kDefaultID).
		KUInt32		fFlags;			///< Drapeaux (e.g. kDirAutoRemoveFlag).
		KUInt32		fVersion;		///< Nombre arbitraire pour la version.
									///< Plus grand signifie plus récent
									///< pour le système.
		SInfoRef	fCopyright;		///< Chaîne UCS-2 avec la notice copyright
									///< (éventuellement vide)
		SInfoRef	fName;			///< Chaîne UCS-2 avec le nom du paquet.
									///< C'est normalement un identifiant
									///< unique avec en préfixe, la signature
									///< du développeur.
		KUInt32		fSize;			///< Taille du paquet en octets, y compris
									///< le catalogue.
		KUInt32		fCreationDate;	///< Date de création, en secondes depuis
									///< le 1/01/1904
		KUInt32		fReserved1;		///< Réservé (doit être 0).
		KUInt32		fReserved2;		///< Réservé (doit être 0).
		KUInt32		fDirectorySize;	///< Taille du catalogue avec les entrées
									///< et les données.
		KUInt32		fNumParts;		///< Nombre de parties dans le paquet.
		// SPartEntry fParts[fNumParts]
		// KUInt8 fVariableLengthData[];
	};
	
	///
	/// Après SPackageDirectory se trouve, pour chaque partie, une structure de
	/// ce type. On fait parfois référence aux parties par leur numéro,
	/// celui-ci est l'indice dans le catalogue commençant par 0.
	///
	struct SPartEntry {
		KUInt32		fOffset;		///< Décalage, par rapport à la partie
									///< données (i.e. après
									///< le catalogue) des données de cette
									///< partie. Multiple
									///< de 4, généralement 0 pour la première
									///< partie.
		KUInt32		fSize;			///< Taille en octets.
		KUInt32		fSize2;			///< Encore la taille, en octets (copie de
									///< fSize).
		KUInt32		fType;			///< Type de la partie.
		KUInt32		fReserved1;		///< Réservé (doit être 0).
		KUInt32		fFlags;			///< Drapeaux (e.g. kPartProtocolPart).
		SInfoRef	fInfo;			///< Bloc de données passé au gestionnaire
									///< de parties lorsque la partie est
									///< activée. Le contenu est spécifié par
									///< le type de partie.
		KUInt32		fReserved2;		///< Réservé (doit être 0).
	};

	///
	/// Constructeur par défaut.
	/// Crée un paquet vide.
	///
	TDCLPackage( void );

	///
	/// Constructeur à partir d'un fichier.
	/// Ouvre le fichier, le lit entièrement, le referme et
	/// analyse le paquet.
	/// Si le fichier était ouvert, il est restitué dans l'état dans lequel
	/// il était.
	///
	/// \param inFile	fichier considéré.
	/// \throws DCLException si le fichier n'est pas un paquet.
	///
	TDCLPackage( TDCLFile* inFile );

	///
	/// Constructeur à partir d'un flux. Lit le paquet entièrement.
	///
	/// \param inStream	flux considéré.
	/// \throws DCLException si le fichier n'est pas un paquet.
	///
	TDCLPackage( TDCLStream* inStream );

	///
	/// Constructeur à partir d'une mémoire tampon.
	/// La mémoire tampon est copiée.
	/// Analyse le catalogue et copie les données des parties.
	///
	/// \param inBuffer	mémoire tampon
	/// \param inSize	taille de la mémoire tampon.
	/// \throws DCLException si la mémoire tampon n'est pas un paquet.
	///
	TDCLPackage( const KUInt8* inBuffer, KUInt32 inSize );

	///
	/// Destructeur.
	/// Supprime toutes les données qui traînent.
	///
	~TDCLPackage( void );

	///
	/// Rapide test pour savoir si le fichier considéré est un
	/// paquet ou non.
	/// Ouvre le fichier si il était fermé, lit 8 octets puis le referme.
	/// Si le fichier était ouvert, il est restitué dans l'état dans lequel
	/// il était.
	///
	/// \param inFile	fichier à considérer.
	/// \return \c true si le fichier semble être un paquet, \c false sinon.
	///
	static Boolean IsPackage( TDCLFile* inFile );

	///
	/// Compile et écrit le paquet sur un flux.
	///
	/// \param inOutStream	flux de sortie.
	///
	void	WriteToStream( TDCLStream* inOutStream ) const;
	
	///
	/// Accesseur sur la compatibilité du paquet.
	///
	/// \return \c true si le paquet est compatible avec NOS 1.x.
	///
	inline Boolean		IsNOS1Compatible( void ) const
		{
			return mNOS1Compatible;
		}

	///
	/// Sélecteur sur la version du paquet.
	///
	/// \param inNOS1Compatible		si le paquet est compatible NOS 1.x
	///
	inline void			SetNOS1Compatible( Boolean inNOS1Compatible )
		{
			mNOS1Compatible = inNOS1Compatible;
		}

	///
	/// Accesseur sur l'identifiant du paquet.
	///
	/// \return l'identifiant du paquet.
	///
	inline KUInt32		GetPackageID( void ) const
		{
			return mPackageID;
		}

	///
	/// Sélecteur sur l'identifiant du paquet.
	///
	/// \param inID					identifiant du paquet.
	///
	inline void			SetPackageID( KUInt32 inID )
		{
			mPackageID = inID;
		}

	///
	/// Accesseur sur les drapeaux du paquet.
	///
	/// \return les drapeaux du paquet.
	///
	inline KUInt32		GetPackageFlags( void ) const
		{
			return mPackageFlags;
		}

	///
	/// Sélecteur sur les drapeaux du paquet.
	///
	/// \param inFlags				drapeaux du paquet.
	///
	inline void			SetPackageFlags( KUInt32 inFlags )
		{
			mPackageFlags = inFlags;
		}

	///
	/// Accesseur sur le drapeau kDirAutoRemoveFlag
	///
	/// \return \c true si le drapeau kDirAutoRemoveFlag est à un.
	///
	inline Boolean		GetAutoRemove( void ) const
		{
			return ((GetPackageFlags() & kDirAutoRemoveFlag) != 0);
		}

	///
	/// Sélecteur sur le drapeau kDirAutoRemoveFlag
	///
	/// \param inAutoRemove		nouvelle valeur du drapeau
	///
	inline void			SetAutoRemove( Boolean inAutoRemove )
		{
			SetPackageFlags(
					inAutoRemove ?
						GetPackageFlags() | kDirAutoRemoveFlag
						: GetPackageFlags() &~ kDirAutoRemoveFlag );
		}

	///
	/// Accesseur sur le drapeau kDirCopyProtectFlag
	///
	/// \return \c true si le drapeau kDirCopyProtectFlag est à un.
	///
	inline Boolean		GetCopyProtect( void ) const
		{
			return ((GetPackageFlags() & kDirCopyProtectFlag) != 0);
		}

	///
	/// Sélecteur sur le drapeau kDirCopyProtectFlag
	///
	/// \param inCopyProtect	nouvelle valeur du drapeau
	///
	inline void			SetCopyProtect( Boolean inCopyProtect )
		{
			SetPackageFlags(
					inCopyProtect ?
						GetPackageFlags() | kDirCopyProtectFlag
						: GetPackageFlags() &~ kDirCopyProtectFlag );
		}

	///
	/// Accesseur sur le drapeau kDirNoCompressionFlag
	///
	/// \return \c true si le drapeau kDirNoCompressionFlag est à un.
	///
	inline Boolean		GetNoCompression( void ) const
		{
			return ((GetPackageFlags() & kDirNoCompressionFlag) != 0);
		}

	///
	/// Sélecteur sur le drapeau kDirNoCompressionFlag.
	/// Si mis à un, change kDirUseFasterCompressionFlag à zéro.
	///
	/// \param inNoCompression	nouvelle valeur du drapeau
	///
	inline void			SetNoCompression( Boolean inNoCompression )
		{
			SetPackageFlags(
					inNoCompression ?
						(GetPackageFlags() &~ kDirUseFasterCompressionFlag)
							| kDirNoCompressionFlag
						: GetPackageFlags() &~ kDirNoCompressionFlag );
		}

	///
	/// Accesseur sur le drapeau kDirUseFasterCompressionFlag
	///
	/// \return \c true si le drapeau kDirUseFasterCompressionFlag est à un.
	///
	inline Boolean		GetUseFasterCompression( void ) const
		{
			return ((GetPackageFlags() & kDirUseFasterCompressionFlag) != 0);
		}

	///
	/// Sélecteur sur le drapeau kDirUseFasterCompressionFlag.
	/// Si celui-ci est mis à \c true, le paquet n'est plus compatible NOS 1.x.
	///
	/// \param inUseFasterCompression	nouvelle valeur du drapeau
	///
	inline void			SetUseFasterCompression(
											Boolean inUseFasterCompression )
		{
			if (inUseFasterCompression)
			{
				mNOS1Compatible = false;
				SetPackageFlags(
						GetPackageFlags() | kDirUseFasterCompressionFlag );
			} else {
				SetPackageFlags(
						GetPackageFlags() &~ kDirUseFasterCompressionFlag );
			}
		}

	///
	/// Accesseur sur la version du paquet.
	///
	/// \return la version du paquet.
	///
	inline KUInt32		GetPackageVersion( void ) const
		{
			return mVersion;
		}

	///
	/// Sélecteur sur la version du paquet.
	///
	/// \param inVersion			version du paquet.
	///
	inline void			SetPackageVersion( KUInt32 inVersion )
		{
			mVersion = inVersion;
		}

	///
	/// Accesseur sur la date de création du paquet.
	///
	/// \return la date de création du paquet (secondes depuis le 1/01/1904).
	///
	inline KUInt32		GetCreationDate( void ) const
		{
			return mCreationDate;
		}

	///
	/// Sélecteur sur la date de création du paquet.
	///
	/// \param inCreationDate		date de création
	///								(secondes depuis le 1/01/1904)
	///
	inline void			SetCreationDate( KUInt32 inCreationDate )
		{
			mCreationDate = inCreationDate;
		}

	///
	/// Accesseur sur la chaîne de copyright.
	///
	/// \return la chaîne de copyright (UCS-2, grand boutien).
	///
	inline const KUInt16*	GetCopyrightString( void ) const
		{
			return mCopyrightStr;
		}

	///
	/// Sélecteur sur la chaîne de copyright (UCS-2, grand boutien).
	///
	/// \param inCopyrightStr	nouvelle chaîne de Copyright (copiée).
	///
	void			SetCopyrightString( const KUInt16* inCopyrightStr );

	///
	/// Sélecteur sur la chaîne de copyright (Latin-1).
	///
	/// \param inCopyrightStr	nouvelle chaîne de Copyright (copiée).
	///
	void			SetCopyrightString( const char* inCopyrightStr );

	///
	/// Accesseur sur le nom du paquet.
	///
	/// \return le nom (UCS-2, grand boutien)
	///
	inline const KUInt16*	GetPackageName( void ) const
		{
			return mNameStr;
		}

	///
	/// Sélecteur sur le nom du paquet (UCS-2, grand boutien).
	///
	/// \param inNameStr	nouveau nom (copié).
	///
	void			SetPackageName( const KUInt16* inNameStr );

	///
	/// Sélecteur sur le nom du paquet (Latin-1)
	///
	/// \param inNameStr	nouveau nom (copié).
	///
	void			SetPackageName( const char* inNameStr );

	///
	/// Accesseur sur le nombre de parties dans le paquet.
	///
	/// \return le nombre de parties
	///
	inline KUInt32		GetNumParts( void ) const
		{
			return mNumParts;
		}
	
	///
	/// Accesseur sur le type de la n-ième partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	///
	inline KUInt32		GetPartType( KUInt32 inPartIndex ) const
		{
			KASSERT( inPartIndex < mNumParts );
			return mParts[inPartIndex].fType;
		}

	///
	/// Sélecteur sur le type de la n-ième partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	/// \param inPartType	nouveau type de la partie.
	///
	inline void			SetPartType( KUInt32 inPartIndex, KUInt32 inPartType )
		{
			KASSERT( inPartIndex < mNumParts );
			mParts[inPartIndex].fType = inPartType;
		}
	
	///
	/// Accesseur sur les drapeaux de la n-ième partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	///
	inline KUInt32		GetPartFlags( KUInt32 inPartIndex ) const
		{
			KASSERT( inPartIndex < mNumParts );
			return mParts[inPartIndex].fFlags;
		}

	///
	/// Sélecteur sur les drapeaux de la n-ième partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	/// \param inPartFlags	nouveaux drapeaux de la partie.
	///
	inline void			SetPartFlags(
							KUInt32 inPartIndex,
							KUInt32 inPartFlags )
		{
			KASSERT( inPartIndex < mNumParts );
			mParts[inPartIndex].fFlags = inPartFlags;
		}

	///
	/// Accesseur sur les informations pour le gestionnaire de la n-ième
	/// partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	/// \param outSize		la taille des informations.
	/// \return les informations pour le gestionnaire.
	///
	inline const KUInt8*	GetPartHandlerInfo(
									KUInt32 inPartIndex,
									KUInt32* outSize ) const
		{
			KASSERT( inPartIndex < mNumParts );
			// Ici, les données doivent être en mémoire (elles ne sont sur
			// le disque que pendant l'analyse du paquet)
			KASSERT( mParts[inPartIndex].fInfoData != nil );
			*outSize = mParts[inPartIndex].fInfoSize;
			return mParts[inPartIndex].fInfoData;
		}

	///
	/// Accesseur sur la n-ième partie. [const]
	///
	/// \param inPartIndex	indice de la partie considérée.
	/// \return la n-ième partie.
	///
	inline const TDCLPkgPart*	GetPart( KUInt32 inPartIndex ) const
		{
			KASSERT( inPartIndex < mNumParts );
			// Ici, les données doivent être en mémoire (elles ne sont sur
			// le disque que pendant l'analyse du paquet)
			KASSERT( mParts[inPartIndex].fPart != nil );
			return mParts[inPartIndex].fPart;
		}

	///
	/// Accesseur sur la n-ième partie.
	///
	/// \param inPartIndex	indice de la partie considérée.
	/// \return la n-ième partie.
	///
	inline TDCLPkgPart*		GetPart( KUInt32 inPartIndex )
		{
			KASSERT( inPartIndex < mNumParts );
			// Ici, les données doivent être en mémoire (elles ne sont sur
			// le disque que pendant l'analyse du paquet)
			KASSERT( mParts[inPartIndex].fPart != nil );
			return mParts[inPartIndex].fPart;
		}

	///
	/// Ajoute une partie à la fin du paquet.
	/// Les données pour le gestionnaire sont déterminées par défaut.
	/// La partie n'est pas dupliquée mais sera supprimée par le destructeur.
	///
	/// \param inType		type de la partie.
	/// \param inFlags		drapeaux pour la partie.
	/// \param inPart		partie (non copiée, sera supprimée par le
	///						destructeur).
	/// \return l'indice de la partie.
	///
	KUInt32 				AddPart(
								KUInt32 inType,
								KUInt32 inFlags,
								TDCLPkgPart* inPart );

	///
	/// Ajoute une partie à la fin du paquet.
	/// La partie n'est pas dupliquée mais sera supprimée par le destructeur.
	///
	/// \param inType		type de la partie.
	/// \param inFlags		drapeaux pour la partie.
	/// \param inInfoSize	taille des informations pour le gestionnaire.
	/// \param inInfoData	données pour le gestionnaire (copiées).
	/// \param inPart		partie (non copiée, sera supprimée par le
	///						destructeur).
	/// \return l'indice de la partie.
	///
	KUInt32 				AddPart(
								KUInt32 inType,
								KUInt32 inFlags,
								KUInt32 inInfoSize,
								const KUInt8* inInfoData,
								TDCLPkgPart* inPart );

	///
	/// Supprime une partie du paquet.
	///
	/// \param inPartIndex	indice de la partie à supprimer. 
	///
	void 				RemovePart( KUInt32 inPartIndex );

private:
	///
	/// Données par défaut pour le gestionnaire (en fait, une chaîne qui
	/// indique la provenance du paquet).
	///
	static const char* const kDefaultHandlerInfoData;

	///
	/// Structure pour les données de localisation des parties dans le paquet.
	///
	struct SPartLocation {
		KUInt32		fInfoOffset;	///< Décalage des données pour le
									///< gestionnaire.
		KUInt32		fDataOffset;	///< Décalage des données de la partie.
		KUInt32		fDataSize;		///< Taille des données de la partie.
	};

	///
	/// Structure pour les données conservées sur les parties.
	///
	struct SPartData {
		KUInt32			fType;		///< Type de la partie.
		KUInt32			fFlags;		///< Drapeaux de la partie.
		KUInt32			fInfoSize;	///< Taille des données pour le
									///< gestionnaire.
		KUInt8*			fInfoData;	///< Données pour le gestionnaire.
		TDCLPkgPart*	fPart;		///< Données de la partie.
	};
		
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPackage( const TDCLPackage& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLPackage& operator = ( const TDCLPackage& inCopy );
	
	///
	/// Lecture du paquet à partir d'un flux.
	/// Lance une exception en cas de problème.
	///
	/// \param inStream		flux à lire.
	///
	void			ReadPackage( TDCLStream* inStream );

	/// \name Variables
	Boolean			mNOS1Compatible;///< Si le paquet est compatible 1.x.
	KUInt32			mPackageID;		///< ID (e.g. kDefaultID).
	KUInt32			mPackageFlags;	///< Drapeaux (e.g. kDirAutoRemoveFlag).
	KUInt32			mVersion;		///< Nombre arbitraire pour la version.
									///< Plus grand signifie plus récent pour
									///< le système.
	KUInt32			mCreationDate;	///< Date de création, en secondes depuis
									///< le 1/01/1904
	KUInt16*		mCopyrightStr;	///< Chaîne pour le copyright
	KUInt16*		mNameStr;		///< Chaîne pour le nom.
	KUInt32			mNumParts;		///< Nombre de parties dans le paquet.
	SPartData*		mParts;			///< Parties.
};

#endif
		// _TDCLPACKAGE_H

// ===================================================================== //
// When someone says "I want a programming language in which I need only //
// say what I wish done," give him a lollipop.                           //
// ===================================================================== //
