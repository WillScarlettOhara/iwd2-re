"""Run the actual Frida calibration math/stability helpers with Node, no game."""
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from frida_orig import CURSOR_CALIBRATION_JS, build_js


@unittest.skipUnless(shutil.which("node"), "Node is needed to execute the Frida JS helpers")
class CalibrationTest(unittest.TestCase):
    def run_js(self, assertions):
        result = subprocess.run([shutil.which("node"), "-"],
                                input="const assert = require('node:assert/strict');\n" + CURSOR_CALIBRATION_JS + assertions,
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_negative_stale_native_sample_rejected(self):
        self.run_js("""
assert.equal(fitCursorMap({desktop:[200,200], game:[191,230]},
                         {desktop:[600,400], game:[166,297]}), null);
""")

    def test_off_window_and_frozen_samples_rejected(self):
        self.run_js("""
assert.equal(fitCursorMap({desktop:[200,200], game:[-1,-1]},
                         {desktop:[600,400], game:[600,400]}), null);
assert.equal(fitCursorMap({desktop:[200,200], game:[200,200]},
                         {desktop:[600,400], game:[200,200]}), null);
""")

    def test_client_offset_scale_and_third_point_validation(self):
        self.run_js("""
const map = fitCursorMap({desktop:[500,400],game:[200,150]},
                         {desktop:[1300,1000],game:[600,450]});
assert.deepEqual(map, {ax:0.5,ay:0.5,bx:-50,by:-50});
assert.equal(mapMatchesSample(map,{desktop:[900,700],game:[400,300]},2),true);
assert.equal(mapMatchesSample(map,{desktop:[900,700],game:[390,300]},2),false);
""")

    def test_sampling_waits_multiple_ticks_and_time(self):
        self.run_js("""
const state={started:0,polls:0,stable:0,last:null};
for(let tick=1;tick<=8;tick++) assert.equal(stableCursorSample(state,[200,150],true,tick*10),null);
assert.equal(stableCursorSample(state,[200,150],true,160),null);
assert.equal(stableCursorSample(state,[200,150],true,190),null);
assert.deepEqual(stableCursorSample(state,[200,150],true,220),[200,150]);
""")

    def test_lost_ownership_resets_sample_stability(self):
        self.run_js("""
const state={started:0,polls:5,stable:2,last:[200,150]};
assert.equal(stableCursorSample(state,[200,150],false,200),null);
assert.equal(state.stable,0);
assert.equal(stableCursorSample(state,[-1,-1],true,220),null);
assert.equal(stableCursorSample(state,[200,150],true,250),null);
""")

    def test_complete_generated_script_parses(self):
        script = build_js({"hooks": []}, slot=3, settle=10)
        result = subprocess.run([shutil.which("node"), "--check", "-"], input=script,
                                text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
