#!/usr/bin/env python3
"""Install/remove deterministic ITM payload fixtures for combat-payload.txt.

These exercise weapon payloads, base damage and projectile delivery in the
combat scenarios. No existing resource is overwritten. Removal verifies bytes.
"""
import argparse
import struct
from pathlib import Path

from runtime_paths import game_directory


def item(item_type, ability_type, launcher_type, projectile, amount):
    header = bytearray(0x82)  # ITM V2.0
    header[:8] = b"ITM V2.0"
    struct.pack_into("<ii", header, 8, -1, -1)
    struct.pack_into("<H", header, 0x1C, item_type)
    struct.pack_into("<H", header, 0x38, 1)
    struct.pack_into("<IHI", header, 0x64, len(header), 1, len(header) + 0x38)
    ability = bytearray(0x38)
    struct.pack_into("<H", ability, 0, ability_type)
    struct.pack_into("<H", ability, 0x0E, 30)
    ability[0x10] = launcher_type
    if amount:
        ability[0x16] = 2 if ability_type == 4 else 4
        ability[0x18] = 1
        struct.pack_into("<hH", ability, 0x1A, amount, 2)
    struct.pack_into("<H", ability, 0x1E, 1)
    struct.pack_into("<H", ability, 0x2A, projectile)
    effect = bytearray(0x30)
    struct.pack_into("<HBBi", effect, 0, 12, 2, 0, amount)
    effect[0x12] = 100
    struct.pack_into("<i", effect, 0x2C, -1)
    return bytes(header + ability + effect)


FIXTURES = {
    "ZRCMELEE": item(20, 1, 0, 1, 1),
    "ZRCARROW": item(5, 2, 1, 2, 1),
    "ZRCBOLT": item(31, 2, 2, 12, 1),
    "ZRCSTONE": item(14, 2, 3, 17, 1),
    "ZRCBOW": item(15, 4, 0, 1, 2),
    "ZRCXBOW": item(27, 4, 0, 1, 2),
    "ZRCSLING": item(18, 4, 0, 1, 2),
    "ZRCDAGGR": item(16, 1, 0, 1, 0),
    "ZRCARMOR": item(63, 1, 0, 1, 0),
    "ZRCSHLD": item(41, 1, 0, 1, 0),
    "ZZA5TST": item(20, 1, 0, 1, 0),
    "ZZ05TST": item(15, 4, 0, 1, 0),
    "ZZ35TST": item(15, 4, 0, 1, 0),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("operation", choices=["install", "remove"])
    parser.add_argument("--game-dir", type=Path, default=game_directory())
    args = parser.parse_args()
    override = args.game_dir.resolve() / "override"
    paths = [(override / (name + ".ITM"), data) for name, data in FIXTURES.items()]
    for path, data in paths:
        if path.exists() and (args.operation == "install" or path.read_bytes() != data):
            parser.error(f"Refusing to overwrite/remove existing or changed resource: {path}")
    if args.operation == "install":
        override.mkdir(exist_ok=True)
        for path, data in paths:
            with path.open("xb") as out:
                out.write(data)
    else:
        for path, _ in paths:
            path.unlink(missing_ok=True)
    print(f"{args.operation}: {len(paths)} combat payload fixtures in {override}")


if __name__ == "__main__":
    main()
