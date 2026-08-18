import json
import random
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import drill


class MatchTests(unittest.TestCase):
    def test_explicit_aliases_match_after_normalization(self):
        self.assertTrue(drill.matches("TCP/UDP", ["tcp and udp"]))
        self.assertTrue(drill.matches("161 udp", ["161", "161 udp"]))

    def test_substrings_are_not_implicitly_accepted(self):
        self.assertFalse(drill.matches("not actually detective", ["detective"]))
        self.assertFalse(drill.matches("secure shell extra", ["secure shell"]))
        self.assertFalse(drill.matches("22 udp", ["22", "22 tcp"]))


class BankTests(unittest.TestCase):
    def test_all_banks_validate(self):
        self.assertEqual(drill.validate_all_banks(), [])

    def test_port_reverse_alias_and_shared_item_id(self):
        _, questions = drill.load_questions("ports.json")
        ftp_forward = next(q for q in questions if q["prompt"] == "FTP data port?")
        ftp_reverse = next(
            q for q in questions if q["prompt"] == "Port 20 — name one associated protocol?"
        )
        self.assertEqual(ftp_forward["item_id"], ftp_reverse["item_id"])
        self.assertTrue(drill.matches("FTP", ftp_reverse["answers"]))
        ssh_reverse = next(
            q for q in questions if q["prompt"] == "Port 22 — name one associated protocol?"
        )
        self.assertTrue(drill.matches("SCP", ssh_reverse["answers"]))

    def test_non_core_ports_are_not_expanded(self):
        _, questions = drill.load_questions("ports.json")
        prompts = {q["prompt"] for q in questions}
        self.assertNotIn("Diameter port?", prompts)
        self.assertNotIn("SRTP port?", prompts)

    def test_corrected_port_facts(self):
        bank = drill.load_json("ports.json")
        by_id = {item["id"]: item for item in bank["items"]}
        self.assertEqual(by_id["ldap"]["transport"], "tcp")
        self.assertEqual(by_id["dns"]["transport"], "both")
        self.assertEqual(by_id["pptp"]["transport"], "tcp")
        self.assertEqual(
            by_id["isakmp"]["full_name"],
            "Internet Security Association and Key Management Protocol",
        )

    def test_control_canonical_answers_and_explanations(self):
        _, questions = drill.load_questions("controls.json")
        training = next(
            q
            for q in questions
            if q["prompt"] == "Security awareness training — type?"
        )
        fim = next(
            q
            for q in questions
            if q["prompt"] == "File integrity monitoring (FIM) — type?"
        )
        self.assertEqual(training["reveal"], "directive (preventive also accepted)")
        self.assertTrue(drill.matches("directive", training["answers"]))
        self.assertIn("known-good baseline", fim["extra"])
        self.assertEqual(fim["reveal"], "detective")

    def test_validation_rejects_bad_question(self):
        errors = drill.validate_bank(
            "bad.json",
            {
                "title": "Bad",
                "questions": [
                    {"id": "x", "prompt": "Question?", "answers": [], "reveal": "x"}
                ],
            },
        )
        self.assertTrue(any("missing answers" in error for error in errors))
        self.assertTrue(any("missing explanation" in error for error in errors))

    def test_validation_rejects_duplicate_prompts(self):
        question = {
            "prompt": "Same?",
            "answers": ["yes"],
            "reveal": "yes",
            "explanation": "Reason",
        }
        errors = drill.validate_bank(
            "bad.json",
            {
                "title": "Bad",
                "questions": [
                    {"id": "one", **question},
                    {"id": "two", **question},
                ],
            },
        )
        self.assertTrue(any("duplicate prompt" in error for error in errors))

    def test_validation_reports_non_object_records(self):
        errors = drill.validate_bank(
            "bad.json", {"title": "Bad", "questions": [None]}
        )
        self.assertTrue(any("expected an object" in error for error in errors))
        port_errors = drill.validate_bank(
            "bad-ports.json",
            {"title": "Bad ports", "kind": "ports", "items": [None]},
        )
        self.assertTrue(any("expected an object" in error for error in port_errors))


