#!/bin/bash
# ==============================
# Fichier:			test-xml-waltersmith.sh
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
# $Id: test-xml-waltersmith.sh,v 1.6 2003/06/21 08:37:22 paul Exp $
# ===========

. common.sh $*

perl tests.pl "$DCLTESTS" xml-waltersmith

## ======================================================================= ##
## I cannot conceive that anybody will require multiplications at the rate ##
## of 40,000 or even 4,000 per hour ...                                    ##
##                 -- F. H. Wales (1936)                                   ##
## ======================================================================= ##
