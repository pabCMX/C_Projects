# PrimeSum Baselines

Generated files live here:

- `expected-sums.tsv` stores known-good sums by endpoint.
- `program-runs.tsv` stores benchmark rows by program and endpoint.

Typical flow:

```sh
./prime-suite.sh capture-sums ../ai/build/primeSearchPresieve19RangeCursorsFastCross.exe
./prime-suite.sh test ../mine/build/PrimeSum6RollingCursors.exe
./prime-suite.sh bench ../mine/build/PrimeSum6RollingCursors.exe
```

`bench` defaults to `2^40` and a two-hour time budget. Tiny cases are run repeatedly so
program setup cost is visible instead of being hidden by one noisy subprocess launch.
