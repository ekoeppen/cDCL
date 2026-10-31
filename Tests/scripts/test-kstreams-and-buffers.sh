#!/bin/bash
# Test K streams, circular buffers, and endianness functionality

DIR="$(cd "$(dirname "$0")" && pwd)"

$1 kstreams-and-buffers | diff -u - $DIR/master-test-kstreams-and-buffers
