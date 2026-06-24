#!/usr/bin/env bash
# Bench a target primeSearch binary against frozen baseline times.
# Target: 5 runs, mean of the 3 fastest wall times.
# Baselines:
#   bench-baseline-powers.txt  — 2^exp timing + sum checks
#   bench-baseline-edges.txt   — edge endpoints (2^exp±1, segment bounds, small ends)
#     Expected sums captured from ai/build/primeSearchPresieve19RangeCursorsFastCross.exe
#
# Usage:
#   ./bench-sweep.sh
#   ./bench-sweep.sh 24 42
#   ./bench-sweep.sh 24 42 --csv
#   TARGET=./mine/build/PrimeSum6RollingCursors.exe ./bench-sweep.sh 24 35
#   RUNS=5 TOP_N=3 ./bench-sweep.sh
#   SKIP_EDGES=1 ./bench-sweep.sh          # powers only
#
# Requires: bash, make, gcc, python3

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR"
# shellcheck source=bench-endpoints.sh
source "$SCRIPT_DIR/bench-endpoints.sh"

BUILDDIR=${BUILDDIR:-build}
BASELINE=${BASELINE:-bench-baseline-powers.txt}
EDGES_BASELINE=${EDGES_BASELINE:-bench-baseline-edges.txt}
TARGET=${TARGET:-./${BUILDDIR}/primeSearchPresieve19RangeCursorsFastCross.exe}
SKIP_EDGES=${SKIP_EDGES:-0}

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

if [[ "$SKIP_EDGES" -eq 0 && ! -f "$EDGES_BASELINE" ]]; then
    echo "Missing $EDGES_BASELINE — run ./bench-capture-baseline.sh (same exp range) first." >&2
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
elif [[ "$TARGET" == *"/build/"* ]]; then
    rel=${TARGET#./}
    submake_dir=${rel%%/build/*}
    make_target=${rel#"$submake_dir/"}
    echo "Building target (${submake_dir}/${make_target})..." >&2
    make -C "$submake_dir" "$make_target"
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

lookup_edge_baseline() {
    local end=$1
    python3 - "$EDGES_BASELINE" "$end" <<'PY'
import csv, sys

path, end = sys.argv[1], sys.argv[2]
with open(path, newline="") as f:
    lines = [line for line in f if not line.startswith("#") and line.strip()]
reader = csv.DictReader(lines, delimiter="\t")
for row in reader:
    if row["end"] == end:
        print(row.get("expected_sum", ""))
        break
else:
    print("")
PY
}

edge_mismatches=0
power_mismatches=0

if [[ $CSV -eq 1 ]]; then
    echo "case,label,end,target_wall,target_sum,orig_wall,opt_wall,sp_opt,sp_target,sum_ok,baseline"
else
    printf '\n'
    printf 'Target: %s\n' "$TARGET"
    printf 'Timing: %s runs, mean of best %s\n' "$RUNS" "$TOP_N"
    printf 'Baselines: %s\n' "$BASELINE"
    if [[ "$SKIP_EDGES" -eq 0 ]]; then
        printf 'Edge baselines: %s\n' "$EDGES_BASELINE"
    fi
    printf '%-4s %-14s %-12s %-12s %-12s %-7s %-7s %-3s %s\n' \
        'exp' 'end' 'target_s' 'orig_s' 'opt_s' 'sp_opt' 'sp_tgt' 'ok' 'notes'
    printf '%s\n' '---- -------------- ------------ ------------ ------------ ------- ------- --- -----'
fi

for exp in $(bench_list_power_exps "$EXP_MIN" "$EXP_MAX"); do
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
        power_mismatches=$((power_mismatches + 1))
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
        echo "power,$exp,$end,$target_wall,$target_sum,$orig_wall,$opt_wall,$speedup_opt,$speedup_tgt,$ok,$has_baseline"
    else
        printf '%-4s %-14s %-12s %-12s %-12s %-7s %-7s %-3s %s\n' \
            "$exp" "$end" "$target_wall" "$orig_wall" "$opt_wall" \
            "$speedup_opt" "$speedup_tgt" "$ok" "$note"
    fi
done

if [[ "$SKIP_EDGES" -eq 0 ]]; then
    if [[ $CSV -eq 0 ]]; then
        printf '\n'
        printf 'Edge cases (2^exp±1, segment boundaries, small ends):\n'
        printf '%-16s %-14s %-12s %-3s %s\n' 'label' 'end' 'target_s' 'ok' 'notes'
        printf '%s\n' '---------------- -------------- ------------ --- -----'
    fi

    while IFS=$'\t' read -r label end; do
        read -r expected_sum <<< "$(lookup_edge_baseline "$end")"
        read -r target_wall target_sum <<< "$(run_timed_avg_top "$TARGET" "$end")"

        ok=yes
        note=""
        if [[ -z "$expected_sum" || "$expected_sum" == "-" ]]; then
            ok="?"
            note="no baseline row"
        elif [[ "$expected_sum" != "$target_sum" ]]; then
            ok=MISMATCH
            edge_mismatches=$((edge_mismatches + 1))
            note="expected $expected_sum"
        fi

        if [[ $CSV -eq 1 ]]; then
            echo "edge,$label,$end,$target_wall,$target_sum,,,,$ok,$expected_sum"
        else
            printf '%-16s %-14s %-12s %-3s %s\n' \
                "$label" "$end" "$target_wall" "$ok" "$note"
        fi
    done < <(bench_list_edge_endpoints_unique "$EXP_MIN" "$EXP_MAX")
fi

if [[ $CSV -eq 0 ]]; then
    printf '\n'
    printf 'Notes:\n'
    printf '  - sp_opt = orig/opt; sp_tgt = orig/target (higher = faster).\n'
    printf '  - Edge sums checked against %s (regenerate with ./bench-capture-baseline.sh).\n' "$EDGES_BASELINE"
    printf '  - Endpoint list: bench-endpoints.sh (BLOCK_SIZE=%s for segment edges).\n' "${BLOCK_SIZE:-524288}"
    if [[ "$SKIP_EDGES" -eq 0 ]]; then
        total_mismatches=$((power_mismatches + edge_mismatches))
        if [[ "$total_mismatches" -gt 0 ]]; then
            printf '  - SUM MISMATCH: %d power, %d edge.\n' "$power_mismatches" "$edge_mismatches"
            exit 1
        fi
        printf '  - All sum checks passed (%d power, edge cases included).\n' \
            "$((EXP_MAX - EXP_MIN + 1))"
    fi
fi
