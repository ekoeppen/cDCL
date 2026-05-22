// ==============================
// Fichier:			TDCLGenericIOException.h
// Projet:			Desktop Connection Library
//
// Créé le:			12/05/2026
// ===========

#ifndef _TDCLGENERICIOEXCEPTION_H
#define _TDCLGENERICIOEXCEPTION_H

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>

/// \name macros pour l'origine de l'exception
//@{
#if kDCLDebugFILEandLINE
	#define DCLIOException							\
		TDCLGenericIOException( __FILE__, __LINE__ )
	#define DCLPlatformIOException( inError )		\
		TDCLGenericIOException( __FILE__, __LINE__, inError )
#else
	#define DCLIOException							\
		TDCLGenericIOException()
	#define DCLPlatformIOException( inError )		\
		TDCLGenericIOException( inError )
#endif
//@}

class TDCLGenericIOException
	:
		public TDCLIOException
{
public:
#if kDCLDebugFILEandLINE
	TDCLGenericIOException(
			const char* inFileName,
			KUInt32 inLine,
			KSInt32 inError = 0 )
        : TDCLIOException( exUnknownError, inError, inFileName, inLine ) {}
#else
	TDCLGenericIOException( KSInt32 inError = 0 )
        : TDCLIOException( exUnknownError, inError ) {}
#endif
};

#endif
