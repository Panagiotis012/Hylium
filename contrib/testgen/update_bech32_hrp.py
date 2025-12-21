#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Update bech32/bech32m HRPs in text files and refresh addr() descriptor checksums.

Only valid segwit addresses are rewritten; invalid addresses are left untouched.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.append(str(ROOT / "test" / "functional"))

from test_framework.segwit_addr import decode_segwit_address, encode_segwit_address  # noqa: E402
from test_framework.descriptors import descsum_create  # noqa: E402

HRP_MAP = {
    "bc": "hyl",
    "tb": "thyl",
    "bcrt": "rhyl",
}

BECH32_RE = re.compile(r"\b(bc|tb|bcrt)1[0-9a-zA-Z]+\b")
ADDR_DESC_RE = re.compile(r"addr\\(([0-9a-zA-Z]+)\\)#[0-9a-z]{8}")


def reencode_address(addr: str) -> str | None:
    hrp = addr.split("1", 1)[0].lower()
    new_hrp = HRP_MAP.get(hrp)
    if not new_hrp:
        return None
    witver, witprog = decode_segwit_address(hrp, addr)
    if witver is None:
        return None
    return encode_segwit_address(new_hrp, witver, witprog)


def update_file(path: Path) -> bool:
    original = path.read_text(encoding="utf-8")

    def replace_addr(match: re.Match) -> str:
        addr = match.group(0)
        updated = reencode_address(addr)
        return updated if updated else addr

    updated = BECH32_RE.sub(replace_addr, original)

    def replace_desc(match: re.Match) -> str:
        addr = match.group(1)
        return descsum_create(f"addr({addr})")

    updated = ADDR_DESC_RE.sub(replace_desc, updated)

    if updated != original:
        path.write_text(updated, encoding="utf-8")
        return True
    return False


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", help="Files to rewrite")
    args = parser.parse_args()

    changed = 0
    for name in args.files:
        path = Path(name)
        if not path.is_file():
            raise SystemExit(f"file not found: {path}")
        if update_file(path):
            changed += 1
    print(f"Updated {changed} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
