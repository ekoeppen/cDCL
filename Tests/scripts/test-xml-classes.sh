#!/bin/bash
# ==============================
# Fichier:			test-xml-classes.sh
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
# $Id: test-xml-classes.sh,v 1.6 2003/06/21 08:37:22 paul Exp $
# ===========

. common.sh $*

perl tests.pl "$DCLTESTS" xml-classes

## ======================================================================== ##
## The last time somebody said, "I find I can write much better with a word ##
## processor.", I replied, "They used to say the same thing about drugs."   ##
##                 -- Roy Blount, Jr.                                       ##
## ======================================================================== ##
