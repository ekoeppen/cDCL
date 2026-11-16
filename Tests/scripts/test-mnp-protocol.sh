#!/bin/bash
# Test TMNPProtocol state machine and codec

DIR="$(cd "$(dirname "$0")" && pwd)"

$1 mnp-protocol | tr -d '\r' | diff -u - $DIR/master-test-mnp-protocol
