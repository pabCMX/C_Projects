# PrimeSum Test Suite

One wrapper drives the suite:

```sh
./prime-suite.sh test ../mine/build/PrimeSum6RollingCursors.exe
./prime-suite.sh bench ../mine/build/PrimeSum6RollingCursors.exe
./prime-suite.sh capture-all
```

## Modes

- `test` checks one executable against the standard endpoint subset through `2^34` by default.
  It runs each endpoint 3 times by default, with no warmup or adaptive timing loop.
- `bench` records comparison data for the full endpoint set through `2^40` unless the two-hour time budget runs out.
- `capture-sums` writes `baseline/expected-sums.tsv` from a trusted reference executable.
- `capture-all` benchmarks all discovered `mine/*.c` and `ai/*.c` programs.
- `latency` measures Python subprocess overhead with repeated `/usr/bin/true` launches.

## Typical Baseline Flow

```sh
./prime-suite.sh capture-sums ../ai/build/primeSearchPresieve19RangeCursorsFastCross.exe
./prime-suite.sh capture-all --time-budget 7200
```

Use `./prime-suite.sh test <exec> --runs 5` if you want five correctness runs per endpoint.
The heavier repeated timing and `/usr/bin/true` latency measurement are reserved for `bench`.

The test endpoint set includes small values, powers, `2^x ± 1`, non-powers,
segment edges, and prime segment-high endpoints such as `7340033`. The bench
endpoint set is a superset of the test set.

Old `bench-*` scripts and captures are archived under `archive/`.
