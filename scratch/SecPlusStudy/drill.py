#!/usr/bin/env python3
"""Security+ cram drill. Run: python3 drill.py"""

from __future__ import annotations

import json
import random
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BANKS_DIR = ROOT / "banks"
STATS_PATH = ROOT / "stats.json"
WINDOW = 20

MENU = [
    ("1", "ports.json", "Ports and protocols"),
    ("2", "controls.json", "Control category vs type"),
    ("3", "vectors.json", "Threat vectors"),
    ("4", "zero_trust.json", "Zero Trust"),
    ("5", "all", "All banks mixed"),
]

BOTH_ANSWERS = (
    "both",
    "tcp/udp",
    "udp/tcp",
    "tcp and udp",
    "udp and tcp",
    "tcp udp",
    "udp tcp",
    "tcp & udp",
    "udp & tcp",
)

FILLER = re.compile(r"\b(the|a|an|and|or|of|to|for|with)\b")
SEP = re.compile(r"[-–—_/+,]+")
SPACE = re.compile(r"\s+")


def normalize(text: str) -> str:
    text = text.strip().lower()
    text = text.replace("&", " and ")
    text = text.replace(".", " ")
    text = SEP.sub(" ", text)
    text = FILLER.sub(" ", text)
    return SPACE.sub(" ", text).strip()


def matches(user: str, answers: list[str]) -> bool:
    got = normalize(user)
    if not got:
        return False
    accepted = {normalize(a) for a in answers if a}
    if got in accepted:
        return True
    # Allow "161 udp" when the expected answer is "161".
    first = got.split(" ", 1)[0]
    if first in accepted and first.isdigit():
        return True
    got_set = set(got.split())
    for item in accepted:
        item_set = set(item.split())
        if len(item_set) >= 3 and got_set == item_set:
            return True
        # "used for remote access" should hit a "remote access" alias.
        if len(item) >= 5 and item in got:
            return True
    return False


def load_json(name: str) -> dict:
    path = BANKS_DIR / name
    with path.open(encoding="utf-8") as fh:
        return json.load(fh)


def extra_line(item: dict) -> str:
    name = item["name"]
    full = item.get("full_name") or name
    use = (item.get("use") or "").strip()
    if not use:
        return full
    if normalize(full) == normalize(name):
        return f"{name} — {use}"
    return f"{full} ({name}) — {use}"


def expand_ports(bank: dict) -> list[dict]:
    items = bank["items"]
    by_port: dict[str, list[dict]] = {}
    for item in items:
        port = item.get("port")
        if port:
            by_port.setdefault(str(port), []).append(item)

    questions = []
    for item in items:
        extra = extra_line(item)
        name = item["name"]
        port = item.get("port")
        transport = item.get("transport")

        if not port:
            clue = item.get("clue")
            if clue and transport:
                t_answers = list(BOTH_ANSWERS) if transport == "both" else [transport]
                questions.append(
                    {
                        "prompt": f"{clue} — TCP/UDP/Both?",
                        "answers": t_answers,
                        "reveal": str(transport).upper(),
                        "extra": extra,
                    }
                )
            continue

        port_s = str(port)
        t_label = "TCP/UDP" if transport == "both" else str(transport).upper()
        questions.append(
            {
                "prompt": f"{name} port?",
                "answers": [port_s, *item.get("port_answers", [])],
                "reveal": f"{port_s} {t_label}",
                "extra": extra,
            }
        )
        if transport == "both":
            t_answers = list(BOTH_ANSWERS)
            t_reveal = "TCP and UDP"
        else:
            t_answers = [transport, *item.get("transport_answers", [])]
            t_reveal = str(transport).upper()
        questions.append(
            {
                "prompt": f"{name} TCP/UDP/Both?",
                "answers": t_answers,
                "reveal": t_reveal,
                "extra": extra,
            }
        )

    seen_ports = set()
    for item in items:
        port = item.get("port")
        if not port or port in seen_ports:
            continue
        seen_ports.add(port)
        group = by_port[str(port)]
        names = []
        for entry in group:
            names.append(entry["name"])
            names.extend(entry.get("name_answers", []))
        questions.append(
            {
                "prompt": f"Port {port}?",
                "answers": names,
                "reveal": " / ".join(entry["name"] for entry in group),
                "extra": "  |  ".join(extra_line(entry) for entry in group),
            }
        )
    return questions


def load_questions(filename: str) -> tuple[str, list[dict]]:
    bank = load_json(filename)
    title = bank["title"]
    if bank.get("kind") == "ports":
        return title, expand_ports(bank)
    return title, bank["questions"]


def load_all() -> tuple[str, list[dict]]:
    questions = []
    for _key, filename, _title in MENU:
        if filename == "all":
            continue
        _, qs = load_questions(filename)
        questions.extend(qs)
    return "All banks mixed", questions


def empty_bank() -> dict:
    return {"hits": 0, "at_bats": 0, "recent": []}


