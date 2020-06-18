#!/bin/bash
# ==============================
# Fichier:			test-nsof-waltersmith.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
#
# Créé le:			2/5/2003
# Tabulation:		4 espaces
#
# Copyright:		© 2003 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-nsof-waltersmith.sh,v 1.6 2003/06/21 08:37:22 paul Exp $
# ===========

#
# Il y a une légère différence avec le NSOF de l'ami Walter.
# 02060607046E616D650704636174730706626F756E6473070575636861720706
# 70686F6E657307096E616D65416761696E081A00570061006C00740065007200
# 200053006D006900740068000000080B0E0A64280220220502081A0034003000
# 38002D003900390036002D00310030003100300000031A070866617850686F6E
# 65003400300038002D003900370034002D003900300039003400000907
#
# Nos éléments sont dans un autre ordre parce que nous trions les
# clés automatiquement lorsqu'elles sont ajoutées par la méthode Set.
#
# Note: le Newton non plus ne trie pas les éléments pour un si petit
# exemple.
# x := {name: "Walter Smith",
#	cats: 2,
#	bounds: {left: 10, top: 14, right: 40, bottom: 100},
#	uchar: $\u2022,
#	phones: ["408-996-1010", nil]};
# x.phones[1] := SetClass("408-974-9094", 'faxPhone);
# x.nameAgain := x.name;
# StrHexDump(Translate(x, 'flattener, GetStores()[0], nil), 32);
#
# Cf NewtonFormats pour le décodage du binaire NSOF de Walter Smith.
#
# Le décodage de notre binaire est le suivant:
# 02-version
# 06-kFrame
# 06-length, 6 slots
# Slot tags:
#   07 (kSymbol) 09 (length of nameAgain) 6E616D65416761696E ("nameAgain")
#   07 05 7563686172 ("uchar")
#   07 04 6E616D65 ("name")
#   07 06 626F756E6473 ("bounds")
#   07 04 63617473 ("cats")
#   07 06 70686F6E6573 ("phones")
# Slot values:
#   08-kString
#   1A-length, 26 bytes
#   00570061006C00740065007200200053006D0069007400680000
#   02-kUnicodeCharacter
#   2022-The character code
#   09-kPrecedent
#   07-ID 7 ("Walter Smith" object above)
#   0B-kSmallRect
#   0E (top) 14 (left=10) 64 (bottom=100) 28 (right=40)
#   00-kImmediate
#   08-Ref of the integer 2
#   05-kPlainArray
#   02-length, 2 slots
#   Slot values:
#     08-kString
#     1A-length, 26 bytes
#     003400300038002D003900390036002D00310030003100300000
#     03-kBinaryObject
#     1A-length, 26 bytes
#     Class:
#       07 (kSymbol) 08 (length of name) 66617850686F6E65
#     003400300038002D003900370034002D00390030003900340000

. common.sh $*

perl tests.pl "$DCLTESTS" nsof-waltersmith

## ===================================================================== ##
## Computer programmers never die, they just get lost in the processing. ##
## ===================================================================== ##
