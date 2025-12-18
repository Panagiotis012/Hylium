#!/usr/bin/env bash
set -euo pipefail
hylium-cli -regtest stop 2>/dev/null || true
pkill -f "hyliumd -regtest" 2>/dev/null || true
echo "regtest down"
