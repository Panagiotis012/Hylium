#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Regenerate BIP324 packet test vectors for the current network magic."""

from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.append(str(ROOT / "test" / "functional"))

from test_framework.v2_p2p import EncryptedP2PState  # noqa: E402


def parse_arg(raw: str):
    raw = raw.strip().rstrip(",")
    if raw.startswith('"') and raw.endswith('"'):
        return raw[1:-1]
    if raw == "true":
        return True
    if raw == "false":
        return False
    return int(raw)


def format_arg(value):
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, int):
        return str(value)
    return f"\"{value}\""


def compute_outputs(args):
    (
        in_idx,
        in_priv_ours_hex,
        in_ellswift_ours_hex,
        in_ellswift_theirs_hex,
        in_initiating,
        in_contents_hex,
        in_multiply,
        in_aad_hex,
        in_ignore,
        _mid_send_garbage_hex,
        _mid_recv_garbage_hex,
        _out_session_id_hex,
        out_ciphertext_hex,
        out_ciphertext_endswith_hex,
    ) = args

    priv = bytes.fromhex(in_priv_ours_hex)
    ellswift_ours = bytes.fromhex(in_ellswift_ours_hex)
    ellswift_theirs = bytes.fromhex(in_ellswift_theirs_hex)

    state = EncryptedP2PState(initiating=in_initiating, net="mainnet")
    state.privkey_ours = priv
    state.ellswift_ours = ellswift_ours
    ecdh_secret = EncryptedP2PState.v2_ecdh(priv, ellswift_theirs, ellswift_ours, in_initiating)
    state.initialize_v2_transport(ecdh_secret)

    mid_send_garbage_hex = state.peer["send_garbage_terminator"].hex()
    mid_recv_garbage_hex = state.peer["recv_garbage_terminator"].hex()
    out_session_id_hex = state.peer["session_id"].hex()

    for _ in range(in_idx):
        state.v2_enc_packet(b"", aad=b"", ignore=True)

    contents = bytes.fromhex(in_contents_hex) * in_multiply
    aad = bytes.fromhex(in_aad_hex)
    ciphertext = state.v2_enc_packet(contents, aad=aad, ignore=in_ignore)
    ciphertext_hex = ciphertext.hex()

    if out_ciphertext_hex:
        new_ciphertext_hex = ciphertext_hex
        new_ciphertext_endswith_hex = ""
    else:
        suffix_len = len(out_ciphertext_endswith_hex) // 2
        new_ciphertext_hex = ""
        new_ciphertext_endswith_hex = ciphertext_hex[-suffix_len * 2:] if suffix_len else ""

    return (
        mid_send_garbage_hex,
        mid_recv_garbage_hex,
        out_session_id_hex,
        new_ciphertext_hex,
        new_ciphertext_endswith_hex,
    )


def main() -> int:
    path = ROOT / "src" / "test" / "bip324_tests.cpp"
    lines = path.read_text(encoding="utf-8").splitlines()

    out_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.lstrip()
        if "TestBIP324PacketVector(" not in line or stripped.startswith("//") or line == stripped:
            out_lines.append(line)
            i += 1
            continue

        block = [line]
        i += 1
        while i < len(lines):
            block.append(lines[i])
            if lines[i].strip().endswith(");"):
                i += 1
                break
            i += 1

        args = []
        for entry in block[1:]:
            entry = entry.strip()
            if entry.endswith(");"):
                entry = entry[:-2]
            if entry:
                args.append(parse_arg(entry))

        if len(args) != 14:
            raise SystemExit(f"Unexpected arg count: {len(args)}")

        (
            mid_send_garbage_hex,
            mid_recv_garbage_hex,
            out_session_id_hex,
            out_ciphertext_hex,
            out_ciphertext_endswith_hex,
        ) = compute_outputs(args)

        args[9] = mid_send_garbage_hex
        args[10] = mid_recv_garbage_hex
        args[11] = out_session_id_hex
        args[12] = out_ciphertext_hex
        args[13] = out_ciphertext_endswith_hex

        out_lines.append(block[0])
        for idx, value in enumerate(args):
            suffix = "," if idx < len(args) - 1 else ""
            out_lines.append(f"        {format_arg(value)}{suffix}")
        out_lines.append("    );")

    path.write_text("\n".join(out_lines) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
