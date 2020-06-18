#!/bin/bash
#==============================
# Fichier:			testm-cl-cad-bsd.sh
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
# $Id: testm-cl-cad-bsd.sh,v 1.3 2004/01/03 18:56:46 paul Exp $
# ===========

. common.sh $*

echo "Ceci est un test manuel. L'experimentateur doit evaluer si le test s'est bien passe."

"$DCLTESTS" cl-cad-bsd

## ====================================================== ##
## No wonder Clairol makes so much money selling shampoo. ##
## Lather, Rinse, Repeat is an infinite loop!             ##
## ====================================================== ##
