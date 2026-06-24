#!/usr/bin/env bash
# Shared endpoint lists for bench-capture-baseline.sh and bench-sweep.sh.
# Source this file; do not execute directly.

# Print exponents for the 2^exp power sweep (one integer per line).
bench_list_power_exps() {
    local exp_min=$1 exp_max=$2
    seq "$exp_min" "$exp_max"
}

# Print edge-case rows as: label<TAB>end
# Includes 2^exp±1 at exp_min, midpoint, and exp_max, plus small and segment-boundary ends.
bench_list_edge_endpoints() {
    local exp_min=$1 exp_max=$2
    local block_size=${BLOCK_SIZE:-524288} # 1<<19; matches PrimeSum default segment
    local mid=$(((exp_min + exp_max) / 2))
    local exp base seg_end

    printf 'min\t2\n'
    printf 'small_3\t3\n'
    printf 'small_4\t4\n'
    printf 'small_5\t5\n'
    printf 'small_100\t100\n'
    printf 'small_1000\t1000\n'

    for exp in "$exp_min" "$mid" "$exp_max"; do
        base=$((1 << exp))
        printf '2^%d-1\t%d\n' "$exp" $((base - 1))
        printf '2^%d+1\t%d\n' "$exp" $((base + 1))
    done

    # Odds-only segment high watermark: high = 2 * (i+1) * block_size + 1
    for seg in 1 2; do
        seg_end=$((2 * seg * block_size + 1))
        printf 'seg%d_end-1\t%d\n' "$seg" $((seg_end - 1))
        printf 'seg%d_end\t%d\n' "$seg" "$seg_end"
        printf 'seg%d_end+1\t%d\n' "$seg" $((seg_end + 1))
    done
}

# Read label<TAB>end rows from bench_list_edge_endpoints, dedupe by end (first label wins).
bench_list_edge_endpoints_unique() {
    local exp_min=$1 exp_max=$2
    declare -A seen=()

    while IFS=$'\t' read -r label end; do
        [[ -z "$end" ]] && continue
        if [[ -n "${seen[$end]+x}" ]]; then
            continue
        fi
        seen[$end]=1
        printf '%s\t%s\n' "$label" "$end"
    done < <(bench_list_edge_endpoints "$exp_min" "$exp_max")
}
