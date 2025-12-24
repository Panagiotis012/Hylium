# Hylium Consensus Rules Specification

**Version:** 1.0.0
**Last Updated:** 2025-12-24
**Status:** Normative Specification

This document defines the complete consensus rules for the Hylium cryptocurrency. Any implementation claiming Hylium compatibility MUST follow these rules exactly. Deviation from these rules will result in chain splits and network incompatibility.

---

## Table of Contents

1. [Block Structure and Validation](#1-block-structure-and-validation)
2. [Transaction Structure and Validation](#2-transaction-structure-and-validation)
3. [Proof of Work](#3-proof-of-work)
4. [Block Subsidy and Monetary Policy](#4-block-subsidy-and-monetary-policy)
5. [Script Validation and Signature Rules](#5-script-validation-and-signature-rules)
6. [Network Upgrades and Soft Forks](#6-network-upgrades-and-soft-forks)
7. [Genesis Blocks](#7-genesis-blocks)
8. [Timestamp Rules](#8-timestamp-rules)
9. [Reorganization Rules](#9-reorganization-rules)

---

## 1. Block Structure and Validation

### 1.1 Block Header

A valid block header MUST contain exactly 80 bytes with the following fields in order:

| Field | Size | Type | Description |
|-------|------|------|-------------|
| nVersion | 4 bytes | int32_t | Block version number |
| hashPrevBlock | 32 bytes | uint256 | SHA256d hash of previous block header |
| hashMerkleRoot | 32 bytes | uint256 | Merkle root of all transactions |
| nTime | 4 bytes | uint32_t | Unix timestamp (seconds since epoch) |
| nBits | 4 bytes | uint32_t | Compact representation of target difficulty |
| nNonce | 4 bytes | uint32_t | Proof-of-work nonce |

### 1.2 Block Size and Weight Limits

```cpp
MAX_BLOCK_SERIALIZED_SIZE = 4,000,000 bytes
MAX_BLOCK_WEIGHT = 4,000,000 weight units
MAX_BLOCK_SIGOPS_COST = 80,000 signature operations
WITNESS_SCALE_FACTOR = 4
```

**Block Weight Calculation:**
```
block_weight = (serialized_size_without_witness * (WITNESS_SCALE_FACTOR - 1))
             + serialized_size_with_witness
```

**Consensus Rule:** A block MUST satisfy:
- `block_weight <= MAX_BLOCK_WEIGHT`
- `serialized_size <= MAX_BLOCK_SERIALIZED_SIZE`
- Total signature operations cost `<= MAX_BLOCK_SIGOPS_COST`

### 1.3 Block Validation Rules

A block is valid if and only if ALL of the following conditions are met:

1. **Syntactic Validity:**
   - Block header is exactly 80 bytes
   - All transactions are syntactically valid (see §2)
   - Merkle root correctly represents all transactions
   - Block size/weight within limits

2. **Proof-of-Work Validity:**
   - `SHA256d(block_header) <= target` (see §3)
   - Target matches nBits compact representation

3. **Timestamp Validity:**
   - Block timestamp rules satisfied (see §8)

4. **Coinbase Validity:**
   - First transaction is coinbase (and only the first)
   - Coinbase subsidy correct for block height (see §4)
   - Coinbase scriptSig includes block height (BIP34)

5. **Transaction Validity:**
   - All non-coinbase transactions are valid
   - No duplicate transaction IDs in block
   - Total input value >= total output value for all transactions

6. **Chain Validity:**
   - `hashPrevBlock` references a known valid block
   - Difficulty target is correct for height (see §3.2)

---

## 2. Transaction Structure and Validation

### 2.1 Transaction Structure

**Legacy Transaction Format:**
```
nVersion (4 bytes)
Input Count (varint)
Inputs (variable)
Output Count (varint)
Outputs (variable)
nLockTime (4 bytes)
```

**SegWit Transaction Format (BIP141):**
```
nVersion (4 bytes)
Marker (1 byte, must be 0x00)
Flag (1 byte, must be 0x01)
Input Count (varint)
Inputs (variable)
Output Count (varint)
Outputs (variable)
Witness Data (variable)
nLockTime (4 bytes)
```

### 2.2 Transaction Validation Rules

A transaction is valid if ALL of the following hold:

1. **Size Constraints:**
   ```cpp
   MIN_TRANSACTION_WEIGHT = 240 weight units
   transaction_weight >= MIN_TRANSACTION_WEIGHT
   ```

2. **Input/Output Validity:**
   - At least one input (except coinbase)
   - At least one output
   - No output value > 21,000,000 HYL
   - Sum of output values <= sum of input values

3. **Coinbase Constraints:**
   - If coinbase: exactly one input with previous output = null (0xffffffff)
   - If not coinbase: no inputs with previous output = null
   - Coinbase outputs spendable only after `COINBASE_MATURITY = 100` blocks

4. **Double-Spend Prevention:**
   - No input references an already-spent output (within this block or chain)
   - No duplicate inputs within the transaction

5. **Script Validation:**
   - All input scripts validate successfully (see §5)
   - Signature operations within limits

6. **Locktime Rules:**
   - If nLockTime < 500,000,000: interpreted as block height
   - If nLockTime >= 500,000,000: interpreted as Unix timestamp
   - Transaction invalid if current block height/time < nLockTime (and not finalized)

---

## 3. Proof of Work

### 3.1 Hash Function

Hylium uses **double SHA-256** (SHA256d) for proof-of-work:

```
block_hash = SHA256(SHA256(block_header))
```

A block is valid only if:
```
block_hash <= target
```

where `target` is derived from the compact `nBits` representation.

### 3.2 Difficulty Adjustment

**Retarget Period:** Every 2016 blocks

**Algorithm:**
```
new_target = old_target * (actual_timespan / expected_timespan)

where:
  actual_timespan = timestamp(block_2015) - timestamp(block_0)
  expected_timespan = 2016 * 600 seconds = 1,209,600 seconds

Constraints:
  actual_timespan = clamp(actual_timespan, 302400, 4838400)  // 0.25x to 4x
```

**Initial Difficulty:**
- **Mainnet:** `nBits = 0x1d00ffff` (difficulty 1)
- **Testnet/Test4:** `nBits = 0x207fffff` (minimum difficulty)
- **Regtest:** `nBits = 0x207fffff` (minimum difficulty, no retarget)

### 3.3 Difficulty Encoding (Compact nBits)

```cpp
target = mantissa * 256^(exponent - 3)

where:
  exponent = (nBits >> 24) & 0xff
  mantissa = nBits & 0x00ffffff
```

---

## 4. Block Subsidy and Monetary Policy

### 4.1 Supply Parameters

```
Total Supply: 21,000,000 HYL
Initial Block Reward: 50 HYL
Halving Interval: 210,000 blocks (~4 years at 10 min/block)
Smallest Unit: 1 satoshi = 0.00000001 HYL
```

### 4.2 Subsidy Calculation

```cpp
int64_t GetBlockSubsidy(int nHeight, const Consensus::Params& params)
{
    int halvings = nHeight / params.nSubsidyHalvingInterval;  // 210,000

    // Subsidy goes to zero after 64 halvings
    if (halvings >= 64)
        return 0;

    int64_t nSubsidy = 50 * COIN;  // COIN = 100,000,000 satoshis
    nSubsidy >>= halvings;  // Divide by 2^halvings
    return nSubsidy;
}
```

**Halving Schedule:**
| Blocks | Subsidy | Era |
|--------|---------|-----|
| 0 - 209,999 | 50 HYL | 0 |
| 210,000 - 419,999 | 25 HYL | 1 |
| 420,000 - 629,999 | 12.5 HYL | 2 |
| 630,000 - 839,999 | 6.25 HYL | 3 |
| ... | ... | ... |
| 13,230,000+ | 0 HYL | 64+ |

### 4.3 Transaction Fees

Miners receive:
```
block_reward = block_subsidy + sum(transaction_fees)

where:
  transaction_fee = sum(input_values) - sum(output_values)
```

**No maximum fee limit.** Users must verify fees before signing.

---

## 5. Script Validation and Signature Rules

### 5.1 Script System

Hylium inherits Bitcoin's script system with the following active features:

- **P2PKH** (Pay-to-Public-Key-Hash)
- **P2SH** (Pay-to-Script-Hash) - BIP16
- **P2WPKH** (Pay-to-Witness-Public-Key-Hash) - BIP141
- **P2WSH** (Pay-to-Witness-Script-Hash) - BIP141
- **P2TR** (Pay-to-Taproot) - BIP341/342

### 5.2 Signature Hash Types

```cpp
SIGHASH_ALL = 0x01           // Sign all inputs and outputs
SIGHASH_NONE = 0x02          // Sign inputs only
SIGHASH_SINGLE = 0x03        // Sign inputs + corresponding output
SIGHASH_ANYONECANPAY = 0x80  // Modifier: sign only this input
```

### 5.3 Signature Validation

**ECDSA Signatures (pre-Taproot):**
- Curve: secp256k1
- Hash: SHA256
- Encoding: DER format (BIP66 strict DER)

**Schnorr Signatures (Taproot):**
- Curve: secp256k1
- Hash: Tagged SHA256
- Encoding: 64-byte format (BIP340)

### 5.4 Script Verification Flags

All blocks MUST validate with these flags:
```cpp
SCRIPT_VERIFY_P2SH           // BIP16
SCRIPT_VERIFY_DERSIG         // BIP66
SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY  // BIP65
SCRIPT_VERIFY_CHECKSEQUENCEVERIFY  // BIP112
SCRIPT_VERIFY_WITNESS        // BIP141/143
SCRIPT_VERIFY_TAPROOT        // BIP341/342
```

---

## 6. Network Upgrades and Soft Forks

### 6.1 Buried Deployments (Always Active)

These are activated at block height 0 on mainnet:

| Feature | BIP | Activation | Description |
|---------|-----|------------|-------------|
| Height in Coinbase | BIP34 | Block 0 | Block height in coinbase |
| Strict DER Signatures | BIP66 | Block 0 | Canonical signature encoding |
| CHECKLOCKTIMEVERIFY | BIP65 | Block 0 | Timelocks |
| CSV (Relative timelocks) | BIP112/113 | Block 0 | Sequence timelocks |
| Segregated Witness | BIP141/143/144 | Block 0 | Witness data separation |

### 6.2 BIP9 Version Bits Deployments

**Taproot (BIP340/341/342):**
- Bit: 2
- Start Time: Always active on mainnet
- Timeout: Never
- Threshold: 1916/2016 blocks (95%)

### 6.3 Upgrade Policy

- **Hard forks:** PROHIBITED without overwhelming consensus
- **Soft forks:** Via BIP9 version bits signaling
- **Emergency fixes:** Security-critical only, maximum compatibility

---

## 7. Genesis Blocks

### 7.1 Mainnet Genesis

```
nTime = 1765886400 (2025-12-16 12:00:00 UTC)
nBits = 0x1d00ffff
nNonce = 3320618862
hashGenesisBlock = 00000000201ce299e7706bcf4234f23816e875a46fb3bb62789460bc18de4d7c
hashMerkleRoot = 44ff3d395d53adbf7437ec960b6440aca1c6731cdd98d6d9e8349d6fe1b3c90d
Message: "Hylium 2025-12-16: Verify, don't trust."
```

**Coinbase Transaction:**
- Output: OP_RETURN (provably unspendable - NO PREMINE)
- Script: `OP_RETURN <message>`

### 7.2 Testnet Genesis

```
nTime = 1765886400
nBits = 0x207fffff
nNonce = 1
hashGenesisBlock = 6e209d4f00bf79b553dc53a2916d1858e456c9deeefa59e39b2e6cf005f1ab20
Message: "Hylium testnet 2025-12-16: break safely."
```

### 7.3 Genesis Determinism

The genesis block MUST be reproducible from:
1. Message string (embedded in coinbase)
2. Target difficulty (nBits)
3. Timestamp (nTime)
4. Proof-of-work search starting from nNonce=0

See `contrib/devtools/genesis_hylium.cpp` for reference implementation.

---

## 8. Timestamp Rules

### 8.1 Block Timestamp Validation

A block's timestamp MUST satisfy:

1. **Median Time Past (MTP) Rule:**
   ```
   block.nTime > MedianTimePast(prev_11_blocks)
   ```

2. **Future Time Limit:**
   ```
   block.nTime <= current_adjusted_time + 2 hours
   ```

3. **Difficulty Adjustment:**
   - Only MTP is used for difficulty calculation (prevents time-warp attacks)

### 8.2 Median Time Past Calculation

```cpp
int64_t GetMedianTimePast(CBlockIndex* pindex)
{
    int64_t pmedian[11];
    int64_t* pbegin = &pmedian[11];
    int64_t* pend = &pmedian[11];

    for (int i = 0; i < 11 && pindex; i++, pindex = pindex->pprev)
        *(--pbegin) = pindex->GetBlockTime();

    std::sort(pbegin, pend);
    return pbegin[(pend - pbegin)/2];
}
```

---

## 9. Reorganization Rules

### 9.1 Chain Selection

The valid chain with the **most accumulated proof-of-work** is canonical.

**NOT** the longest chain by block count.

```cpp
chain_work = sum(2^256 / (target + 1)) for all blocks
```

### 9.2 Reorg Limits

**No hard reorg depth limit** - always follow most-work chain.

However:
- Checkpoints MAY be used for performance (not consensus)
- AssumeValid MAY skip signature checks for old blocks (not consensus)

### 9.3 Reorg Safety

Implementations SHOULD:
- Require 6 confirmations (1 hour) for high-value transactions
- Monitor for reorgs > 2 blocks (potential attack)
- Alert on reorgs > 6 blocks (serious attack)

---

## 10. Mempool and Relay Policy

**NOTE:** Mempool rules are **NOT consensus rules**. Different nodes MAY have different policies.

### 10.1 Standard Transaction Rules (Relay Policy)

Common relay policies (not enforced by consensus):
- Minimum fee rate: 1 sat/vByte (default)
- Maximum transaction size: 400,000 weight units
- Standard script types only (P2PKH, P2SH, P2WPKH, P2WSH, P2TR)
- No bare multisig or non-standard scripts in outputs

### 10.2 Replace-By-Fee (RBF)

- BIP125 opt-in RBF supported
- Transaction signals replaceability if any input has nSequence < 0xfffffffe

---

## Appendix A: Constants Reference

```cpp
// Monetary
COIN = 100,000,000 satoshis
MAX_MONEY = 21,000,000 * COIN
SUBSIDY_HALVING_INTERVAL = 210,000 blocks

// Block Limits
MAX_BLOCK_WEIGHT = 4,000,000
MAX_BLOCK_SERIALIZED_SIZE = 4,000,000
MAX_BLOCK_SIGOPS_COST = 80,000
COINBASE_MATURITY = 100 blocks

// Transaction
MIN_TRANSACTION_WEIGHT = 240
WITNESS_SCALE_FACTOR = 4

// Difficulty
POW_TARGET_TIMESPAN = 14 days = 1,209,600 seconds
POW_TARGET_SPACING = 10 minutes = 600 seconds
DIFFICULTY_ADJUSTMENT_INTERVAL = POW_TARGET_TIMESPAN / POW_TARGET_SPACING = 2016 blocks

// Timestamp
MAX_FUTURE_BLOCK_TIME = 2 hours = 7200 seconds
MEDIAN_TIME_SPAN = 11 blocks
```

---

## Appendix B: Chain Parameters by Network

| Parameter | Mainnet | Testnet | Regtest |
|-----------|---------|---------|---------|
| Address Prefix (HRP) | hyl | thyl | hylrt |
| Default P2P Port | 8333 | 18333 | 18444 |
| Default RPC Port | 8332 | 18332 | 18443 |
| Genesis Timestamp | 1765886400 | 1765886400 | 1296688602 |
| Genesis nBits | 0x1d00ffff | 0x207fffff | 0x207fffff |
| Difficulty Retarget | Yes (2016 blocks) | Yes (2016 blocks) | No |

---

## Document History

- **v1.0.0** (2025-12-24): Initial consensus specification

---

## Normative References

- BIP16: Pay to Script Hash (P2SH)
- BIP34: Block Height in Coinbase
- BIP65: CHECKLOCKTIMEVERIFY
- BIP66: Strict DER Signatures
- BIP68/112/113: Relative Timelocks
- BIP141/143/144: Segregated Witness
- BIP340/341/342: Taproot/Schnorr

---

**CRITICAL:** This specification is the authoritative definition of Hylium consensus. Any ambiguity or conflict with code MUST be resolved in favor of this document for future implementations. The reference implementation (Hylium Core) is considered canonical for unspecified edge cases until this document is updated.
