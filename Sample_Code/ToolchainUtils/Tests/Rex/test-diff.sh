#!/bin/sh
TMPDIR=/tmp
SRCFILE=$2
MASTERFILE=$SRCFILE.master
OUTFILE=$TMPDIR/rex-test-diff-$$.out

REX=$1
echo $REX -o $OUTFILE $SRCFILE
if $REX -o $OUTFILE $SRCFILE ; then
    if diff -q $MASTERFILE $OUTFILE ; then
        STATUS=0
    else
        STATUS=1
    fi
else
    STATUS=1
fi

rm -f $OUTFILE
if [ $STATUS -eq 0 ]; then
    echo "$0: SUCCEEDED"
else
    echo "$0: FAILED"
fi
exit $STATUS

