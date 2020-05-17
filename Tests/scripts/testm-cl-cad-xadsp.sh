#!/bin/sh
#==============================
# Fichier:			testm-cl-cad-xadsp.sh
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
# $Id: testm-cl-cad-xadsp.sh,v 1.1 2003/06/21 13:32:28 paul Exp $
# ===========

. common.sh $*

echo "Ceci est un test manuel. L'expérimentateur doit évaluer si le test s'est bien passé."

"$DCLTESTS" cl-cad-xadsp

## ========================================================================== ##
## No proper program contains an indication which as an operator-applied      ##
## occurrence identifies an operator-defining occurrence which as an          ##
## indication-applied occurrence identifies an indication-defining occurrence ##
## different from the one identified by the given indication as an            ##
## indication-applied occurrence.                                             ##
##                 -- ALGOL 68 Report                                         ##
## ========================================================================== ##
