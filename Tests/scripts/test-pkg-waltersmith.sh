#!/bin/bash
# ==============================
# Fichier:			test-pkg-waltersmith.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
#
# Créé le:			28/2/2003
# Tabulation:		4 espaces
#
# Copyright:		© 2004 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-pkg-waltersmith.sh,v 1.1 2004/02/28 12:46:34 paul Exp $
# ===========

#
# Le décodage de notre binaire est le suivant:
# obj1: 0x00
# 0x000	00001041	// tableau racine, 0x10 octets.
# 0x004	00000000
# 0x008	00000002	// classe = NIL
# 0x00C	00000011	// &obj2
# obj2: 0x10
# 0x010	00002443	// structure, 0x24 octets.
# 0x014	00000000
# 0x018	00000035	// map = &obj3
# 0x01C	00000239	// &238
# 0x020	00020226
# 0x024	00000239	// &238
# 0x028	000001A1	// &1A0
# 0x02C	00000008	// int(2)
# 0x030	000000F1	// &F0
# obj3: 0x34
# 0x034	00002841	// tableau, 0x28 octets.
# 0x038	00000000
# 0x03C	00000004	// classe = int(1)
# 0x040	00000002	// supermap = NIL
# 0x044	000000D5	// &obj9
# 0x048	000000BD	// &obj8
# 0x04C	000000A5	// &obj7
# 0x050	0000008D	// &obj6
# 0x054	00000075	// &obj5
# 0x058	0000005D	// &obj4
# obj4: 0x5C
# 0x05C	00001740	// binaire, 0x17 octets.
# 0x060	00000000
# 0x064	00055552	// symbole.
# 0x068	E9E63225	// hash
# 0x06C	70686F6E	// "phones"
# 0x070	657300
# 0x073	      00	// alignement.
# obj5: 0x74
#		00001540	// binaire, 0x15 octets.
#		00000000
#		00055552	// symbole.
#		CACB2B13	// hash.
#		63617473	// "cats"
#		00
#		  000000	// alignement.
# obj6: 0x8C
#	00001740		// binaire, 0x17 octets.
#	00000000
#	00055552		// symbole.
#	AD773EB3		// hash.
#	626F756E		// "bounds"
#	647300
#	      00
# obj7: 0xA4
#	00001540		// binaire, 0x15 octets.
#	00000000
#	00055552		// symbole.
#	9CA069D9		// hash.
#	6E616D65		// "name"
#	00
#	  000000
# obj8: 0xBC
#	00001640		// binaire, 0x16 octets.
#	00000000
#	00055552		// symbole.
#	4A65671B		// hash.
#	75636861		// "uchar"
#	7200
#	    0000
# obj9: 0xD4
#	00001A40		// binaire, 0x1A octets.
#	00000000
#	00055552
#	28E7C839		// hash.
#	6E616D65		// nameAgain
#	41676169
#	6E00
#		0000
# obj10: 0xF0
#	00001441		// tableau, 0x14 octets.
#	00000000
#	00000189		// &188
#	00000149		// &148
#	00000105		// &104
# obj11: 0x104
#	00002640		// binaire, 0x26 octets.
#	00000000
#	0000012D		// classe: &12C
#	00340030
#	0038002D
#	00390037
#	0034002D
#	00390030
#	00390034
#	0000
#		0000
# obj12: 0x12C
#	00001940		// binaire, 0x19 octets.
#	00000000
#	00055552
#	703CC351		// hash.
#	66617850
#	686F6E65
#	00
#	  000000
# obj13: 0x
#	0000264000000000
#00000171003400300038002D00390039
#0036002D003100300031003000000000
#0000174000000000000555521810F35F
#737472696E6700000000164000000000
#00055552B4FF1BC76172726179000000
#00001C4300000000000001BD000000A0
#00000038000000280000019000002041
#00000000000000040000000200000221
#0000020D000001F5000001DD00001740
#0000000000055552DBA1FFED626F7474
#6F6D0000000015400000000000055552
#CACB2B136C6566740000000000001440
#00000000000555522EA88A9B746F7000
#00001640000000000005555216C7A20E
#72696768740000000000264000000000
#0000017100570061006C007400650072
#00200053006D00690074006800000000

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

perl tests.pl "$DCLTESTS" pkg-waltersmith

## ===================================================================== ##
## Computer programmers never die, they just get lost in the processing. ##
## ===================================================================== ##
