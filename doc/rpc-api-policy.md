# Hylium RPC API Versioning and Stability Policy

**Version:** 1.0
**Last Updated:** 2025-12-24

This document defines the RPC API compatibility guarantees, versioning scheme, and deprecation policy for Hylium Core.

---

## Table of Contents

1. [Overview](#overview)
2. [API Versioning Scheme](#api-versioning-scheme)
3. [Stability Guarantees](#stability-guarantees)
4. [Deprecation Policy](#deprecation-policy)
5. [Core RPC Methods](#core-rpc-methods)
6. [Response Format Standards](#response-format-standards)
7. [Error Codes](#error-codes)
8. [Best Practices for Clients](#best-practices-for-clients)

---

## Overview

The Hylium RPC API provides programmatic access to node functionality for wallets, explorers, and other infrastructure. To enable a healthy ecosystem, we commit to:

1. **Stability**: No breaking changes without deprecation cycle
2. **Versioning**: Clear version indicators for all endpoints
3. **Documentation**: Comprehensive and up-to-date API docs
4. **Backwards Compatibility**: Support old clients when reasonable

### API Types

Hylium exposes two types of APIs:

1. **JSON-RPC API** (primary): Traditional HTTP POST endpoint
2. **REST API** (secondary): Lightweight HTTP GET for specific queries

---

## API Versioning Scheme

### Version Format

RPC methods are versioned using the node's software version:

```json
{
  "version": 10000,       // 0.1.0 = 0*10000 + 1*100 + 0
  "protocolversion": 70016,
  "walletversion": 169900
}
```

### Version Compatibility Matrix

| Hylium Version | RPC API Version | Compatible Clients |
|----------------|-----------------|-------------------|
| 0.1.x | 1.0 | All 0.1.x clients |
| 0.2.x | 1.1 | All 0.1.x, 0.2.x clients |
| 1.x.x | 2.0 | Only 1.x.x clients (major break) |

**Rule:** MINOR version bumps (0.1 → 0.2) MUST maintain backward compatibility.

---

## Stability Guarantees

### Stable RPCs (Tier 1)

These methods are **guaranteed stable** and will not change without deprecation:

#### Blockchain Query
- `getblockchaininfo`
- `getbestblockhash`
- `getblock <blockhash> [verbosity]`
- `getblockcount`
- `getblockhash <height>`
- `getblockheader <blockhash> [verbose]`
- `getchaintips`
- `getdifficulty`
- `getmempoolinfo`
- `getrawmempool [verbose]`
- `gettxout <txid> <n> [include_mempool]`
- `verifytxoutproof <proof>`

#### Transaction Creation/Broadcast
- `getrawtransaction <txid> [verbose] [blockhash]`
- `sendrawtransaction <hex> [maxfeerate]`
- `testmempoolaccept <rawtxs> [maxfeerate]`
- `decoderawtransaction <hex>`
- `decodescript <hex>`

#### Wallet Operations (if wallet enabled)
- `getbalance`
- `getnewaddress [label] [address_type]`
- `sendtoaddress <address> <amount>`
- `listunspent [minconf] [maxconf] [addresses] [include_unsafe] [query_options]`
- `getwalletinfo`
- `dumpprivkey <address>`
- `importprivkey <privkey> [label] [rescan]`

#### Network Information
- `getnetworkinfo`
- `getpeerinfo`
- `getconnectioncount`
- `addnode <node> <command>`
- `disconnectnode <node>`

#### Control/Debug
- `help [command]`
- `stop`
- `uptime`
- `getmemoryinfo`

**Guarantee:** These methods will maintain:
- Same method name
- Same required parameters (new optional params may be added at end)
- Same response structure (new fields may be added, old fields never removed)
- Same error codes for documented error conditions

### Experimental RPCs (Tier 2)

These methods are **not yet stable** and may change:

- `estimatesmartfee` (may change algorithm/params)
- `scantxoutset` (heavy operation, may be redesigned)
- `getzmqnotifications` (ZMQ interface still evolving)

**Warning:** Experimental methods may have breaking changes in MINOR releases.

### Hidden RPCs (Tier 3)

Methods not listed in `help` output are **internal** and may disappear:

- Used only by automated tests
- No stability guarantee
- Use at own risk

---

## Deprecation Policy

### Deprecation Lifecycle

```
Phase 1: Mark Deprecated → Phase 2: Warning → Phase 3: Removal
  (Version N)           (Version N+1)        (Version N+2)
   6 months               6 months             Final
```

**Example:**
1. **v0.2.0** (Jan 2026): `oldmethod` marked deprecated in docs, still works
2. **v0.3.0** (Jul 2026): `oldmethod` logs warning, still works
3. **v0.4.0** (Jan 2027): `oldmethod` removed, returns error

### Deprecation Notice Format

```json
{
  "result": {...},
  "error": null,
  "id": 1,
  "warnings": [
    "RPC method 'oldmethod' is deprecated and will be removed in v0.4.0. Use 'newmethod' instead."
  ]
}
```

### Named Parameters Transition

**Old Style (positional):**
```bash
hylium-cli sendtoaddress "hyl1qxyz..." 1.5 "comment" "comment-to" false true 1 unset
```

**New Style (named):**
```bash
hylium-cli -named sendtoaddress address="hyl1qxyz..." amount=1.5 fee_rate=1
```

**Policy:** Always accept both, prefer named in documentation.

---

## Core RPC Methods

### Required for Wallet Integration

Minimum set for basic wallet functionality:

```bash
# Address Management
getnewaddress
getaddressinfo
validateaddress

# Balance/UTXOs
getbalance
listunspent

# Transaction Building
createrawtransaction
signrawtransactionwithwallet
sendrawtransaction

# Transaction History
listtransactions
gettransaction

# Block/Chain Info
getblockcount
getbestblockhash
getblock
```

### Required for Block Explorer

Minimum set for blockchain explorer:

```bash
# Chain Info
getblockchaininfo
getblockcount
getbestblockhash
getdifficulty

# Blocks
getblock
getblockhash
getblockheader

# Transactions
getrawtransaction
decoderawtransaction
gettxout

# Mempool
getmempoolinfo
getrawmempool
getmempoolentry

# Network
getnetworkinfo
```

---

## Response Format Standards

### Standard Success Response

```json
{
  "result": {
    // Method-specific data
  },
  "error": null,
  "id": "request-id"
}
```

### Standard Error Response

```json
{
  "result": null,
  "error": {
    "code": -5,
    "message": "Block not found"
  },
  "id": "request-id"
}
```

### Field Naming Conventions

- **Lowercase with underscores**: `block_height`, `tx_count`
- **Consistent units**:
  - Amounts in HYL (decimal): `1.50000000`
  - Time in Unix timestamp (seconds): `1765886400`
  - Hash as hex string: `"00000000201ce299..."`
- **Boolean as true/false**: `"confirmations": 0` vs `"confirmed": true`

### Optional Fields

Optional fields MUST be:
- Documented as optional
- Omitted (not `null`) when not applicable
- Consistent across calls

**Good:**
```json
{"balance": 1.5, "unconfirmed_balance": 0.1}  // Has unconfirmed
{"balance": 1.5}                               // No unconfirmed
```

**Bad:**
```json
{"balance": 1.5, "unconfirmed_balance": null}  // Don't use null
```

---

## Error Codes

### Standard Error Code Ranges

| Range | Category | Description |
|-------|----------|-------------|
| -1 to -9 | General | Miscellaneous errors |
| -10 to -19 | P2P/Network | Network-related errors |
| -20 to -29 | Blockchain | Blockchain query errors |
| -30 to -39 | Mempool | Mempool/relay errors |
| -40 to -49 | Wallet | Wallet-related errors |
| -50 to -99 | RPC Server | RPC server errors |

### Core Error Codes

```cpp
// General
RPC_MISC_ERROR = -1                    // Generic error
RPC_TYPE_ERROR = -3                    // Invalid type
RPC_INVALID_PARAMETER = -8             // Invalid parameter value

// Blockchain (-5)
RPC_INVALID_ADDRESS_OR_KEY = -5        // Invalid address
RPC_OUT_OF_MEMORY = -7                 // Out of memory
RPC_BLOCK_NOT_FOUND = -5               // Block not found
RPC_TRANSACTION_NOT_FOUND = -5         // Transaction not found

// Mempool/Relay (-26)
RPC_VERIFY_REJECTED = -26              // Transaction rejected (e.g., fee too low)
RPC_VERIFY_ALREADY_IN_CHAIN = -27      // Already in blockchain

// Wallet (-4, -6, -11 to -18)
RPC_WALLET_ERROR = -4                  // Generic wallet error
RPC_WALLET_INSUFFICIENT_FUNDS = -6     // Not enough funds
RPC_WALLET_INVALID_LABEL_NAME = -11    // Invalid label
RPC_WALLET_KEYPOOL_RAN_OUT = -12       // Keypool ran out
RPC_WALLET_UNLOCK_NEEDED = -13         // Wallet locked
RPC_WALLET_PASSPHRASE_INCORRECT = -14  // Wrong passphrase
RPC_WALLET_WRONG_ENC_STATE = -15       // Command needs unlocked wallet
RPC_WALLET_ENCRYPTION_FAILED = -16     // Encryption failed
RPC_WALLET_ALREADY_UNLOCKED = -17      // Already unlocked
RPC_WALLET_NOT_FOUND = -18             // Wallet file not found

// RPC Server (-32xxx = JSON-RPC standard)
RPC_INVALID_REQUEST = -32600           // Malformed JSON-RPC
RPC_METHOD_NOT_FOUND = -32601          // Method doesn't exist
RPC_INVALID_PARAMS = -32602            // Invalid params
RPC_INTERNAL_ERROR = -32603            // Internal JSON-RPC error
RPC_PARSE_ERROR = -32700               // Parse error
```

**Policy:** Error codes are **stable** - never reuse deleted codes.

---

## Best Practices for Clients

### 1. Version Detection

Always check version on startup:

```python
info = rpc.getnetworkinfo()
if info['version'] < 10100:  # Require 0.1.1+
    raise Exception("Hylium Core 0.1.1+ required")
```

### 2. Graceful Degradation

Handle missing methods:

```python
try:
    result = rpc.newmethod()
except JSONRPCException as e:
    if e.error['code'] == -32601:  # Method not found
        result = rpc.oldmethod()  # Fallback
    else:
        raise
```

### 3. Pagination for Large Results

For methods like `listunspent`, `listtransactions`:

```bash
# Get first 100
hylium-cli listtransactions "*" 100 0

# Get next 100
hylium-cli listtransactions "*" 100 100
```

### 4. Use Named Parameters

More readable and forward-compatible:

```bash
# Good
hylium-cli -named sendtoaddress address="hyl1q..." amount=1.5 fee_rate=2

# Avoid
hylium-cli sendtoaddress "hyl1q..." 1.5 "" "" false true 2
```

### 5. Handle Warnings

Check for deprecation warnings:

```python
response = rpc.somemethod()
if 'warnings' in response:
    for warning in response['warnings']:
        log.warning(f"RPC warning: {warning}")
```

### 6. Timeout Configuration

Set appropriate timeouts:

```python
rpc = AuthServiceProxy(
    service_url="http://user:pass@localhost:8332",
    timeout=120  # 2 minutes for heavy operations
)
```

### 7. Error Handling

Distinguish between different error classes:

```python
try:
    result = rpc.getrawtransaction(txid, True)
except JSONRPCException as e:
    if e.error['code'] == -5:  # Not found
        return None  # Normal case
    elif e.error['code'] == -28:  # Still warming up
        time.sleep(5)
        return rpc.getrawtransaction(txid, True)  # Retry
    else:
        raise  # Unexpected error
```

---

## REST API

### Endpoints

```
GET /rest/tx/<txid>.<format>
GET /rest/block/<blockhash>.<format>
GET /rest/blockheadersbyhash/<blockhash>.<format>
GET /rest/blockheadersbyheight/<height>.<format>
GET /rest/chaininfo.<format>
GET /rest/mempool/info.<format>
GET /rest/mempool/contents.<format>
```

**Formats:** `json`, `bin`, `hex`

### Example

```bash
curl http://localhost:8332/rest/block/00000000201ce299.json
```

**Guarantee:** REST endpoints follow same stability guarantees as RPC.

---

## Versioning for Major Upgrades

### API Version 2.0 (Future, Breaking Changes)

If we ever need to break compatibility:

1. **Run both versions concurrently:**
   ```
   /v1/rpc  (old API, deprecated)
   /v2/rpc  (new API)
   ```

2. **Deprecation timeline: 12 months minimum**

3. **Clear migration guide:**
   - Side-by-side examples
   - Automated migration tools
   - Comprehensive changelog

**Commitment:** We will avoid v2.0 unless absolutely necessary.

---

## Documentation

### Official RPC Documentation

```bash
# List all methods
hylium-cli help

# Get method details
hylium-cli help <method>
```

### Online Documentation

- Full API reference: https://hylium.org/api/
- Interactive playground: https://hylium.org/api/playground
- Swagger/OpenAPI spec: https://github.com/hylium/hylium/blob/main/openapi.yaml

---

## Monitoring API Changes

Clients should monitor:

1. **CHANGELOG.md**: All API changes documented
2. **Release notes**: Highlight breaking changes
3. **Mailing list**: Advance notice of deprecations
4. **GitHub labels**: Issues tagged `api-change`, `deprecation`

---

## Feedback and Requests

API change requests:

1. Open GitHub issue with label `api-enhancement`
2. Provide use case and proposed interface
3. Community discussion (minimum 1 month)
4. Core dev approval
5. Implementation and testing
6. Documentation update

---

## Appendix: Full Method List (v0.1.0)

### Blockchain
`getbestblockhash`, `getblock`, `getblockchaininfo`, `getblockcount`, `getblockfilter`, `getblockhash`, `getblockheader`, `getblockstats`, `getchaintips`, `getchaintxstats`, `getdifficulty`, `getmempoolancestors`, `getmempooldescendants`, `getmempoolentry`, `getmempoolinfo`, `getrawmempool`, `gettxout`, `gettxoutproof`, `gettxoutsetinfo`, `preciousblock`, `pruneblockchain`, `savemempool`, `scantxoutset`, `verifychain`, `verifytxoutproof`

### Control
`getmemoryinfo`, `getrpcinfo`, `help`, `logging`, `stop`, `uptime`

### Mining
`getblocktemplate`, `getmininginfo`, `getnetworkhashps`, `prioritisetransaction`, `submitblock`, `submitheader`

### Network
`addnode`, `clearbanned`, `disconnectnode`, `getaddednodeinfo`, `getconnectioncount`, `getnettotals`, `getnetworkinfo`, `getnodeaddresses`, `getpeerinfo`, `listbanned`, `ping`, `setban`, `setnetworkactive`

### Rawtransactions
`analyzepsbt`, `combinepsbt`, `combinerawtransaction`, `converttopsbt`, `createpsbt`, `createrawtransaction`, `decodepsbt`, `decoderawtransaction`, `decodescript`, `finalizepsbt`, `fundrawtransaction`, `getrawtransaction`, `joinpsbts`, `sendrawtransaction`, `signrawtransactionwithkey`, `testmempoolaccept`, `utxoupdatepsbt`

### Util
`createmultisig`, `deriveaddresses`, `estimatesmartfee`, `getdescriptorinfo`, `getindexinfo`, `signmessagewithprivkey`, `validateaddress`, `verifymessage`

### Wallet (if enabled)
`abandontransaction`, `abortrescan`, `addmultisigaddress`, `backupwallet`, `bumpfee`, `createwallet`, `dumpprivkey`, `dumpwallet`, `encryptwallet`, `getaddressesbylabel`, `getaddressinfo`, `getbalance`, `getbalances`, `getnewaddress`, `getrawchangeaddress`, `getreceivedbyaddress`, `getreceivedbylabel`, `gettransaction`, `getunconfirmedbalance`, `getwalletinfo`, `importaddress`, `importdescriptors`, `importmulti`, `importprivkey`, `importprunedfunds`, `importpubkey`, `importwallet`, `keypoolrefill`, `listaddressgroupings`, `listlabels`, `listlockunspent`, `listreceivedbyaddress`, `listreceivedbylabel`, `listsinceblock`, `listtransactions`, `listunspent`, `listwalletdir`, `listwallets`, `loadwallet`, `lockunspent`, `psbtbumpfee`, `removeprunedfunds`, `rescanblockchain`, `send`, `sendmany`, `sendtoaddress`, `sethdseed`, `setlabel`, `settxfee`, `setwalletflag`, `signmessage`, `signrawtransactionwithwallet`, `unloadwallet`, `upgradewallet`, `walletcreatefundedpsbt`, `walletdisplayaddress`, `walletlock`, `walletpassphrase`, `walletpassphrasechange`, `walletprocesspsbt`

### ZMQ
`getzmqnotifications`

---

**Last Review:** 2025-12-24
**Next Review:** 2026-01-24 (monthly for first year)
