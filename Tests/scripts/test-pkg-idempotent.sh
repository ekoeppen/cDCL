#!/bin/bash

. common.sh $*
perl tests.pl "$DCLTESTS" pkg-idempotent
