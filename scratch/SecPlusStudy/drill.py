#!/usr/bin/env python3
"""Security+ cram drill. Run: python3 drill.py"""

from __future__ import annotations

import json
import random
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BANKS_DIR = ROOT / "banks"
STATS_PATH = ROOT / "stats.json"
WINDOW = 20
MAX_WEAKNESS = 10
MISS_PENALTY = 2
CORRECT_RECOVERY = 1
RETRY_GAP = 4

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
    """Match only explicit aliases, plus an optional transport after a port."""
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
    items = [item for item in bank["items"] if item.get("core", True)]
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
        item_id = f"ports:{port}" if port else f"ports:{item['id']}"

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
                        "item_id": item_id,
                        "variant_id": f"{item['id']}:transport",
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
                "item_id": item_id,
                "variant_id": f"{item['id']}:port",
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
                "item_id": item_id,
                "variant_id": f"{item['id']}:transport",
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
            names.extend(entry.get("aliases", []))
        questions.append(
            {
                "prompt": f"Port {port} — name one associated protocol?",
                "answers": names,
                "reveal": " / ".join(entry["name"] for entry in group),
                "extra": "  |  ".join(extra_line(entry) for entry in group),
                "item_id": f"ports:{port}",
                "variant_id": f"port-{port}:reverse",
            }
        )
    return questions


def expand_controls(bank: dict) -> list[dict]:
    questions = []
    for item in bank["items"]:
        display = item.get("display") or item["name"]
        item_id = f"controls:{item['id']}"
        rationale = item["rationale"]
        for axis in ("category", "type"):
            canonical = item[axis]
            alternatives = item.get(f"{axis}_alternatives", [])
            answers = [canonical, *alternatives]
            reveal = canonical
            if alternatives:
                reveal += f" ({' / '.join(alternatives)} also accepted)"
            questions.append(
                {
                    "prompt": f"{display} — {axis}?",
                    "answers": answers,
                    "reveal": reveal,
                    "extra": rationale,
                    "item_id": item_id,
                    "variant_id": f"{item['id']}:{axis}",
                }
            )
    return questions


def prepare_questions(bank_name: str, questions: list[dict]) -> list[dict]:
    prepared = []
    for index, question in enumerate(questions):
        q = dict(question)
        q.setdefault("item_id", f"{bank_name}:{q.get('id', index)}")
        q.setdefault("variant_id", f"{q['item_id']}:{q.get('id', index)}")
        q.setdefault("extra", q.get("explanation"))
        prepared.append(q)
    return prepared


def load_questions(filename: str) -> tuple[str, list[dict]]:
    bank = load_json(filename)
    title = bank["title"]
    if bank.get("kind") == "ports":
        return title, expand_ports(bank)
    if bank.get("kind") == "controls":
        return title, expand_controls(bank)
    return title, prepare_questions(filename, bank["questions"])


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
        return {"window": WINDOW, "banks": {}, "items": {}}
    try:
        with STATS_PATH.open(encoding="utf-8") as fh:
            data = json.load(fh)
    except (OSError, json.JSONDecodeError):
        return {"window": WINDOW, "banks": {}, "items": {}}
    data.setdefault("window", WINDOW)
    data.setdefault("banks", {})
    data.setdefault("items", {})
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


def item_stats(stats: dict, item_id: str) -> dict:
    items = stats.setdefault("items", {})
    slot = items.setdefault(
        item_id, {"weakness": 0, "attempts": 0, "correct": 0, "wrong": 0}
    )
    slot.setdefault("weakness", 0)
    slot.setdefault("attempts", 0)
    slot.setdefault("correct", 0)
    slot.setdefault("wrong", 0)
    return slot


def weak_item_count(stats: dict, questions: list[dict]) -> int:
    item_ids = {q["item_id"] for q in questions}
    return sum(
        1
        for item_id in item_ids
        if stats.get("items", {}).get(item_id, {}).get("weakness", 0) > 0
    )


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


def record_result(stats: dict, key: str, item_id: str, correct: bool) -> dict:
    slot = bank_stats(stats, key)
    slot["at_bats"] += 1
    if correct:
        slot["hits"] += 1
    recent = slot["recent"]
    recent.append(1 if correct else 0)
    slot["recent"] = recent[-WINDOW:]
    item = item_stats(stats, item_id)
    item["attempts"] += 1
    if correct:
        item["correct"] += 1
        item["weakness"] = max(0, item["weakness"] - CORRECT_RECOVERY)
    else:
        item["wrong"] += 1
        item["weakness"] = min(MAX_WEAKNESS, item["weakness"] + MISS_PENALTY)
    save_stats(stats)
    return slot


def group_questions(questions: list[dict]) -> dict[str, list[dict]]:
    groups: dict[str, list[dict]] = defaultdict(list)
    for question in questions:
        groups[question["item_id"]].append(question)
    return dict(groups)


def choose_item(
    groups: dict[str, list[dict]],
    stats: dict,
    recent_items: list[str],
    rng=random,
) -> str:
    item_ids = list(groups)
    candidates = [item_id for item_id in item_ids if item_id not in recent_items[-2:]]
    if not candidates:
        candidates = item_ids
    weights = [
        1 + 2 * item_stats(stats, item_id)["weakness"] for item_id in candidates
    ]
    return rng.choices(candidates, weights=weights, k=1)[0]


