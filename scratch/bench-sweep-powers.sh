#!/usr/bin/env bash
# Sweep endpoint = 2^N for both primeSearch variants.
# Highlights when the base-primes table exceeds per-physical-core L2 budget.
#
# Usage:
#   ./bench-sweep-powers.sh
#   ./bench-sweep-powers.sh 24 40          # exponents 24..40 inclusive
#   ./bench-sweep-powers.sh 28 38 --csv    # CSV to stdout
#   L2_KIB=512 SEGMENT_KIB=256 OVERHEAD_KIB=32 ./bench-sweep-powers.sh
#
# L2_KIB is per physical core. Workers should be (physical cores - 1), not logical.
# If logical > physical (SMT), two workers on one core share L2 and execution units.
#
# Requires: bash, make, gcc, python3

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"

BUILDDIR=${BUILDDIR:-build}
ORIG=./${BUILDDIR}/primeSearch.exe
BIT=./${BUILDDIR}/primeSearchBitArrays.exe

detect_cpu_topology() {
    LOGICAL=${LOGICAL:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)}
    PHYSICAL=${PHYSICAL:-0}

    if [[ -d /sys/devices/system/cpu ]]; then
        PHYSICAL=$(
            for cpu in /sys/devices/system/cpu/cpu[0-9]*; do
                [[ -f "$cpu/topology/physical_package_id" ]] || continue
                pkg=$(<"$cpu/topology/physical_package_id")
                core=$(<"$cpu/topology/core_id")
                echo "$pkg:$core"
            done | sort -u | wc -l
        )
    fi

    if [[ "$PHYSICAL" -le 0 ]]; then
        PHYSICAL=$LOGICAL
    fi

    if [[ "$LOGICAL" -gt "$PHYSICAL" ]]; then
        SMT=$((LOGICAL / PHYSICAL))
    else
        SMT=1
    fi
}

detect_cpu_topology

EXP_MIN=${1:-24}
EXP_MAX=${2:-40}
CSV=0
if [[ "${3:-}" == "--csv" ]] || [[ "${2:-}" == "--csv" ]]; then
    CSV=1
    [[ "${2:-}" == "--csv" ]] && EXP_MAX=${EXP_MIN}
fi

L2_KIB=${L2_KIB:-512}           # per physical core; 3900X=512, Xeon 6230=1024
SEGMENT_KIB=${SEGMENT_KIB:-256} # bit version @ 1ULL << 21
OVERHEAD_KIB=${OVERHEAD_KIB:-32}
RUNS=${RUNS:-3}                 # timed runs per point (best of last two kept)

echo "Building (via Makefile: -O3 -march=native)..." >&2
make "${ORIG#./}" "${BIT#./}"

base_budget_kib=$((L2_KIB - SEGMENT_KIB - OVERHEAD_KIB))
workers=$((PHYSICAL > 1 ? PHYSICAL - 1 : 1))
shared_l2_kib=$((L2_KIB / SMT))
shared_base_budget_kib=$((shared_l2_kib - SEGMENT_KIB - OVERHEAD_KIB))

run_timed() {
    local bin=$1 end=$2
    python3 - "$bin" "$end" "$RUNS" <<'PY'
import subprocess, sys, time

bin_path, end, runs = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
best = None
for i in range(runs):
    t0 = time.perf_counter()
    r = subprocess.run([bin_path, str(end), "--sum-only"],
                       stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=True)
    elapsed = time.perf_counter() - t0
    if i > 0:
        best = elapsed if best is None else min(best, elapsed)
    sum_out = r.stdout.decode().strip()
if best is None:
    best = elapsed
print(f"{best:.6f}\t{sum_out}")
PY
}

estimate_base_kib() {
    python3 - "$1" <<'PY'
import math, sys
end = int(sys.argv[1])
root = math.isqrt(end)
if root < 2:
    print("0.0", "0", "0")
    raise SystemExit
count = int(root / (math.log(root) - 1.0832))
print(f"{count * 4 / 1024:.1f}", root, count)
PY
}