class StatsAndWeightingTests(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.stats_path = Path(self.temp_dir.name) / "stats.json"
        self.path_patch = mock.patch.object(drill, "STATS_PATH", self.stats_path)
        self.path_patch.start()

    def tearDown(self):
        self.path_patch.stop()
        self.temp_dir.cleanup()

    def test_old_aggregate_stats_migrate_without_fake_item_history(self):
        self.stats_path.write_text(
            json.dumps(
                {
                    "window": 20,
                    "banks": {
                        "ports.json": {"hits": 2, "at_bats": 3, "recent": [1, 0, 1]}
                    },
                }
            ),
            encoding="utf-8",
        )
        stats = drill.load_stats()
        self.assertEqual(stats["banks"]["ports.json"]["hits"], 2)
        self.assertEqual(stats["items"], {})

    def test_wrong_increases_and_correct_gradually_reduces_weakness(self):
        stats = {"window": 20, "banks": {}, "items": {}}
        drill.record_result(stats, "ports.json", "ports:20", False)
        self.assertEqual(stats["items"]["ports:20"]["weakness"], 2)
        drill.record_result(stats, "ports.json", "ports:20", True)
        self.assertEqual(stats["items"]["ports:20"]["weakness"], 1)
        drill.record_result(stats, "ports.json", "ports:20", True)
        self.assertEqual(stats["items"]["ports:20"]["weakness"], 0)

    def test_weighted_selection_favors_weak_item(self):
        stats = {
            "items": {
                "a": {"weakness": 0},
                "b": {"weakness": 4},
            }
        }
        groups = {"a": [{}], "b": [{}]}

        class PickLargest:
            @staticmethod
            def choices(items, weights, k):
                return [items[weights.index(max(weights))]]

        self.assertEqual(drill.choose_item(groups, stats, [], PickLargest()), "b")

    def test_skip_does_not_record_result(self):
        stats = {"window": 20, "banks": {}, "items": {}}
        question = {
            "prompt": "Test?",
            "answers": ["yes"],
            "reveal": "yes",
            "extra": "Explanation",
            "item_id": "test:item",
            "variant_id": "test:item:one",
        }
        with mock.patch.object(drill, "ask", side_effect=["s", "q"]):
            with mock.patch("builtins.print"):
                drill.drill("Test", [question], stats, "test.json")
        self.assertEqual(stats["banks"]["test.json"]["at_bats"], 0)
        self.assertNotIn("test:item", stats["items"])

    def test_missed_item_returns_after_intervening_answers(self):
        stats = {"window": 20, "banks": {}, "items": {}}
        questions = [
            {
                "prompt": "A?",
                "answers": ["a"],
                "reveal": "a",
                "item_id": "a",
                "variant_id": "a:one",
            },
            {
                "prompt": "B?",
                "answers": ["b"],
                "reveal": "b",
                "item_id": "b",
                "variant_id": "b:one",
            },
        ]
        responses = iter(["wrong", "b", "a", "b", "a", "a", "q"])
        prompts = []

        def fake_ask(prompt):
            prompts.append(prompt)
            return next(responses)

        selected = iter(["a", "b", "a", "b", "a", "b"])
        with mock.patch.object(drill, "ask", side_effect=fake_ask):
            with mock.patch.object(
                drill, "choose_item", side_effect=lambda *_args: next(selected)
            ):
                with mock.patch("builtins.print"):
                    drill.drill("Test", questions, stats, "test.json")
        self.assertEqual(prompts[0], "A? ")
        self.assertEqual(prompts[5], "A? ")

    def test_variant_selection_uses_least_seen_form(self):
        variants = [
            {"variant_id": "x:forward"},
            {"variant_id": "x:reverse"},
        ]
        counts = {"x:forward": 3, "x:reverse": 1}
        picked = drill.choose_variant(variants, counts, random.Random(1))
        self.assertEqual(picked["variant_id"], "x:reverse")


if __name__ == "__main__":
    unittest.main()
