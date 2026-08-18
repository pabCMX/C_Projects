# SecPlus Study Drill

Terminal cram tool for Security+ ports, controls, threat vectors, and Zero Trust.

```bash
python3 drill.py
```

- Pick a bank with `1`–`5`.
- Ports are core Security+ drills (`SSH port?` → `22`, transport, and reverse questions). A broad protocol answer such as `FTP` is accepted for port 20 or 21; the explanation still teaches data versus control. Low-yield reference entries marked non-core are skipped.
- Type answers. Case does not matter. Only listed aliases count, so unrelated words around an answer are not silently accepted.
- Control **category** describes how a control is implemented: `technical`, `managerial`, `operational`, or `physical`. Control **type** describes what it does: `preventive`, `detective`, `corrective`, `deterrent`, `compensating`, or `directive`.
- Acronyms are expanded in prompts or explanations. Each answer includes a short reason, not just a label.
- `q` during a bank returns to the menu. The process keeps running.
- `s` skips a question without changing its score or weakness.
- `q` at the menu exits.
- Wrong answers raise the weight of the whole study item, including its forward and reverse forms. The item returns after several other questions in the same session and stays more likely in future sessions. Correct answers reduce that weight gradually.
- Each stack keeps a last-20 batting average and lifetime average in `stats.json`. Existing aggregate history is preserved when item-level weakness tracking is added. Delete `stats.json` only if you intentionally want to reset all history.
- Validate bank structure without starting a drill:

```bash
python3 drill.py --validate
```
