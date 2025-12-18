#!/usr/bin/env bash
set -euo pipefail

hylium-cli -regtest -rpcwait getblockchaininfo >/dev/null

# ensure wallet loaded
if ! hylium-cli -regtest listwallets | grep -q '"dev"'; then
  hylium-cli -regtest loadwallet dev >/dev/null
fi

MINER=$(hylium-cli -regtest -rpcwallet=dev getnewaddress)
hylium-cli -regtest -rpcwallet=dev generatetoaddress 1 "$MINER" >/dev/null

TO=$(hylium-cli -regtest -rpcwallet=dev getnewaddress)
TXID=$(hylium-cli -regtest -rpcwallet=dev sendtoaddress "$TO" 1)
echo "TXID=$TXID"

# confirm
hylium-cli -regtest -rpcwallet=dev generatetoaddress 1 "$MINER" >/dev/null
hylium-cli -regtest -rpcwallet=dev gettransaction "$TXID" | grep -E '"confirmations"|\"txid\"'
echo "OK"