if [[ $CSV -eq 1 ]]; then
    echo "exp,end,sqrt_end,base_primes,base_kib,l2_over,l2_kib,orig_wall,bit_wall,speedup,sum_ok"
else
    printf '\n'
    printf 'CPU: %s physical cores, %s logical CPUs, SMT=%sx\n' \
        "$PHYSICAL" "$LOGICAL" "$SMT"
    printf 'Workers (binaries): %s  (= physical - 1)\n' "$workers"
    if [[ "$SMT" -gt 1 ]]; then
        printf 'SMT note: if workers matched logical CPUs, effective L2 per worker ~%s KiB (base budget ~%s KiB)\n' \
            "$shared_l2_kib" "$shared_base_budget_kib"
    fi
    printf 'L2 model: %s KiB per physical core, %s KiB segment, %s KiB overhead → %s KiB base budget\n' \
        "$L2_KIB" "$SEGMENT_KIB" "$OVERHEAD_KIB" "$base_budget_kib"
    printf '%-4s %-14s %-10s %-8s %-6s %-5s %-8s %-8s %-8s %-7s %s\n' \
        'exp' 'end' 'sqrt(end)' 'base_KiB' 'L2?' 'runs' 'orig_s' 'bit_s' 'speedup' 'ok' 'notes'
    printf '%s\n' '---- -------------- ---------- -------- ------ ----- -------- -------- ------- --- -----'
fi

for exp in $(seq "$EXP_MIN" "$EXP_MAX"); do
    end=$((1 << exp))
    read -r base_kib root prime_count <<< "$(estimate_base_kib "$end")"

    l2_over=no
    if python3 - "$base_kib" "$base_budget_kib" <<'PY'
import sys
sys.exit(0 if float(sys.argv[1]) <= float(sys.argv[2]) else 1)
PY
    then
        :
    else
        l2_over=yes
    fi

    read -r orig_wall orig_sum <<< "$(run_timed "$ORIG" "$end")"
    read -r bit_wall bit_sum <<< "$(run_timed "$BIT" "$end")"

    ok=yes
    [[ "$orig_sum" == "$bit_sum" ]] || ok=MISMATCH

    speedup=$(python3 - "$orig_wall" "$bit_wall" <<'PY'
import sys
o, b = map(float, sys.argv[1:3])
print(f"{o/b:.2f}" if b > 0 else "inf")
PY
)

    note=""
    if [[ "$l2_over" == yes ]]; then
        note="base > L2 budget"
    fi

    if [[ $CSV -eq 1 ]]; then
        echo "$exp,$end,$root,$prime_count,$base_kib,$l2_over,$L2_KIB,$orig_wall,$bit_wall,$speedup,$ok"
    else
        printf '%-4s %-14s %-10s %-8s %-6s %-5s %-8s %-8s %-7s %-3s %s\n' \
            "$exp" "$end" "$root" "$base_kib" "$l2_over" "$RUNS" \
            "$orig_wall" "$bit_wall" "$speedup" "$ok" "$note"
    fi
done

if [[ $CSV -eq 0 ]]; then
    printf '\n'
    printf 'Notes:\n'
    printf '  - end = 2^exp; L2? = yes when estimated base_kib > %s KiB (one worker per physical core).\n' "$base_budget_kib"
    printf '  - With SMT oversubscription, halve L2_KIB (or use shared budget ~%s KiB) for the cliff estimate.\n' "$shared_base_budget_kib"
    printf '  - speedup = orig_wall / bit_wall (higher = bit version wins).\n'
    printf '  - Watch for speedup compression or bit/orig wall times converging after L2? flips to yes.\n'
    printf '  - For Xeon 6230: L2_KIB=1024 SEGMENT_KIB=256 OVERHEAD_KIB=32 ./bench-sweep-powers.sh\n'
fi
