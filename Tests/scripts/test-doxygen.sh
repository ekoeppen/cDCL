#!/bin/bash
# ==============================
# Fichier:			test-doxygen.sh
# Projet:			(Desktop Connection Library)
# Ecrit par:		Paul Guyot (pguyot@kallisys.net)
#
# CrŽŽ le:			2/5/2003
# Tabulation:		4 espaces
#
# Copyright:		© 2003-2004 by Paul Guyot.
# 					Tous droits rŽservŽs pour tous pays.
# Licence:			Licence RŽflexive Kallisys
# 					http://www.kallisys.org/reflexive/
# ===========
# $Id: test-doxygen.sh,v 1.9 2004/05/11 21:28:54 paul Exp $
# ===========

. common.sh $*

# Test pour doxygen.
# Requiert que doxygen soit installŽ et correctement configurŽ.

if [ -e DoxyOutput ] ; then
	rm -f DoxyOutput/doxywarnings.txt
fi

mkdir -p DoxyOutput/html/

doxygen 2>&1 > /dev/null
cat DoxyOutput/doxywarnings.txt > $OUTFILE

cat > $MASTERFILE <<ENDMASTER
ENDMASTER

compare

# Nettoyage
rm $OUTFILE
rm $MASTERFILE
rm -f DoxyOutput/doxywarnings.txt
rm -rf DoxyOutput/

## ====================================================================== ##
## Documentation is like sex: when it is good, it is very, very good; and ##
## when it is bad, it is better than nothing.                             ##
##                 -- Dick Brandon                                        ##
## ====================================================================== ##
