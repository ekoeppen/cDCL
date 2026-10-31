// ==============================
// Fichier:			UTestKMappedFile.h
// Projet:			cDCL
// Ecrit par:		Eckhart Koeppen (eck@40hz.org)
// 
// Cr le:			31/10/2026
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
// The Original Code is UTestKMappedFile.h.
// 
// The Initial Developer of the Original Code is Eckhart Koeppen.
// Portions created by the Initial Developer are Copyright (C) 2026 the
// Initial Developer. All Rights Reserved.
// 
// Contributor(s):
//   Eckhart Koeppen <eck@40hz.org> (original author)
// 
// ***** END LICENSE BLOCK *****

#ifndef _UTESTKMAPPEDFILE_H
#define _UTESTKMAPPEDFILE_H

#include <K/Defines/KDefinitions.h>

class UTestKMappedFile
{
public:
	/// Run all TMappedFile tests.
	static void Run(void);

private:
	static void TestMapAndGrow(void);
	static void TestReadWriteSync(void);
	static void TestReadOnly(void);
};

#endif
		// _UTESTKMAPPEDFILE_H
