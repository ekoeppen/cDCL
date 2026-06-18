#!/bin/bash
# Test TMappedFile functionality

DIR="$(cd "$(dirname "$0")" && pwd)"

$1 kmappedfile | tr -d '\r' | diff -u - $DIR/master-test-kmappedfile
