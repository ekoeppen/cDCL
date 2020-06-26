#!/bin/sh
TMPDIR=/tmp
PKGFILE=$2
DIRNAME=`dirname $2`
BASENAME=`basename $2`
MASTERFILE=$DIRNAME/DumpPkgDir-$BASENAME.master
OUTFILE=$TMPDIR/DumpPkgDir-$$.out

DUMPPKGDIR=$1
if $DUMPPKGDIR $PKGFILE > $OUTFILE ; then
    if diff -q $MASTERFILE $OUTFILE ; then
        STATUS=0
    else
        STATUS=1
    fi
else
    STATUS=1
fi

#rm -f $OUTFILE
if [ $STATUS -eq 0 ]; then
    echo "$0: SUCCEEDED"
else
    echo "$0: FAILED"
fi
exit $STATUS
