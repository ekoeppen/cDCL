#!/bin/bash
. common.sh $*

set -e
perl tests.pl "$DCLTESTS" event-queue
