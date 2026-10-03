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

    def damage_records(self):
        for item, missile, amount in [('ZRCMELEE', 1, 5), ('ZRCARROW', 2, 9),
                                     ('ZRCBOLT', 12, 9), ('ZRCSTONE', 17, 9)]:
            self.records += [dict(op='weapondamage-assembled', item=item, damage=amount,
                                  missile=missile, valid=1),
                             dict(op='weapondamage', item=item, hpBefore=100,
                                  hpAfter=100-amount, expectedDamage=amount)]
        self.records.append(dict(op='verdict', verdict='PASS'))

    def test_cleanup_requires_both_branches(self):
        self.damage_records()
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, cleanup=True)

    def test_cleanup_retention_and_removal_are_preserved(self):
        self.damage_records()
        self.records += [dict(op='projectile-compress', case='unregistered', returned=0, retained=1),
                         dict(op='projectile-compress', case='registered', returned=0, removed=1)]
        self.write()
        self.assertEqual(len(reference.reconstructed_events(self.path, cleanup=True)), 10)
        self.records[-1]['removed'] = 0
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, cleanup=True)

    def swing_records(self):
        self.damage_records()
        for item in ('ZRCMELEE', 'ZRCARROW', 'ZRCBOLT', 'ZRCSTONE'):
            amount = 6 if item == 'ZRCMELEE' else 12
            self.records += [dict(op='swingdamage-round', item=item, calls=102,
                                  remaining=3 if item == 'ZRCMELEE' else 2, valid=1),
                             dict(op='swingdamage', item=item, hpBefore=100,
                                  hpAfter=100-amount, expectedDamage=amount)]

    def test_round_mode_rejects_base_only_capture(self):
        self.damage_records()
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, rounds=True)

    def test_round_mode_preserves_payload_damage_and_ammunition(self):
        self.swing_records()
        self.write()
        records = reference.reconstructed_events(self.path, rounds=True)
        self.assertEqual(len(records), 16)
        self.assertEqual(records[-1]['damage'], 12)
        self.records[-2]['remaining'] = 3
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, rounds=True)

    def test_round_mode_rejects_missing_launcher_payload(self):
        self.swing_records()
        self.records[-1]['hpAfter'] = 91
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, rounds=True)

    def attack_records(self):
        self.swing_records()
        actions = dict(ZRCMELEE=3, ZRCARROW=98, ZRCBOLT=105, ZRCSTONE=3)
        for record in self.records:
            if record.get('op') == 'swingdamage-round':
                action = actions[record['item']]
                record.update(op='attackdamage-round', action=action,
                              returned=-1 if action == 105 else 1, frame=-2)
            elif record.get('op') == 'swingdamage':
                record['op'] = 'attackdamage'

    def test_attack_mode_requires_action_evidence(self):
        self.attack_records()
        self.write()
        records = reference.reconstructed_events(self.path, attack=True)
        self.assertEqual(len(records), 16)
        self.assertEqual(records[12]['action'], 105)
        self.assertEqual(records[12]['returned'], -1)
        del self.records[-2]['action']
        self.write()
        with self.assertRaises(KeyError):
            reference.reconstructed_events(self.path, attack=True)

    def test_attack_one_round_must_complete(self):
        self.attack_records()
        next(e for e in self.records if e.get('action') == 105)['returned'] = 1
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, attack=True)

    def test_attack_mode_rejects_wrong_action_or_unfinished_frame(self):
        self.attack_records()
        self.records[-2]['frame'] = 101
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, attack=True)
        self.records[-2]['frame'] = -2
        self.records[-2]['action'] = 105
        self.write()
        with self.assertRaises(ValueError):
            reference.reconstructed_events(self.path, attack=True)


if __name__ == '__main__':
    unittest.main()
