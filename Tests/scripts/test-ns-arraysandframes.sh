#!/bin/bash
# ==============================
# Fichier:			test-ns-framekeyssort.sh
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
# $Id: test-ns-arraysandframes.sh,v 1.1 2004/03/21 16:09:30 paul Exp $
# ===========

. common.sh $*

perl tests.pl "$DCLTESTS" ns-arraysandframes

## =================================================================== ##
## Heuristics are bug ridden by definition.  If they didn't have bugs, ##
## then they'd be algorithms.                                          ##
## =================================================================== ##