def load_stats() -> dict:
    if not STATS_PATH.exists():
        return {"window": WINDOW, "banks": {}}
    try:
        with STATS_PATH.open(encoding="utf-8") as fh:
            data = json.load(fh)
    except (OSError, json.JSONDecodeError):
        return {"window": WINDOW, "banks": {}}
    data.setdefault("window", WINDOW)
    data.setdefault("banks", {})
    return data


def save_stats(stats: dict) -> None:
    tmp = STATS_PATH.with_suffix(".json.tmp")
    tmp.write_text(json.dumps(stats, indent=2) + "\n", encoding="utf-8")
    tmp.replace(STATS_PATH)


def bank_stats(stats: dict, key: str) -> dict:
    banks = stats.setdefault("banks", {})
    slot = banks.setdefault(key, empty_bank())
    slot.setdefault("hits", 0)
    slot.setdefault("at_bats", 0)
    slot.setdefault("recent", [])
    return slot


def avg_text(hits: int, n: int) -> str:
    if n == 0:
        return "---"
    if hits == n:
        return "1.000"
    return f".{round(1000 * hits / n):03d}"


def ba_text(slot: dict, window: int = WINDOW) -> str:
    recent = slot.get("recent", [])[-window:]
    n = len(recent)
    hits = sum(recent)
    return f"BA {avg_text(hits, n)} ({hits}/{n})"


def lifetime_text(slot: dict) -> str:
    at_bats = slot.get("at_bats", 0)
    hits = slot.get("hits", 0)
    return f"lifetime {avg_text(hits, at_bats)} ({hits}/{at_bats})"


def record_result(stats: dict, key: str, correct: bool) -> dict:
    slot = bank_stats(stats, key)
    slot["at_bats"] += 1
    if correct:
        slot["hits"] += 1
    recent = slot["recent"]
    recent.append(1 if correct else 0)
    slot["recent"] = recent[-WINDOW:]
    save_stats(stats)
    return slot


def color(text: str, code: str) -> str:
    if not sys.stdout.isatty():
        return text
    return f"\033[{code}m{text}\033[0m"


def ask(prompt: str) -> str:
    try:
        return input(prompt)
    except EOFError:
        print()
        return "q"
    except KeyboardInterrupt:
        print()
        return "q"


def drill(title: str, questions: list[dict], stats: dict, key: str) -> None:
    if not questions:
        print("No questions in that bank.\n")
        return

    slot = bank_stats(stats, key)
    print()
    print(f"=== {title} ({len(questions)} questions) ===")
    print("q = back to menu (app stays running). Empty answer counts as a miss.")
    print(f"{ba_text(slot)}  {lifetime_text(slot)}")
    print()

    correct = 0
    total = 0
    missed: list[tuple[str, str]] = []
    queue = questions[:]
    random.shuffle(queue)

    while True:
        if not queue:
            queue = questions[:]
            random.shuffle(queue)
            print(color("— reshuffled —", "90"))

        q = queue.pop()
        raw = ask(f"{q['prompt']} ")
        token = raw.strip().lower()
        if token in {"q", "quit", "menu", "m"}:
            break

        total += 1
        hit = matches(raw, q["answers"])
        slot = record_result(stats, key, hit)
        ba = ba_text(slot)
        extra = q.get("extra")
        if hit:
            correct += 1
            print(color("Correct!", "32"), f"  [{correct}/{total}]  {ba}")
        else:
            missed.append((q["prompt"], q["reveal"]))
            print(color("Wrong!", "31"), q["reveal"], f"  [{correct}/{total}]  {ba}")
        if extra:
            print(color(f"  {extra}", "90"))

    print()
    if total == 0:
        print(f"Left {title} without answering.\n")
        return
    pct = round(100 * correct / total)
    print(f"--- {title} ---")
    print(f"session {correct}/{total} ({pct}%)")
    print(f"{ba_text(slot)}  {lifetime_text(slot)}")
    if missed:
        print("Missed:")
        seen = set()
        unique = []
        for prompt, reveal in missed:
            if prompt in seen:
                continue
            seen.add(prompt)
            unique.append((prompt, reveal))
        for prompt, reveal in unique:
            print(f"  {prompt}  →  {reveal}")
    print()


def menu(stats: dict) -> None:
    print("SecPlus Study Drill")
    print("q on a bank returns here. q here exits.")
    print()
    for key, filename, title in MENU:
        slot = bank_stats(stats, filename)
        print(f"  {key}) {title:<28}  {ba_text(slot)}")
    print("  q) Quit")
    print()


def main() -> int:
    stats = load_stats()
    while True:
        menu(stats)
        choice = ask("Bank> ").strip().lower()
        if choice in {"q", "quit", "exit"}:
            return 0
        picked = next((item for item in MENU if item[0] == choice), None)
        if picked is None:
            print("Pick 1-5, or q to quit.\n")
            continue
        _key, filename, _title = picked
        if filename == "all":
            title, questions = load_all()
        else:
            title, questions = load_questions(filename)
        drill(title, questions, stats, filename)


if __name__ == "__main__":
    raise SystemExit(main())
