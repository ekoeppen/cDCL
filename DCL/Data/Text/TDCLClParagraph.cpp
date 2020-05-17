// ==============================
// Fichier:			TDCLClParagraph.cp
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
// The Original Code is TDCLClParagraph.cp.
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
// $Id: TDCLClParagraph.cp,v 1.4 2004/11/24 18:11:15 paul Exp $
// ===========

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Data/Text/TDCLClParagraph.h>

// ANSI
#include <string.h>

// DCL
#include <DCL/Data/Text/TDCLFontSpecification.h>
#include <DCL/Exceptions/TDCLNSException.h>
#include <DCL/Exceptions/Errors/TDCLNotImplementedError.h>
#include <DCL/NS_Objects/Objects/KDCLSYM.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSString.h>

// -------------------------------------------------------------------------- //
//  * TDCLClParagraph( void )
// -------------------------------------------------------------------------- //
TDCLClParagraph::TDCLClParagraph( void )
{
	mFrameRef = TDCLNSRef::MakeFrame();
	mFrame = &mFrameRef.ToFrame();
	
	// Ajout du texte.
	mFrame->Set( KDCLSYM::kSYMtext, TDCLNSRef::MakeString("") );
}

// -------------------------------------------------------------------------- //
//  * TDCLClParagraph( const TDCLNSRef& )
// -------------------------------------------------------------------------- //
TDCLClParagraph::TDCLClParagraph( const TDCLNSRef& inFrame )
{
	mFrameRef = inFrame;
	mFrame = &mFrameRef.ToFrame();
}

// -------------------------------------------------------------------------- //
//  * TDCLClParagraph( const TDCLRichText& )
// -------------------------------------------------------------------------- //
TDCLClParagraph::TDCLClParagraph( const TDCLRichText& inCopy )
{
	// Création de la structure.
	mFrameRef = TDCLNSRef::MakeFrame();
	mFrame = &mFrameRef.ToFrame();
	
	// Extraction du texte.
	KUInt32 theSize = inCopy.GetSize();
	KUInt16* theText = new KUInt16[theSize + 1];
	theText[theSize] = 0;
	
	inCopy.GetText( 0, theSize, theText );
	
	// Ajout du texte.
	mFrame->Set( KDCLSYM::kSYMtext, TDCLNSRef::MakeString( theText ) );
	
	delete [] theText;
	
	// Ajout des styles.
	KUInt32 nbStyles = inCopy.CountStyleRuns();
	if (nbStyles == 1)
	{
		TDCLStyleRef theStyleRef = inCopy.GetStyle( 0 );
		if (theStyleRef.IsPicture())
		{
			throw DCLNotImplementedError;
			#warning TODO.
		}
		mFrame->Set(
			KDCLSYM::kSYMviewFont,
			((TDCLFontSpecification&) theStyleRef).GetNSRef() );
	} else {
		TDCLNSRef theStylesArrayRef = TDCLNSRef::MakeArray();
		TDCLNSArray& theStylesArray = theStylesArrayRef.ToArray();
		KUInt32 indexStyles;
		for (indexStyles = 0; indexStyles < nbStyles; indexStyles++)
		{
			theStylesArray.Add(
				TDCLNSRef::MakeInt(
					(KSInt32) inCopy.GetStyleRunLength( indexStyles ) ) );
			TDCLStyleRef theStyleRef = inCopy.GetStyle( indexStyles );
			if (theStyleRef.IsPicture())
			{
				throw DCLNotImplementedError;
				#warning TODO.
			}
			theStylesArray.Add(
				((TDCLFontSpecification&) theStyleRef ).GetNSRef() );
		}
		
		mFrame->Set( KDCLSYM::kSYMstyles, theStylesArrayRef );
	}
}

// -------------------------------------------------------------------------- //
//  * Catenate( const TDCLRichText& inAlter )
// -------------------------------------------------------------------------- //
void
TDCLClParagraph::Catenate( const TDCLRichText& inAlter )
{
#warning TODO.
}

// -------------------------------------------------------------------------- //
//  * GetSize( void ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLClParagraph::GetSize( void ) const
{
	return mFrame->Get( KDCLSYM::kSYMtext ).ToString().GetStrLen();
}

// -------------------------------------------------------------------------- //
//  * GetText( KUInt32, KUInt32, KUInt16* ) const
// -------------------------------------------------------------------------- //
void
TDCLClParagraph::GetText(
						KUInt32 inOffset,
						KUInt32 inCount,
						KUInt16* outChars ) const
{
	TDCLNSString& theString = mFrame->Get( KDCLSYM::kSYMtext ).ToString();
	KUInt32 theLength = theString.GetStrLen();
	
	if (theLength < inOffset + inCount)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}
	
	const KUInt16* theChars = theString.GetString();
	(void) ::memcpy(
				outChars, &theChars[inOffset], sizeof( KUInt16 ) * inCount );
}

