#!/bin/bash
. common.sh $*

set -e
perl tests.pl "$DCLTESTS" wakeable-wait
