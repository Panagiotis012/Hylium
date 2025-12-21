#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Rewrite BIP32 test vectors to use Hylium extended key version bytes."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

OLD_PUB = bytes.fromhex("0488b21e")  # xpub
OLD_PRV = bytes.fromhex("0488ade4")  # xprv
NEW_PUB = bytes.fromhex("049d7cb2")  # hpub
NEW_PRV = bytes.fromhex("049d7a04")  # hprv

ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"


def b58encode(b: bytes) -> str:
    n = int.from_bytes(b, "big")
    out = []
    while n > 0:
        n, rem = divmod(n, 58)
        out.append(ALPHABET[rem])
    out = "".join(reversed(out)) if out else ""
    pad = 0
    for c in b:
        if c == 0:
            pad += 1
        else:
            break
    return "1" * pad + out


def b58decode(s: str) -> bytes:
    n = 0
    for c in s:
        n = n * 58 + ALPHABET.index(c)
    h = n.to_bytes((n.bit_length() + 7) // 8, "big")
    pad = 0
    for c in s:
        if c == "1":
            pad += 1
        else:
            break
    return b"\x00" * pad + h


def b58check_decode(s: str) -> bytes:
    raw = b58decode(s)
    if len(raw) < 4:
        raise ValueError("short base58")
    data, checksum = raw[:-4], raw[-4:]
    expect = hashlib.sha256(hashlib.sha256(data).digest()).digest()[:4]
    if checksum != expect:
        raise ValueError("bad checksum")
    return data


def b58check_encode(data: bytes) -> str:
    chk = hashlib.sha256(hashlib.sha256(data).digest()).digest()[:4]
    return b58encode(data + chk)


def convert_extkey(s: str) -> str:
    data = b58check_decode(s)
    prefix = data[:4]
    if prefix == OLD_PUB:
        data = NEW_PUB + data[4:]
    elif prefix == OLD_PRV:
        data = NEW_PRV + data[4:]
    else:
        return s
    return b58check_encode(data)


def main() -> int:
    path = Path("src/test/bip32_tests.cpp")
    text = path.read_text(encoding="utf-8")
    marker = "const std::vector<std::string> TEST5"
    head, tail = text.split(marker, 1)

    def repl(match: re.Match) -> str:
        value = match.group(1)
        return f"\"{convert_extkey(value)}\""

    head = re.sub(r"\"(xpub[1-9A-HJ-NP-Za-km-z]+)\"", repl, head)
    head = re.sub(r"\"(xprv[1-9A-HJ-NP-Za-km-z]+)\"", repl, head)

    path.write_text(head + marker + tail, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
