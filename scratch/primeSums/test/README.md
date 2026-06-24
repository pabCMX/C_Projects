# PrimeSum Test Suite

One wrapper drives the suite:

```sh
./prime-suite.sh test ../mine/build/PrimeSum6RollingCursors.exe
./prime-suite.sh bench ../mine/build/PrimeSum6RollingCursors.exe
./prime-suite.sh capture-all
```

## Modes

- `test` checks one executable against the standard endpoint set through `2^34` by default.
- `bench` records comparison data through `2^40` unless the two-hour time budget runs out.
- `capture-sums` writes `baseline/expected-sums.tsv` from a trusted reference executable.
- `capture-all` benchmarks all discovered `mine/*.c` and `ai/*.c` programs.
- `latency` measures Python subprocess overhead with repeated `/usr/bin/true` launches.

## Typical Baseline Flow

```sh
./prime-suite.sh capture-sums ../ai/build/primeSearchPresieve19RangeCursorsFastCross.exe
./prime-suite.sh capture-all --time-budget 7200
```

Tiny inputs are timed with repeated fresh subprocess launches. That intentionally includes
program startup and setup drag. The `/usr/bin/true` latency measurement is reported beside
benchmarks for context, but it is not subtracted from program timings.

Old `bench-*` scripts and captures are archived under `archive/`.
