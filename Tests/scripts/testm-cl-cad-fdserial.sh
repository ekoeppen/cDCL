#!/bin/bash
# Manual test for POSIX serial connect & disconnect.

if [ $# -ne 2 ]; then
    echo "Syntax: $0 <path to DCLManualTests> <serial port>"
    exit 1
fi

echo "This is a manual test. The tester must evaluate whether the test passed."
"$1" cl-cad-fdserial "$2"
