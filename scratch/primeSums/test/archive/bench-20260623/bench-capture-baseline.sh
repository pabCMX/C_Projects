#!/usr/bin/env bash
# Capture legacy primeSearch variant wall times once per 2^exp.
# Output: bench-baseline-powers.txt (read by bench-sweep.sh).
#
# Usage:
#   ./bench-capture-baseline.sh           # exp 4..42
#   ./bench-capture-baseline.sh 4 30      # exp 4..30
#   OUT=bench-baseline-powers.txt ./bench-capture-baseline.sh
#
# Requires: bash, make, gcc, python3

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"

BUILDDIR=${BUILDDIR:-build}
OUT=${OUT:-bench-baseline-powers.txt}

ORIG=./${BUILDDIR}/primeSearch.exe
BIT=./${BUILDDIR}/primeSearchBitArrays.exe
BLK=./${BUILDDIR}/primeSearchBlockScheduleWithRollingOffsets.exe
OPT=./${BUILDDIR}/primeSearchPresieveOptimized.exe

EXP_MIN=${1:-4}
EXP_MAX=${2:-42}

if ! [[ "$EXP_MIN" =~ ^[0-9]+$ && "$EXP_MAX" =~ ^[0-9]+$ && "$EXP_MIN" -le "$EXP_MAX" ]]; then
    echo "Usage: $0 [exp_min] [exp_max]  (integers, exp_min <= exp_max)" >&2
    exit 1
fi

run_once() {
    local bin=$1 end=$2
    python3 - "$bin" "$end" <<'PY'
import subprocess, sys, time

bin_path, end = sys.argv[1], int(sys.argv[2])
try:
    t0 = time.perf_counter()
    r = subprocess.run(
        [bin_path, str(end), "--sum-only"],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    elapsed = time.perf_counter() - t0
    if r.returncode != 0:
        print("-\t-")
    else:
        print(f"{elapsed:.6f}\t{r.stdout.decode().strip()}")
except Exception:
    print("-\t-")
PY
}

echo "Building legacy variants (-O3 -march=native)..." >&2
make "${ORIG#./}" "${BIT#./}" "${BLK#./}" "${OPT#./}"

tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

{
    echo "# bench-baseline-powers.txt"
    echo "# generated_by=bench-capture-baseline.sh"
    echo "# exp_min=${EXP_MIN} exp_max=${EXP_MAX}"
    echo "# columns: exp orig_wall orig_sum bit_wall bit_sum blk_wall blk_sum opt_wall opt_sum"
    echo -e "exp\torig_wall\torig_sum\tbit_wall\tbit_sum\tblk_wall\tblk_sum\topt_wall\topt_sum"

    for exp in $(seq "$EXP_MIN" "$EXP_MAX"); do
        end=$((1 << exp))
        echo "baseline 2^$exp ($end)..." >&2

        read -r orig_wall orig_sum <<< "$(run_once "$ORIG" "$end")"
        read -r bit_wall bit_sum <<< "$(run_once "$BIT" "$end")"
        read -r blk_wall blk_sum <<< "$(run_once "$BLK" "$end")"
        read -r opt_wall opt_sum <<< "$(run_once "$OPT" "$end")"

        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$exp" "$orig_wall" "$orig_sum" "$bit_wall" "$bit_sum" \
            "$blk_wall" "$blk_sum" "$opt_wall" "$opt_sum"
    done
} >"$tmp"

mv "$tmp" "$OUT"
trap - EXIT

echo "Wrote $OUT (exp $EXP_MIN..$EXP_MAX)." >&2
