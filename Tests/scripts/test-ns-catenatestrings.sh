#!/bin/bash
# ==============================
# Fichier:			test-ns-catenatestrings.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
#
# Créé le:			3/5/2003
# Tabulation:		4 espaces
#
# Copyright:		© 2003 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-ns-catenatestrings.sh,v 1.3 2003/06/21 08:37:22 paul Exp $
# ===========

. common.sh $*

perl tests.pl "$DCLTESTS" ns-catenatestrings

## ================================================================== ##
## In seeking the unattainable, simplicity only gets in the way.      ##
##                 -- Epigrams in Programming, ACM SIGPLAN Sept. 1982 ##
## ================================================================== ##
