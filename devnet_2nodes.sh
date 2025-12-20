#!/usr/bin/env bash
set -Eeuo pipefail

A=/tmp/hylA
B=/tmp/hylB2

A_P2P=41445
A_RPC=41446
B_P2P=42445
B_RPC=42446

rpcA(){ hylium-cli -regtest -datadir="$A" -rpcport="$A_RPC" -rpcuser=auser -rpcpassword=apass "$@"; }
rpcB(){ hylium-cli -regtest -datadir="$B" -rpcport="$B_RPC" -rpcuser=buser -rpcpassword=bpass "$@"; }

cleanup() {
  rpcA stop 2>/dev/null || true
  rpcB stop 2>/dev/null || true
  pkill -f "hyliumd.*datadir=$A" 2>/dev/null || true
  pkill -f "hyliumd.*datadir=$B" 2>/dev/null || true
}
trap cleanup EXIT

wait_peers() {
  local which="$1"
  for _ in {1..80}; do
    local n
    if [ "$which" = "A" ]; then
      n=$(rpcA getpeerinfo | jq 'length' 2>/dev/null || echo 0)
    else
      n=$(rpcB getpeerinfo | jq 'length' 2>/dev/null || echo 0)
    fi
    [ "$n" -ge 1 ] && return 0
    sleep 0.25
  done
  echo "ERROR: node $which has no peers" >&2
  return 1
}

wait_height() {
  local which="$1" target="$2"
  for _ in {1..200}; do
    local h
    if [ "$which" = "A" ]; then h=$(rpcA getblockcount 2>/dev/null || echo -1); else h=$(rpcB getblockcount 2>/dev/null || echo -1); fi
    [ "$h" -ge "$target" ] && return 0
    sleep 0.25
  done
  echo "ERROR: node $which did not reach height >= $target" >&2
  return 1
}

wait_besthash_A_equals_B() {
  local bbest
  bbest=$(rpcB getbestblockhash)
  for _ in {1..200}; do
    [ "$(rpcA getbestblockhash)" = "$bbest" ] && return 0
    sleep 0.25
  done
  echo "ERROR: A did not sync B bestblockhash" >&2
  echo "A=$(rpcA getbestblockhash)" >&2
  echo "B=$bbest" >&2
  return 1
}

wait_mempool_tx_B() {
  local txid="$1"
  for _ in {1..200}; do
    if rpcB getrawmempool | jq -e --arg tx "$txid" 'index($tx) != null' >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.25
  done
  echo "ERROR: tx not seen in B mempool: $txid" >&2
  return 1
}

echo "[*] kill old daemons"
pkill -f hyliumd 2>/dev/null || true
sleep 1

echo "[*] init datadirs"
rm -rf "$A" "$B"
mkdir -p "$A" "$B"

cat > "$A/hylium.conf" <<CONF
regtest=1
server=1
fallbackfee=0.00001
upnp=0
listenonion=0
rpcuser=auser
rpcpassword=apass

[regtest]
listen=1
bind=127.0.0.1
port=$A_P2P
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
rpcport=$A_RPC
CONF

cat > "$B/hylium.conf" <<CONF
regtest=1
server=1
fallbackfee=0.00001
upnp=0
listenonion=0
rpcuser=buser
rpcpassword=bpass

[regtest]
listen=1
bind=127.0.0.1
port=$B_P2P
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
rpcport=$B_RPC
CONF

echo "[*] start daemons"
hyliumd -datadir="$A" -daemon
hyliumd -datadir="$B" -daemon

echo "[*] wait RPC"
rpcA -rpcwait -rpcwaittimeout=10 getblockchaininfo >/dev/null
rpcB -rpcwait -rpcwaittimeout=10 getblockchaininfo >/dev/null

echo "[*] connect both ways"
rpcA addnode "127.0.0.1:$B_P2P" add
rpcB addnode "127.0.0.1:$A_P2P" add

echo "[*] wait peers"
wait_peers A
wait_peers B
rpcA getpeerinfo | jq 'length, .[].addr'
rpcB getpeerinfo | jq 'length, .[].addr'

echo "[*] wallets"
rpcA createwallet minerA >/dev/null || true
rpcB createwallet minerB >/dev/null || true

echo "[*] mine on A (101 blocks)"
A_MINER_ADDR=$(rpcA -rpcwallet=minerA getnewaddress)
rpcA -rpcwallet=minerA generatetoaddress 101 "$A_MINER_ADDR" >/dev/null
rpcA -rpcwallet=minerA getbalance

echo "[*] wait B sync to >=101"
wait_height B 101

echo "[*] send 1 to B"
B_ADDR=$(rpcB -rpcwallet=minerB getnewaddress)
TXID=$(rpcA -rpcwallet=minerA sendtoaddress "$B_ADDR" 1)
echo "TXID=$TXID"

echo "[*] wait B mempool sees tx"
wait_mempool_tx_B "$TXID"

echo "[*] mine 1 block on B (confirm tx)"
B_MINER_ADDR=$(rpcB -rpcwallet=minerB getnewaddress)
rpcB -rpcwallet=minerB generatetoaddress 1 "$B_MINER_ADDR" >/dev/null

echo "[*] wait A sync B's best block"
wait_besthash_A_equals_B

echo "[*] verify tx on both"
rpcA -rpcwallet=minerA gettransaction "$TXID" | jq '{txid, confirmations}'
rpcB -rpcwallet=minerB gettransaction "$TXID" | jq '{txid, confirmations, amount}'

echo "[OK] devnet up. Ctrl+C to stop."
while true; do sleep 3600; done
