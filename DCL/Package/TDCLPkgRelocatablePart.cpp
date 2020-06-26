// ==============================
// Fichier:         TDCLPkgRelocatablePart.h
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
// The Original Code is TDCLPkgRelocatablePart.h.
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

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Package/TDCLPkgRelocatablePart.h>
#include <DCL/Exceptions/Errors/TDCLBadParamError.h>

#include <K/Defines/UByteSex.h>

// ANSI C
#include <stdlib.h>
#include <string.h>

#define RELOCATION_PAGE_SIZE 1024

// -------------------------------------------------------------------------- //
//  * TDCLPkgRelocatablePart( void )
// -------------------------------------------------------------------------- //
TDCLPkgRelocatablePart::TDCLPkgRelocatablePart( void )
    :
        TDCLPkgPart(),
        mBaseAddress( 0 ),
        mRelocationCount( 0 ),
        mRelocations( NULL )
{
}

// -------------------------------------------------------------------------- //
//  * TDCLPkgRelocatablePart( KUInt32, const void*, KUInt32 )
// -------------------------------------------------------------------------- //
TDCLPkgRelocatablePart::TDCLPkgRelocatablePart(
            KUInt32 inOffset,
            const void* inData,
            KUInt32 inSize )
    :
        TDCLPkgPart( inOffset, inData, inSize ),
        mBaseAddress( inOffset ),
        mRelocationCount( 0 ),
        mRelocations( NULL )
{
}

// -------------------------------------------------------------------------- //
//  * ~TDCLPkgRelocatablePart( void )
// -------------------------------------------------------------------------- //
TDCLPkgRelocatablePart::~TDCLPkgRelocatablePart( void )
{
    if (mRelocations)
    {
        ::free( mRelocations );
    }
}

// -------------------------------------------------------------------------- //
//  * compare_relocations(const void*, const void*)
// -------------------------------------------------------------------------- //
extern "C" int compare_relocations(const void* a, const void* b)
{
    KUInt32 aValue = *((const KUInt32*) a);
    KUInt32 bValue = *((const KUInt32*) b);
    if (a < b)
    {
        return -1;
    }
    if (a > b)
    {
        return 1;
    }
    return 0;
}

// -------------------------------------------------------------------------- //
//  * SetRelocations(KUInt32, const KUInt32*, KUInt32, KUInt32)
// -------------------------------------------------------------------------- //
void
TDCLPkgRelocatablePart::SetRelocations( KUInt32 inCount, const KUInt32* inRelocations, KUInt32 inBaseAddress, KUInt32 inOffset )
{
    mBaseAddress = inBaseAddress;
    ssize_t size = inCount * sizeof(KUInt32);
    if (mRelocations == NULL)
    {
        mRelocations = (KUInt32*) ::malloc( size );
    } else {
        mRelocations = (KUInt32*) ::realloc( mRelocations, size );
    }
    ::memcpy( mRelocations, inRelocations, size );
    if (inOffset != 0) {
        for (int relIndex = 0; relIndex < inCount; relIndex++) {
            mRelocations[relIndex] -= inOffset;
        }
    }
    ::qsort( mRelocations, inCount, sizeof(KUInt32), compare_relocations);
    mRelocationCount = inCount;
}

// -------------------------------------------------------------------------- //
//  * GetRelocationPageCountEstimate( void )
// -------------------------------------------------------------------------- //
KUInt32
TDCLPkgRelocatablePart::GetRelocationPageCountEstimate( void ) const
{
    if (mRelocations == NULL || mRelocationCount == 0)
    {
        return 0;
    }
    KUInt32 firstRelocation = mRelocations[0];
    KUInt32 lastRelocation = mRelocations[mRelocationCount - 1];
    // single value -> 1 page
    if (lastRelocation == firstRelocation) {
        return 1;
    }
    // delta less than RELOCATION_PAGE_SIZE -> 2 pages.
    return 2 + (lastRelocation - firstRelocation) / RELOCATION_PAGE_SIZE;
}

// -------------------------------------------------------------------------- //
//  * IsOffsetDependant( void ) const
// -------------------------------------------------------------------------- //
Boolean
TDCLPkgRelocatablePart::IsOffsetDependant( void ) const
{
    return mRelocationCount > 0;
}

// -------------------------------------------------------------------------- //
//  * Encode( KUInt32, void**, KUInt32* )
// -------------------------------------------------------------------------- //
void
TDCLPkgRelocatablePart::Encode(
        KUInt32 inOffset,
        void** ioBuffer,
        KUInt32* ioSize )
{
    KSInt32 delta = inOffset - mBaseAddress;
    KUInt32* buffer = (KUInt32*) *ioBuffer;
    for (int i = 0; i < mRelocationCount; i++)
    {
        KUInt32 relocationAddr = mRelocations[i];
        if (relocationAddr > *ioSize)
        {
            throw DCLBadParamError;
        }
        KUInt32 previousValue = UByteSex_FromBigEndian(buffer[relocationAddr / 4]);
        KUInt32 newValue = previousValue + delta;
        buffer[relocationAddr / 4] = UByteSex_ToBigEndian(newValue);
    }
}


// ============================================================= //
// "Remember, if it's being done correctly, here or abroad, it's //
// *not* the U.S. Army doing it!"                                //
//          -- Good Morning VietNam                              //
// ============================================================= //
