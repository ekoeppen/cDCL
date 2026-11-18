#!/bin/bash
# Manual test for BSD socket connect & disconnect.

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <path to DCLManualTests>"
    exit 1
fi

echo "This is a manual test. The tester must evaluate whether the test passed."
"$1" cl-cad-bsd
