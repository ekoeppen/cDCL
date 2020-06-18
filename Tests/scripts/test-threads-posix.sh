#!/bin/bash
#==============================
# Fichier:			test-threads-posix.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
# 
# Créé le:			14/6/2003
# Tabulation:		4 espaces
# 
# Copyright:		© 2003 by Paul Guyot.
# 					Tous droits réservés pour tous pays.
# Licence:			Licence Réflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-threads-posix.sh,v 1.2 2003/06/21 08:37:22 paul Exp $
# ===========

. common.sh $*

set -e
perl tests.pl "$DCLTESTS" threads-posix

## ================================================== ##
## All your files have been destroyed (sorry).  Paul. ##
## ================================================== ##