// -------------------------------------------------------------------------- //
//  * CountStyleRuns( void ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLClParagraph::CountStyleRuns( void ) const
{
	// Deux cas:
	// On a un élément styles ou pas.
	// Si on en a un, le nombre de style runs est égal au nombre d'éléments
	// dans ce tableau divisé par deux.
	// Sinon, c'est un.
	
	KUInt32 theResult = 1;
	if (mFrame->HasSlot( KDCLSYM::kSYMstyles ))
	{
		theResult =
			mFrame->Get( KDCLSYM::kSYMstyles ).ToArray().GetLength() / 2;
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetStyleRunStart( KUInt32 ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLClParagraph::GetStyleRunStart( KUInt32 inStyleRunIndex ) const
{
	// Deux cas:
	// On a un élément styles ou pas.

	KUInt32 theResult = 0;
	if (mFrame->HasSlot( KDCLSYM::kSYMstyles ))
	{
		TDCLNSArray& stylesArray =
			mFrame->Get( KDCLSYM::kSYMstyles ).ToArray();

		// Calcul de la position.
		KUInt32 indexStyle;
		for (indexStyle = 0; indexStyle < inStyleRunIndex; indexStyle++)
		{
			theResult += stylesArray.Get( indexStyle * 2 ).ToInt();
		}
	} else {
		if (inStyleRunIndex > 0)
		{
			throw DCLNS( kNSErrOutOfBounds );
		}
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetStyleRunLength( KUInt32 ) const
// -------------------------------------------------------------------------- //
KUInt32
TDCLClParagraph::GetStyleRunLength( KUInt32 inStyleRunIndex ) const
{
	// Deux cas:
	// On a un élément styles ou pas.

	KUInt32 theResult = 0;
	if (mFrame->HasSlot( KDCLSYM::kSYMstyles ))
	{
		theResult = (KUInt32) mFrame->Get( KDCLSYM::kSYMstyles ).ToArray()
											.Get( inStyleRunIndex ).ToInt();
	} else {
		if (inStyleRunIndex > 0)
		{
			throw DCLNS( kNSErrOutOfBounds );
		}
		
		theResult = GetSize();
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * GetStyle( KUInt32 ) const
// -------------------------------------------------------------------------- //
TDCLStyleRef
TDCLClParagraph::GetStyle( KUInt32 inStyleRunIndex ) const
{
	// Deux cas:
	// On a un élément styles ou pas.

	TDCLStyleRef theResult;
	if (mFrame->HasSlot( KDCLSYM::kSYMstyles ))
	{
		TDCLNSRef theStyle = mFrame->Get( KDCLSYM::kSYMstyles ).ToArray()
											.Get( inStyleRunIndex + 1 );
		if (theStyle.IsBinary())
		{
		} else {
			theResult = new TDCLFontSpecification( theStyle );
		}
	} else {
		if (inStyleRunIndex > 0)
		{
			throw DCLNS( kNSErrOutOfBounds );
		}
		
		theResult = new TDCLFontSpecification( mFrame->Get( KDCLSYM::kSYMviewFont ) );
	}
	
	return theResult;
}

// -------------------------------------------------------------------------- //
//  * SetStyle( KUInt32, KUInt32, TDCLStyleRef )
// -------------------------------------------------------------------------- //
void
TDCLClParagraph::SetStyle(
								KUInt32 inTextIndex,
								KUInt32 inTextLength,
								TDCLStyleRef inStyle )
{
	KASSERT( inTextLength > 0 );

	// Taille du texte actuel.
	TDCLNSString& theString = mFrame->Get( KDCLSYM::kSYMtext ).ToString();
	KUInt32 theLength = theString.GetStrLen();
	KUInt32 inTextEnd = inTextIndex + inTextLength;
	
	if (theLength < inTextEnd)
	{
		throw DCLNS( kNSErrOutOfBounds );
	}

	// Une image ne peut s'appliquer qu'à un seul caractère.
	if (inStyle.IsPicture())
	{
		if (inTextLength != 1)
		{
			throw DCLNS( kNSErrOutOfBounds );
		}
		
		throw DCLNotImplementedError;
#warning TODO.
	} else {
		if (inTextLength == theLength)
		{
			// On vire styles (si on avait styles)
			// et on met un viewFont.
			
			(void) mFrame->RemoveSlot( KDCLSYM::kSYMstyles );
			
			mFrame->Set( KDCLSYM::kSYMviewFont,
				TDCLFontSpecification( inStyle ).GetNSRef() );
		} else {
			// Si on avait un viewFont, on remplace par un styles.
			// Sinon, on modifie le tableau styles.
			if (mFrame->HasSlot( KDCLSYM::kSYMstyles ))
			{
				// Ici, on a déjà plusieurs styles.
				// Déterminons où ce nouveau style va s'insérer.
				TDCLNSArray& theStylesArray = mFrame->Get( KDCLSYM::kSYMstyles ).ToArray();
				KUInt32 nbStyles = theStylesArray.GetLength() / 2;
				KUInt32 textCursor = 0;

				// Début.
				KUInt32 indexStyles;
				KUInt32 styleLength;
				for (indexStyles = 0; indexStyles < nbStyles; indexStyles++)
				{
					styleLength = (KUInt32) theStylesArray.Get( indexStyles ).ToInt();
					if (inTextIndex < textCursor + styleLength)
					{
						// On a trouvé le début.
						break;
					}
					
					textCursor += styleLength;
				}
				
				KASSERT( indexStyles < nbStyles );
				
				TDCLFontSpecification theOriginalStyle(
									theStylesArray.Get( indexStyles + 1 ) );
				
				if (theOriginalStyle == (TDCLFontSpecification&) inStyle)
				{
					// Extension du style original au-delà de la taille.
					if (textCursor + styleLength < inTextEnd)
					{
						
					} // Sinon, rien à faire, le texte n'est pas modifié.
				} else {
					// Deux cas: le style qu'on a trouvé commence là où on veut
					// changer le style ou pas.
					if (inTextIndex > textCursor)
					{
						// On divise le style actuel.
					}
				}
				
				
				KUInt32 thePreviousStyle = indexStyles;

				// Fin
				for ( ; indexStyles < nbStyles; indexStyles++)
				{
					if (inTextEnd < textCursor + styleLength)
					{
						// On a trouvé la fin.
						break;
					}
					textCursor += styleLength;
				}

				KUInt32 theNextStyle = indexStyles;
				
				// Cas 1:
				// 
			} else {
				TDCLFontSpecification& theFontSpec = inStyle;
				TDCLNSRef theViewFont = mFrame->Get( KDCLSYM::kSYMviewFont );
				// On ne modifie quelque chose que si le style fourni en paramètre
				// n'est pas déjà celui de notre paragraphe.
				if (theFontSpec != TDCLFontSpecification( theViewFont ))
				{
					TDCLNSRef theStyleRef =
								TDCLFontSpecification( theFontSpec ).GetNSRef();

					TDCLNSRef theStylesArrayRef = TDCLNSRef::MakeArray();
					TDCLNSArray& theStylesArray = theStylesArrayRef.ToArray();
					
					// Le style original.
					if (inTextIndex > 0)
					{
						theStylesArray.Add(
								TDCLNSRef::MakeInt( (KSInt32) inTextIndex ) );
						theStylesArray.Add( theViewFont );
					}
					
					// Le style fourni en paramètre.
					theStylesArray.Add(
								TDCLNSRef::MakeInt( (KSInt32) inTextLength ) );
					theStylesArray.Add( theStyleRef );
					
					// Et à nouveau le style original.
					KUInt32 theLastRunLength = theLength - inTextEnd;
					if (theLastRunLength > 0)
					{
						theStylesArray.Add(
							TDCLNSRef::MakeInt( (KSInt32) theLastRunLength ) );
						theStylesArray.Add( theViewFont );
					}
					
					// Ajout dans le paragraphe.
					(void) mFrame->RemoveSlot( KDCLSYM::kSYMviewFont );
					mFrame->Set( KDCLSYM::kSYMstyles, theStylesArrayRef );
				} // if (theFontSpec != TDCLFontSpecification( theViewFont ))
			}
		}
	}
}

// ==================================================================== //
//         THE LESSER-KNOWN PROGRAMMING LANGUAGES #13: SLOBOL           //
//                                                                      //
// SLOBOL is best known for the speed, or lack of it, of its compiler.  //
// Although many compilers allow you to take a coffee break while they  //
// compile, SLOBOL compilers allow you to travel to Bolivia to pick the //
// coffee.  Forty-three programmers are known to have died of boredom   //
// sitting at their terminals while waiting for a SLOBOL program to     //
// compile.  Weary SLOBOL programmers often turn to a related (but      //
// infinitely faster) language, COCAINE.                                //
// ==================================================================== //
