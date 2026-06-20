#!/usr/bin/env bash
# Bench a target primeSearch binary against frozen baseline times.
# Target: 5 runs, mean of the 3 fastest wall times.
# Baselines: bench-baseline-powers.txt from bench-capture-baseline.sh.
#
# Usage:
#   ./bench-sweep.sh
#   ./bench-sweep.sh 24 42
#   ./bench-sweep.sh 24 42 --csv
#   TARGET=./build/primeSearchPresieve19RangeCursorsFastCross.exe ./bench-sweep.sh
#   RUNS=5 TOP_N=3 ./bench-sweep.sh
#
# Requires: bash, make, gcc, python3

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"

BUILDDIR=${BUILDDIR:-build}
BASELINE=${BASELINE:-bench-baseline-powers.txt}
TARGET=${TARGET:-./${BUILDDIR}/primeSearchPresieve19RangeCursorsFastCross.exe}

RUNS=${RUNS:-5}
TOP_N=${TOP_N:-3}

EXP_MIN=${1:-24}
EXP_MAX=${2:-42}
CSV=0
if [[ "${3:-}" == "--csv" ]] || [[ "${2:-}" == "--csv" ]]; then
    CSV=1
    [[ "${2:-}" == "--csv" ]] && EXP_MAX=${EXP_MIN}
fi

if [[ ! -f "$BASELINE" ]]; then
    echo "Missing $BASELINE — run ./bench-capture-baseline.sh first." >&2
    exit 1
fi

if [[ "$TOP_N" -gt "$RUNS" ]]; then
    echo "TOP_N ($TOP_N) must be <= RUNS ($RUNS)." >&2
    exit 1
fi

if [[ "$TARGET" == "${BUILDDIR}/"* || "$TARGET" == "./${BUILDDIR}/"* ]]; then
    make_target=${TARGET#./}
    echo "Building target (${make_target})..." >&2
    make "$make_target"
elif [[ ! -x "$TARGET" ]]; then
    echo "Target not executable: $TARGET" >&2
    exit 1
fi

run_timed_avg_top() {
    local bin=$1 end=$2
    python3 - "$bin" "$end" "$RUNS" "$TOP_N" <<'PY'
import subprocess, sys, time

bin_path, end, runs, top_n = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
times = []
sum_out = ""
for _ in range(runs):
    t0 = time.perf_counter()
    r = subprocess.run(
        [bin_path, str(end), "--sum-only"],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=True,
    )
    times.append(time.perf_counter() - t0)
    sum_out = r.stdout.decode().strip()

best = sorted(times)[:top_n]
avg = sum(best) / len(best)
print(f"{avg:.6f}\t{sum_out}")
PY
}

lookup_baseline() {
    local exp=$1
    python3 - "$BASELINE" "$exp" <<'PY'
import csv, sys

path, exp = sys.argv[1], sys.argv[2]
with open(path, newline="") as f:
    lines = [line for line in f if not line.startswith("#") and line.strip()]
reader = csv.DictReader(lines, delimiter="\t")
for row in reader:
    if row["exp"] == exp:
        print(
            row.get("orig_wall", ""),
            row.get("orig_sum", ""),
            row.get("opt_wall", ""),
            row.get("opt_sum", ""),
        )
        break
else:
    print("", "", "", "")
PY
}

if [[ $CSV -eq 1 ]]; then
    echo "exp,end,target_wall,target_sum,orig_wall,opt_wall,sp_opt,sp_target,sum_ok,baseline"
else
    target_name=$(basename "$TARGET")
    printf '\n'
    printf 'Target: %s\n' "$TARGET"
    printf 'Timing: %s runs, mean of best %s\n' "$RUNS" "$TOP_N"
    printf 'Baselines: %s\n' "$BASELINE"
    printf '%-4s %-14s %-8s %-8s %-8s %-7s %-7s %-3s %s\n' \
        'exp' 'end' 'target_s' 'orig_s' 'opt_s' 'sp_opt' 'sp_tgt' 'ok' 'notes'
    printf '%s\n' '---- -------------- -------- -------- -------- ------- ------- --- -----'
fi

for exp in $(seq "$EXP_MIN" "$EXP_MAX"); do
    end=$((1 << exp))

    read -r orig_wall orig_sum opt_wall opt_sum <<< "$(lookup_baseline "$exp")"
    has_baseline=yes
    if [[ -z "$orig_wall" || "$orig_wall" == "-" ]]; then
        has_baseline=no
    fi

    read -r target_wall target_sum <<< "$(run_timed_avg_top "$TARGET" "$end")"

    ok=yes
    if [[ "$has_baseline" == yes && "$orig_sum" != "-" && "$orig_sum" != "$target_sum" ]]; then
        ok=MISMATCH
    fi

    read -r speedup_opt speedup_tgt <<< "$(python3 - "$orig_wall" "$opt_wall" "$target_wall" <<'PY'
import sys

def ratio(num, den):
    if not num or not den or num == "-" or den == "-":
        return ""
    num, den = float(num), float(den)
    return f"{num / den:.2f}" if den > 0 else "inf"

orig, opt, target = sys.argv[1:4]
print(ratio(orig, opt), ratio(orig, target))
PY
)"

    note=""
    if [[ "$has_baseline" == no ]]; then
        note="no baseline row"
    fi

    if [[ $CSV -eq 1 ]]; then
        echo "$exp,$end,$target_wall,$target_sum,$orig_wall,$opt_wall,$speedup_opt,$speedup_tgt,$ok,$has_baseline"
    else
        printf '%-4s %-14s %-8s %-8s %-8s %-7s %-7s %-3s %s\n' \
            "$exp" "$end" "$target_wall" "$orig_wall" "$opt_wall" \
            "$speedup_opt" "$speedup_tgt" "$ok" "$note"
    fi
done

if [[ $CSV -eq 0 ]]; then
    printf '\n'
    printf 'Notes:\n'
    printf '  - sp_opt = orig/opt; sp_tgt = orig/target (higher = faster).\n'
    printf '  - Regenerate baselines: ./bench-capture-baseline.sh\n'
fi
