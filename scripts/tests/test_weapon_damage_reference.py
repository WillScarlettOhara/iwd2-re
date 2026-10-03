"""Reject captures that cannot establish comparable weapon-test inputs."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

PROBE = Path(__file__).resolve().parents[1] / 'probes/weapon_damage_reference.py'
SPEC = importlib.util.spec_from_file_location('weapon_reference', PROBE)
reference = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(reference)


class ActorInputTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / 'ui.jsonl'
        self.context = dict(op='damage-context', x=232, y=810, baseCrushing=0,
                            barbarianLevel=0, tempMaxHP=291, derivedCrushing=0,
                            tempCrushing=0, difficulty=3, multiplier=0, mpMultiplier=0)
        self.records = [dict(op='center', wx=232, wy=858)] + [dict(self.context) for _ in range(4)]

    def write(self):
        self.path.write_text(''.join(json.dumps(e)+'\n' for e in self.records), encoding='utf-8')

    def test_world_positions_survive_party_slot_reordering(self):
        self.write()
        positions, context = reference.actor_inputs(self.path)
        self.assertEqual(positions, [[232, 858], [232, 810]])
        self.assertEqual(context['tempMaxHP'], 291)

    def test_incomplete_context_is_rejected(self):
        self.records.pop()
        self.write()
        with self.assertRaises(ValueError):
            reference.actor_inputs(self.path)

    def test_target_resistance_change_is_rejected(self):
        self.records[-1]['tempCrushing'] = 4
        self.write()
        with self.assertRaises(ValueError):
            reference.actor_inputs(self.path)

    def test_overlapping_source_and_target_is_rejected(self):
        self.records[0]['wy'] = 810
        self.write()
        with self.assertRaises(ValueError):
            reference.actor_inputs(self.path)

    def test_pass_without_damage_records_is_rejected(self):
        self.records.append(dict(op='verdict', verdict='PASS'))
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path)


if __name__ == '__main__':
    unittest.main()
