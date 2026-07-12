#!/usr/bin/env python3
"""Compare program-runs.tsv timings against a baseline (default: primeSearch).

Ratio is subject_time / baseline_time:
  ×0.80  subject is faster (80% of baseline time)
  ×1.20  subject is slower (120% of baseline time)

Examples:
  ./compare-runs.py --program PrimeSum9WheelPhasing
  ./compare-runs.py --history --all-programs
  ./compare-runs.py --history --only PrimeSum8Presieve13 --only PrimeSum9WheelPhasing
  ./compare-runs.py --max-exp 34
  ./compare-runs.py --runs test/wheel-limit-sweep-bench40-v2/v9.tsv --program PrimeSum9WheelPhasing
"""

from __future__ import annotations

import argparse
import csv
import math
import sys
from pathlib import Path
from typing import Iterable


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_RUNS = SCRIPT_DIR / "test" / "baseline" / "program-runs.tsv"


def skip_comments(lines: Iterable[str]) -> Iterable[str]:
    for line in lines:
        if line.strip() and not line.startswith("#"):
            yield line


def load_run_rows(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []

    rows: list[dict[str, str]] = []
    with path.open(newline="") as handle:
        reader = csv.DictReader(skip_comments(handle), delimiter="\t")
        for row in reader:
            if row.get("program") and row.get("end"):
                rows.append(row)
    return rows


def latest_runs_by_program_endpoint(
    rows: Iterable[dict[str, str]],
) -> dict[tuple[str, int], dict[str, str]]:
    latest: dict[tuple[str, int], dict[str, str]] = {}
    for row in rows:
        try:
            key = (row["program"], int(row["end"]))
        except (KeyError, ValueError):
            continue
        current = latest.get(key)
        if current is None or row.get("captured_at", "") >= current.get("captured_at", ""):
            latest[key] = row
    return latest


def newest_and_previous_successful_runs(
    rows: Iterable[dict[str, str]],
) -> tuple[
    dict[tuple[str, int], dict[str, str]],
    dict[tuple[str, int], dict[str, str]],
]:
    grouped: dict[tuple[str, int], list[dict[str, str]]] = {}
    for row in rows:
        if row.get("status") != "ok":
            continue
        try:
            key = (row["program"], int(row["end"]))
        except (KeyError, ValueError):
            continue
        grouped.setdefault(key, []).append(row)

    newest: dict[tuple[str, int], dict[str, str]] = {}
    previous: dict[tuple[str, int], dict[str, str]] = {}
    for key, candidates in grouped.items():
        candidates.sort(key=lambda row: row.get("captured_at", ""), reverse=True)
        if candidates:
            newest[key] = candidates[0]
        if len(candidates) > 1:
            previous[key] = candidates[1]
    return newest, previous


def parse_seconds(raw: str | None) -> float | None:
    if raw in (None, "", "-"):
        return None
    try:
        value = float(raw)
    except ValueError:
        return None
    if math.isnan(value) or value < 0:
        return None
    return value


def format_seconds(value: float | None) -> str:
    if value is None:
        return "-"
    return f"{value:.6f}"


def format_ratio(ratio: float | None) -> str:
    if ratio is None or math.isnan(ratio):
        return "-"
    return f"×{ratio:.2f}"


def parse_nonnegative_int(text: str) -> int:
    value = int(text)
    if value < 0:
        raise argparse.ArgumentTypeError("value must be nonnegative")
    return value


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runs", type=Path, default=DEFAULT_RUNS, help="program-runs.tsv path")
    parser.add_argument("--baseline", default="primeSearch", help="baseline program name")
    parser.add_argument(
        "--history",
        action="store_true",
        help="compare each program's newest successful rows with its previous successful rows",
    )
    parser.add_argument("--program", default=None, help="single program to compare")
    parser.add_argument(
        "--all-programs",
        action="store_true",
        help="compare every program (excluding the baseline in program-comparison mode)",
    )
    parser.add_argument("--only", action="append", default=[], help="filter to these program names")
    parser.add_argument("--metric", choices=["median", "best"], default="median")
    parser.add_argument("--max-exp", type=parse_nonnegative_int, default=None)
    parser.add_argument("--min-end", type=parse_nonnegative_int, default=None)
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    metric = "median_s" if args.metric == "median" else "best_s"
    runs_path = args.runs.resolve()

    rows = load_run_rows(runs_path)
    if not rows:
        print(f"No run rows found in {runs_path}.", file=sys.stderr)
        return 1

    if args.history:
        indexed, previous_indexed = newest_and_previous_successful_runs(rows)
    else:
        indexed = latest_runs_by_program_endpoint(rows)
        previous_indexed = {}
    baseline_name = args.baseline
    baseline_rows = {end: row for (program, end), row in indexed.items() if program == baseline_name}
    if not args.history and not baseline_rows:
        print(f"Baseline program {baseline_name!r} not found in {runs_path}.", file=sys.stderr)
        return 1

    if args.program:
        program_names = [args.program]
    elif args.all_programs:
        program_names = sorted(
            {
                program
                for (program, _end) in indexed
                if args.history or program != baseline_name
            }
        )
    else:
        program_names = sorted(
            {program for (program, _end) in indexed if program.startswith("PrimeSum")}
        )

    if args.only:
        wanted = set(args.only)
        program_names = [name for name in program_names if name in wanted]

    if not program_names:
        print("No programs selected for comparison.", file=sys.stderr)
        return 1

    max_end = (1 << args.max_exp) if args.max_exp is not None else None
    min_end = args.min_end
    exit_code = 0

    for program_name in program_names:
        subject_rows = {
            end: row for (program, end), row in indexed.items() if program == program_name
        }
        if args.history:
            comparison_name = f"previous {program_name} capture"
            comparison_rows = {
                end: row
                for (program, end), row in previous_indexed.items()
                if program == program_name
            }
        else:
            comparison_name = baseline_name
            comparison_rows = baseline_rows

        common_ends = sorted(set(subject_rows) & set(comparison_rows))
        if min_end is not None:
            common_ends = [end for end in common_ends if end >= min_end]
        if max_end is not None:
            common_ends = [end for end in common_ends if end <= max_end]

        if not common_ends:
            print(f"\n{program_name}: no shared endpoints with {comparison_name}.")
            continue

        print()
        print(f"program:  {program_name}")
        print(f"compare:  {comparison_name}")
        print(f"metric:   {metric}")
        print(f"source:   {runs_path}")
        if args.history:
            newest_times = [subject_rows[end].get("captured_at", "") for end in common_ends]
            previous_times = [comparison_rows[end].get("captured_at", "") for end in common_ends]
            print(f"newest:   {min(newest_times)} .. {max(newest_times)}")
            print(f"previous: {min(previous_times)} .. {max(previous_times)}")
        print()
        subject_heading = "new_s" if args.history else "time_s"
        comparison_heading = "old_s" if args.history else "base_s"
        print(
            f"{'label':<22} {'end':>14} {subject_heading:>12} "
            f"{comparison_heading:>12} {'ratio':>8}"
        )
        print("-" * 74)

        ratios: list[float] = []
        for end in common_ends:
            subject_row = subject_rows[end]
            baseline_row = comparison_rows[end]
            subject_time = parse_seconds(subject_row.get(metric))
            baseline_time = parse_seconds(baseline_row.get(metric))

            label = subject_row.get("label") or baseline_row.get("label") or "-"
            ratio: float | None = None
            if (
                subject_time is not None
                and baseline_time is not None
                and baseline_time > 0
                and subject_row.get("status") == "ok"
                and baseline_row.get("status") == "ok"
            ):
                ratio = subject_time / baseline_time
                ratios.append(ratio)
            else:
                exit_code = 1

            print(
                f"{label:<22} {end:>14} "
                f"{format_seconds(subject_time):>12} "
                f"{format_seconds(baseline_time):>12} "
                f"{format_ratio(ratio):>8}"
            )

        if ratios:
            geo_mean = math.exp(sum(math.log(value) for value in ratios) / len(ratios))
            print("-" * 74)
            print(
                f"{'geomean ratio':<22} {'':>14} {'':>12} {'':>12} "
                f"{format_ratio(geo_mean):>8}  ({len(ratios)} endpoints)"
            )

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
