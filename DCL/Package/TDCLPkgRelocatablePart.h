// ==============================
// Fichier:         TDCLPkgNOSPart.h
// Projet:          Desktop Connection Library
//
// Créé le:         19/5/2020
// Tabulation:      4 espaces
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
// The Original Code is TDCLPkgNOSPart.cpp.
//
// The Initial Developer of the Original Code is Paul Guyot. Portions created
// by the Initial Developers are Copyright (C) 2020 the Initial Developers.
// All Rights Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//
// ***** END LICENSE BLOCK *****
// ===========

#ifndef _TDCLPKGRELOCATABLEPART_H
#define _TDCLPKGRELOCATABLEPART_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPkgPart.h>

///
/// Class for a relocatable package part, adjusting the binary from a given
/// base address.
///
class TDCLPkgRelocatablePart
    :
        public TDCLPkgPart
{
public:
    ///
    /// Constructeur par défaut.
    /// L'ensemble des données est vide.
    ///
    TDCLPkgRelocatablePart( void );

    ///
    /// Constructeur à partir de données.
    /// Les données sont copiées.
    ///
    /// \param inOffset décalage dans le paquet.
    /// \param inData   données (copiées)
    /// \param inSize   taille des données.
    ///
    TDCLPkgRelocatablePart( KUInt32 inOffset, const void* inData, KUInt32 inSize );

    ///
    /// Définit les relocations. Ne modifie pas l'adresse de base qui est
    /// \c inOffset fourni au constructeur.
    /// Les relocations sont copiées.
    ///
    /// \param inCount          nombre de relocations
    /// \param inRelocations    relocations par rapport à l'origine.
    ///
    void SetRelocations( KUInt32 inCount, const KUInt32* inRelocations )
        {
            SetRelocations( inCount, inRelocations, mBaseAddress );
        }

    ///
    /// Définit les relocations.
    /// Les relocations sont copiées.
    ///
    /// \param inCount          nombre de relocations
    /// \param inRelocations    relocations par rapport à l'origine.
    /// \param inBaseAddress    adresse de base du début des données de la
    ///                         partie.
    ///
    void SetRelocations( KUInt32 inCount, const KUInt32* inRelocations, KUInt32 inBaseAddress );

    ///
    /// Accesseur sur les relocations.
    ///
    /// \return  le nombre de relocations
    ///
    virtual KUInt32 GetRelocationCount( void ) const override
        {
            return mRelocationCount;
        }

    ///
    /// Accesseur sur les relocations.
    ///
    /// \return  les relocations.
    ///
    virtual const KUInt32* GetRelocations( void ) const override
        {
            return mRelocations;
        }

    ///
    /// Destructeur.
    ///
    virtual ~TDCLPkgRelocatablePart( void );

    ///
    /// Détermine le nombre de pages de relocation.
    /// Chaque page fait 1Ko (0x400). Le nombre final de pages doit être
    /// inférieur ou égal au résultat de cette fonction.
    ///
    /// \return un estimateur du nombre de pages de relocation.
    ///
    KUInt32     GetRelocationPageCountEstimate( void ) const override;

private:
    ///
    /// Les données dépendent de l'offset si on a des relocations.
    ///
    /// \return \c true si \c mRelocationCount > 0.
    ///
    virtual Boolean IsOffsetDependant( void ) const override;

    ///
    /// Relocalise les données par rapport à l'offset dans le paquet.
    /// Utilise la différence inOffset - mBaseAddress.
    ///
    /// \param inOffset             décalage dans le paquet.
    /// \param ioBuffer             pointeur sur les données
    ///                             (alloué avec malloc)
    /// \param ioSize               taille des données (en entrée et en sortie)
    ///
    virtual void Encode(
                    KUInt32 inOffset,
                    void** ioBuffer,
                    KUInt32* ioSize ) const override;

    KUInt32     mRelocationPageCountEstimate;
    KUInt32     mBaseAddress;
    KUInt32     mRelocationCount;
    KUInt32*    mRelocations;
};

#endif
        // _TDCLPKGRELOCATABLEPART_H

// ===================================================================== //
// Warning: Listening to WXRT on April Fools' Day is not recommended for //
// those who are slightly disoriented the first few hours after waking   //
// up.                                                                   //
//      -- Chicago Reader 4/22/83                                        //
// ===================================================================== //
