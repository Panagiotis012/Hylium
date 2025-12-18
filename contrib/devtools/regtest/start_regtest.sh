#!/usr/bin/env bash
set -euo pipefail
hyliumd -regtest -daemon
hylium-cli -regtest -rpcwait getblockchaininfo >/dev/null
echo "regtest up"
hylium-cli -regtest listwallets
