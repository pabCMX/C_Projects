# SecPlus Study Drill

Terminal cram tool for Security+ ports, controls, threat vectors, and Zero Trust.

```bash
python3 drill.py
```

- Pick a bank with `1`–`5`.
- Ports bank is fast drills only (`SSH port?` → `22`, `SSH TCP/UDP/Both?`, `Port 22?`). Full name and use show as a one-line reminder after you answer, not as questions.
- Type answers. Case does not matter. Control prompts are one word (`technical` or `detective`, not both).
- `q` during a bank returns to the menu. The process keeps running.
- `q` at the menu exits.
- A bank reshuffles when you exhaust it. Keep going until you hit `q`.
- Each stack keeps a last-20 batting average in `stats.json`. Compare it to lifetime: if BA is rising and above lifetime, you are improving. Delete `stats.json` to reset.
