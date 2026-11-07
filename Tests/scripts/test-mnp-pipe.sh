#!/bin/bash
# Test the TDCLMNPPipe facade
set -o pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"

$1 mnp-pipe | tr -d '\r' | diff -u - $DIR/master-test-mnp-pipe
