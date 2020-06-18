#!/bin/bash
# ==============================
# Fichier:			testall.sh
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
# $Id: testall.sh,v 1.4 2003/06/14 16:43:52 paul Exp $
# ===========

# Script pour appeler tous les tests non manuels.
for test in test-*.sh
do
	./$test $*
done

## ================================================== ##
## Row, row, row your bits, gently down the stream... ##
## ================================================== ##
