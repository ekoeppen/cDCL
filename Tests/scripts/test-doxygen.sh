#!/bin/bash
# Doxygen warnings check (requires doxygen installed).

set -e

rm -rf DoxyOutput
mkdir -p DoxyOutput/html

doxygen > /dev/null 2>&1

if [ -s DoxyOutput/doxywarnings.txt ]; then
    cat DoxyOutput/doxywarnings.txt >&2
    rm -rf DoxyOutput
    exit 1
fi

rm -rf DoxyOutput