def choose_variant(
    variants: list[dict],
    variant_counts: dict[str, int],
    rng=random,
) -> dict:
    least_seen = min(variant_counts[q["variant_id"]] for q in variants)
    candidates = [
        q for q in variants if variant_counts[q["variant_id"]] == least_seen
    ]
    return rng.choice(candidates)


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
    groups = group_questions(questions)
    print()
    print(f"=== {title} ({len(groups)} items / {len(questions)} question forms) ===")
    print("q = menu, s = skip. Empty answer counts as a miss.")
    print(f"{ba_text(slot)}  {lifetime_text(slot)}")
    print(f"weak items: {weak_item_count(stats, questions)}")
    print()

    correct = 0
    total = 0
    missed: list[tuple[str, str]] = []
    recent_items: list[str] = []
    retries: list[tuple[int, str]] = []
    variant_counts: dict[str, int] = defaultdict(int)

    while True:
        ready = next(
            (
                (index, item_id)
                for index, (after, item_id) in enumerate(retries)
                if after <= total
            ),
            None,
        )
        if ready is not None:
            retry_index, selected_item = ready
            retries.pop(retry_index)
            reason = "missed recently"
        else:
            selected_item = choose_item(groups, stats, recent_items)
            weakness = item_stats(stats, selected_item)["weakness"]
            reason = "weak item" if weakness else "normal mix"

        q = choose_variant(groups[selected_item], variant_counts)
        variant_counts[q["variant_id"]] += 1
        recent_items.append(selected_item)
        recent_items = recent_items[-4:]
        print(color(f"[{reason}]", "90"))
        raw = ask(f"{q['prompt']} ")
        token = raw.strip().lower()
        if token in {"q", "quit", "menu", "m"}:
            break
        if token in {"s", "skip"}:
            print(color(f"Skipped. Answer: {q['reveal']}", "90"))
            if q.get("extra"):
                print(color(f"  {q['extra']}", "90"))
            if reason == "missed recently":
                retries.append((total + 1, q["item_id"]))
            continue

        total += 1
        hit = matches(raw, q["answers"])
        slot = record_result(stats, key, q["item_id"], hit)
        ba = ba_text(slot)
        extra = q.get("extra")
        if hit:
            correct += 1
            print(
                color("Correct!", "32"),
                f"→ {q['reveal']}  [{correct}/{total}]  {ba}",
            )
        else:
            missed.append((q["prompt"], q["reveal"]))
            if not any(item_id == q["item_id"] for _after, item_id in retries):
                retries.append((total + RETRY_GAP, q["item_id"]))
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
        if filename == "all":
            _loaded_title, questions = load_all()
        else:
            _loaded_title, questions = load_questions(filename)
        weak = weak_item_count(stats, questions)
        print(f"  {key}) {title:<28}  {ba_text(slot)}  weak {weak}")
    print("  q) Quit")
    print()


def validate_bank(filename: str, bank: dict) -> list[str]:
    errors: list[str] = []
    seen_ids: set[str] = set()
    kind = bank.get("kind")
    records = bank.get("items") if kind in {"ports", "controls"} else bank.get("questions")
    if not isinstance(records, list):
        return [f"{filename}: expected a list of items/questions"]

    categories = {"technical", "managerial", "operational", "physical"}
    control_types = {
        "preventive",
        "detective",
        "corrective",
        "deterrent",
        "compensating",
        "directive",
    }
    for index, record in enumerate(records):
        record_id = record.get("id")
        label = f"{filename}[{index}]"
        if not record_id:
            errors.append(f"{label}: missing stable id")
        elif record_id in seen_ids:
            errors.append(f"{label}: duplicate id {record_id!r}")
        else:
            seen_ids.add(record_id)

        if kind == "ports":
            transport = record.get("transport")
            if transport not in {"tcp", "udp", "both"}:
                errors.append(f"{label}: invalid transport {transport!r}")
            if not record.get("name"):
                errors.append(f"{label}: missing name")
            if record.get("port") is None and not record.get("clue"):
                errors.append(f"{label}: portless item needs a clue")
        elif kind == "controls":
            if record.get("category") not in categories:
                errors.append(f"{label}: invalid control category")
            if record.get("type") not in control_types:
                errors.append(f"{label}: invalid control type")
            if not record.get("rationale"):
                errors.append(f"{label}: missing rationale")
        else:
            if not record.get("prompt"):
                errors.append(f"{label}: missing prompt")
            if not record.get("answers"):
                errors.append(f"{label}: missing answers")
            if not record.get("reveal"):
                errors.append(f"{label}: missing canonical reveal")
            if not (record.get("explanation") or record.get("extra")):
                errors.append(f"{label}: missing explanation")

    try:
        if kind == "ports":
            expanded = expand_ports(bank)
        elif kind == "controls":
            expanded = expand_controls(bank)
        else:
            expanded = prepare_questions(filename, records)
    except (KeyError, TypeError, ValueError) as exc:
        errors.append(f"{filename}: cannot expand bank: {exc}")
        return errors

    seen_variants: set[str] = set()
    for question in expanded:
        variant_id = question["variant_id"]
        if variant_id in seen_variants:
            errors.append(f"{filename}: duplicate question variant {variant_id!r}")
        seen_variants.add(variant_id)
        if not question.get("answers"):
            errors.append(f"{filename}: {variant_id!r} has no answers")
    return errors


def validate_all_banks() -> list[str]:
    errors = []
    for _key, filename, _title in MENU:
        if filename == "all":
            continue
        try:
            bank = load_json(filename)
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"{filename}: {exc}")
            continue
        errors.extend(validate_bank(filename, bank))
    return errors


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] == "--validate":
        errors = validate_all_banks()
        if errors:
            print("\n".join(errors))
            return 1
        print("All Security+ question banks are valid.")
        return 0

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
