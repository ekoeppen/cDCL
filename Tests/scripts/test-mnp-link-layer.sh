#!/bin/bash
# Test TDCLLink over TMNPProtocol and TDCLSessionBus

DIR="$(cd "$(dirname "$0")" && pwd)"

$1 mnp-link-layer | tr -d '\r' | diff -u - $DIR/master-test-mnp-link-layer
