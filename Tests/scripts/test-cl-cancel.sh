#!/bin/bash

. common.sh $*

perl tests.pl "$DCLTESTS" cl-cancel
