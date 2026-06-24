#!/usr/bin/env bash
# Thin wrapper around the PrimeSum Python harness.
#
# Examples:
#   ./prime-suite.sh test ../mine/build/PrimeSum6RollingCursors.exe
#   ./prime-suite.sh bench ../mine/build/PrimeSum6RollingCursors.exe
#   ./prime-suite.sh capture-sums ../ai/build/primeSearchPresieve19RangeCursorsFastCross.exe
#   ./prime-suite.sh capture-all --time-budget 7200

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
exec python3 "$SCRIPT_DIR/prime_harness.py" "$@"
