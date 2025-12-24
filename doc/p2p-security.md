# Hylium P2P Network Security

**Version:** 1.0
**Last Updated:** 2025-12-24

This document outlines the security measures implemented in the Hylium P2P network layer to protect against Denial of Service (DoS) attacks, resource exhaustion, and malicious peers.

---

## Table of Contents

1. [Overview](#overview)
2. [Connection Management](#connection-management)
3. [Message Size Limits](#message-size-limits)
4. [Rate Limiting](#rate-limiting)
5. [Peer Misbehavior Scoring](#peer-misbehavior-scoring)
6. [Resource Limits](#resource-limits)
7. [Network Flood Protection](#network-flood-protection)
8. [Configuration Parameters](#configuration-parameters)

---

## Overview

The Hylium P2P network is a peer-to-peer overlay network where nodes communicate directly without a central authority. This architecture is robust but vulnerable to various attacks. Our security model follows these principles:

1. **Defense in Depth**: Multiple layers of protection
2. **Resource Bounds**: Strict limits on memory, disk, and CPU usage
3. **Fail-Safe Defaults**: Secure by default, configurable for advanced users
4. **Misbehavior Tracking**: Score-based peer banning system

---

## Connection Management

### Connection Limits

```cpp
// Default connection limits
MAX_INBOUND_CONNECTIONS = 125
MAX_OUTBOUND_CONNECTIONS = 10  // Full relay
MAX_BLOCK_RELAY_ONLY_CONNECTIONS = 8
MAX_FEELER_CONNECTIONS = 1
MAX_ADDNODE_CONNECTIONS = 8
```

**Rationale:**
- Limits total resource usage per node
- Prevents connection exhaustion attacks
- Balances inbound (accept) vs outbound (initiate) connections

### Connection Diversity

**Outbound Connection Selection:**
- Select peers from diverse /16 IPv4 subnets (or /32 IPv6)
- Prefer peers with different ASNs (Autonomous System Numbers)
- Avoid connecting to multiple peers behind same IP

**Protection Against:**
- Sybil attacks (one attacker controls many IPs)
- Eclipse attacks (isolating a node from honest network)

### Handshake Timeout

```cpp
TIMEOUT_INTERVAL = 60 seconds
```

Peers must complete version handshake within 60 seconds or be disconnected.

**Protection Against:**
- Slowloris-style attacks (incomplete handshakes holding connections)

---

## Message Size Limits

### Per-Message Limits

```cpp
// Maximum message sizes
MAX_PROTOCOL_MESSAGE_LENGTH = 4,000,000 bytes  // ~4 MB
MAX_HEADERS_RESULTS = 2,000 headers
MAX_INV_SZ = 50,000 inventory items
MAX_LOCATOR_SZ = 101 block locators
MAX_ADDR_TO_SEND = 1,000 addresses per ADDR message
```

### Block and Transaction Limits

```cpp
MAX_BLOCK_SERIALIZED_SIZE = 4,000,000 bytes
MAX_STANDARD_TX_WEIGHT = 400,000 weight units
```

**Protection Against:**
- Memory exhaustion from oversized messages
- CPU exhaustion from processing huge structures

---

## Rate Limiting

### Message Frequency Limits

Peers are rate-limited on sensitive message types:

```cpp
// Inventory items (blocks, txs) per second
MAX_INV_SENDS_PER_SECOND = 7

// Address announcements
MAX_ADDR_RATE_PER_SECOND = 0.1  // Once per 10 seconds

// Ping/pong frequency
PING_INTERVAL = 120 seconds
```

### Bandwidth Limits

```cpp
// Upload throttling (optional, default: unlimited)
-maxuploadtarget=<MiB per day>  // e.g., 144 for ~1 Mbps average

// Download limits
- Blocks: Unlimited from outbound peers
- Transactions: Rate-limited per peer
```

**Protection Against:**
- Bandwidth exhaustion
- Flooding attacks

---

## Peer Misbehavior Scoring

### Misbehavior Points System

Each peer has a misbehavior score. When score exceeds threshold, peer is banned.

```cpp
MISBEHAVIOR_BAN_THRESHOLD = 100 points
DEFAULT_BAN_TIME = 24 hours
```

### Point Penalties

| Misbehavior | Points | Description |
|-------------|--------|-------------|
| Invalid block | 100 | Sends syntactically invalid block |
| Invalid transaction | 100 | Sends invalid transaction |
| Duplicate block | 10 | Sends same block twice |
| Invalid proof-of-work | 50 | Block doesn't meet target |
| Protocol version violation | 100 | Violates handshake protocol |
| Excessive inventory | 20 | Sends too many INV messages |
| Bad headers | 10 | Non-continuous header chain |
| Unrequested block/tx | 10 | Sends unsolicited data |

### Automatic Banning

Immediate ban (no score needed):
- Sends more than `MAX_PROTOCOL_MESSAGE_LENGTH` bytes in single message
- Violates known consensus rules
- Attempts known protocol exploits

### Manual Banning

Operators can manually ban peers:
```bash
hylium-cli setban "192.168.1.1" add 86400  # Ban for 24 hours
hylium-cli setban "192.168.1.0/24" add      # Ban subnet indefinitely
```

---

## Resource Limits

### Memory Limits

```cpp
// Mempool
DEFAULT_MAX_MEMPOOL_SIZE = 300 MB
DEFAULT_MEMPOOL_EXPIRY = 336 hours  // 14 days

// Orphan transactions (missing parent)
MAX_ORPHAN_TRANSACTIONS = 100
MAX_ORPHAN_WEIGHT = 400,000  // Per orphan

// Rejected transactions (DoS cache)
MAX_REJECT_FILTER_SIZE = 50,000 entries

// Headers
MAX_UNCONNECTED_HEADERS = 10  // Headers without known parent
```

**Protection Against:**
- Memory exhaustion from endless transaction submissions
- Orphan transaction flooding

### Disk Limits

```cpp
// Blocks awaiting validation
MAX_BLOCKS_IN_TRANSIT_PER_PEER = 16
MAX_BLOCKS_TO_ANNOUNCE = 8  // Per peer inventory
```

### CPU Limits

```cpp
// Signature operations per transaction (relay policy)
MAX_STANDARD_TX_SIGOPS = MAX_BLOCK_SIGOPS_COST / 5

// Script validation (per input)
MAX_OPS_PER_SCRIPT = 201
MAX_SCRIPT_SIZE = 10,000 bytes
MAX_SCRIPT_ELEMENT_SIZE = 520 bytes (pre-Taproot)
```

**Protection Against:**
- CPU exhaustion from expensive signature validation
- Quadratic hashing attacks (fixed with SegWit)

---

## Network Flood Protection

### Transaction Relay

**Fee-Based Priority:**
- Maintain mempool sorted by fee rate
- Evict low-fee transactions when full
- Relay higher-fee transactions preferentially

**Rate Limiting:**
- Track transactions relayed per peer
- Limit to reasonable rate (prevents spam)

### Block Relay

**Compact Block Relay (BIP152):**
- Send only short transaction IDs, not full transactions
- Reduces bandwidth ~97% for block relay
- Faster propagation, less DoS surface

**Headers-First Sync:**
- Download headers before blocks
- Validate proof-of-work before requesting full blocks
- Prevents disk exhaustion from invalid blocks

### Address Relay

**Address Manager (AddrMan):**
- Separate "tried" and "new" address buckets
- Rate-limit address announcements
- Prevent address database pollution

**Protection Against:**
- Address flooding (filling addrman with attacker IPs)

---

## Configuration Parameters

### Connection Settings

```ini
# Maximum inbound connections
maxconnections=125

# Bind to specific IP
bind=0.0.0.0

# Listen for connections
listen=1

# Whitelist specific peers (no banning, prioritized)
whitelist=192.168.1.0/24

# Block-relay-only connections
blocksonly=0
```

### Bandwidth Settings

```ini
# Limit upload (MiB per 24 hours)
maxuploadtarget=5000

# Limit mempool size (MB)
maxmempool=300

# Mempool expiration (hours)
mempoolexpiry=336
```

### DoS Protection Settings

```ini
# Ban time (seconds)
bantime=86400

# Peer timeout (milliseconds)
timeout=60000

# Enable peer bloom filters (BIP37 - DISABLE for DoS protection)
peerbloomfilters=0
```

### Recommended Production Settings

```ini
# Full node (accepting inbound)
maxconnections=125
maxuploadtarget=0  # Unlimited
maxmempool=300
bantime=86400
peerbloomfilters=0  # Disabled for security
listen=1

# Limited bandwidth node
maxconnections=40
maxuploadtarget=5000  # ~5 GB/day
maxmempool=100
blocksonly=1  # Don't relay transactions
```

---

## Monitoring and Alerts

### Key Metrics to Monitor

1. **Connection Stats:**
   ```bash
   hylium-cli getnetworkinfo
   # Watch: connections, relayfee, warnings
   ```

2. **Peer Quality:**
   ```bash
   hylium-cli getpeerinfo
   # Watch: banscore, bytesrecv, bytessent, pingtime
   ```

3. **Mempool Health:**
   ```bash
   hylium-cli getmempoolinfo
   # Watch: size, bytes, usage
   ```

4. **Ban List:**
   ```bash
   hylium-cli listbanned
   # Watch for suspicious patterns
   ```

### Alert Conditions

🚨 **Critical Alerts:**
- Mempool size > 90% of limit
- Banned peers > 100 in 24 hours
- All connections to same /16 subnet (eclipse attack)
- Sustained high inbound traffic with low validation rate

⚠️ **Warning Alerts:**
- Average peer ping time > 1 second
- Connection count at max limit for > 1 hour
- Mempool growing faster than block processing

---

## Attack Scenarios and Mitigations

### 1. Connection Exhaustion Attack

**Attack:** Attacker opens max connections, preventing legitimate peers.

**Mitigations:**
- Inbound connection limits
- Automatic connection eviction (prefer outbound)
- IP-based rate limiting (OS-level via `iptables`)

### 2. Memory Exhaustion Attack

**Attack:** Flood with transactions to fill mempool RAM.

**Mitigations:**
- `maxmempool` limit
- Fee-based eviction
- Orphan transaction limits

### 3. Disk Exhaustion Attack

**Attack:** Send invalid blocks to fill disk.

**Mitigations:**
- Validate proof-of-work before saving
- Limit blocks-in-flight per peer
- Prune old blocks (optional)

### 4. CPU Exhaustion Attack

**Attack:** Send expensive-to-validate transactions.

**Mitigations:**
- Signature operation limits
- Script size limits
- Caching of validated signatures

### 5. Eclipse Attack

**Attack:** Isolate victim from honest network.

**Mitigations:**
- Outbound connection diversity (/16 subnets, ASNs)
- Anchor connections (persist on restart)
- Multiple P2P listen ports

### 6. Sybil Attack

**Attack:** One entity controls many nodes/IPs.

**Mitigations:**
- Proof-of-work for consensus (not identity)
- Diversified outbound connections
- ASN diversity preference

---

## Future Improvements

Planned security enhancements:

1. **BIP324 (Encrypted P2P Transport):**
   - Encrypt all P2P traffic
   - Prevent ISP-level eavesdropping
   - Target: Version 0.3.0

2. **Dandelion++ (Transaction Privacy):**
   - Improved transaction relay privacy
   - Prevent IP-to-transaction correlation
   - Target: Version 0.4.0

3. **ASMAP (ASN-Based Peer Selection):**
   - Use Autonomous System mapping for peer diversity
   - Improve eclipse attack resistance
   - Target: Version 0.2.0

4. **Tor/I2P Native Support:**
   - Better anonymity for users
   - Censorship resistance
   - Already supported via proxy

---

## References

- Bitcoin P2P Network: https://en.bitcoin.it/wiki/Network
- Eclipse Attacks: https://eprint.iacr.org/2015/263.pdf
- BIP37 (Bloom Filters): https://github.com/bitcoin/bips/blob/master/bip-0037.mediawiki
- BIP152 (Compact Blocks): https://github.com/bitcoin/bips/blob/master/bip-0152.mediawiki
- BIP324 (v2 P2P Transport): https://github.com/bitcoin/bips/blob/master/bip-0324.mediawiki

---

**Last Review:** 2025-12-24
**Next Review:** 2026-06-24 (or after major security incident)
