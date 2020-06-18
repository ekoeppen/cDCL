#!/bin/bash
#==============================
# Fichier:			test-cl-sast-bsd.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
# 
# Créé le:			21/6/2003
# Tabulation:		4 espaces
# 
# Copyright:		© 2003 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-cl-sast-bsd.sh,v 1.1 2003/06/21 08:37:21 paul Exp $
# ===========

. common.sh $*

perl tests.pl "$DCLTESTS" cl-sast-bsd

## ==================================================================== ##
## Real programmers don't comment their code.  It was hard to write, it ##
## should be hard to understand.                                        ##
## ==================================================================== ##
