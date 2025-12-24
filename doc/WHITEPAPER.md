# Hylium: A Bitcoin-Derived Cryptocurrency for the Next Generation

**Version 0.1 - DRAFT**  
**December 2025**

---

## Abstract

Hylium is a new cryptocurrency built on the proven foundation of Bitcoin Core. By inheriting Bitcoin's battle-tested consensus mechanism and cryptographic primitives while launching with modern features enabled from genesis, Hylium aims to provide a secure, decentralized digital currency that learns from Bitcoin's 15+ years of operation.

This whitepaper describes Hylium's technical design, economic model, and distinguishing characteristics.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Technical Overview](#2-technical-overview)
3. [Network Parameters](#3-network-parameters)
4. [Consensus Mechanism](#4-consensus-mechanism)
5. [Monetary Policy](#5-monetary-policy)
6. [Security Model](#6-security-model)
7. [Roadmap](#7-roadmap)
8. [Conclusion](#8-conclusion)

---

## 1. Introduction

### 1.1 Background

Bitcoin, introduced in 2009 by Satoshi Nakamoto, demonstrated that decentralized digital scarcity is achievable through proof-of-work consensus. Since then, thousands of alternative cryptocurrencies have emerged, yet few have matched Bitcoin's security properties.

### 1.2 Motivation

Hylium is created with three core objectives:

1. **Security First**: Inherit Bitcoin's proven security while enabling modern improvements from day one
2. **Clean Slate**: Launch with all soft forks (SegWit, Taproot) active from genesis, avoiding legacy complexity
3. **Transparency**: No premine, fair launch, open development

### 1.3 Design Philosophy

> *"Verify, don't trust."* — Hylium Genesis Message

Hylium prioritizes:
- **Decentralization** over convenience
- **Security** over speed
- **Simplicity** over features
- **Transparency** over expediency

---

## 2. Technical Overview

### 2.1 Codebase Heritage

Hylium is derived from Bitcoin Core, the reference implementation of Bitcoin. This provides:

- 15+ years of peer-reviewed code
- Battle-tested cryptographic implementations
- Proven P2P networking layer
- Comprehensive test suites

### 2.2 Key Modifications

| Component | Bitcoin | Hylium |
|-----------|---------|--------|
| Network ID | Bitcoin mainnet | Hylium mainnet (unique) |
| P2P Port | 8333 | 9333 |
| RPC Port | 8332 | 9332 |
| Address Prefix | 1, 3, bc1 | K, N, hyl1 |
| Genesis Block | 2009-01-03 | 2025-12-16 |
| Soft Forks | Activated gradually | Active from genesis |

### 2.3 Feature Set

**Active from Genesis (Block 0):**
- Segregated Witness (BIP141/143/144)
- Taproot & Schnorr Signatures (BIP340/341/342)
- Check Lock Time Verify (BIP65)
- Check Sequence Verify (BIP68/112/113)
- Strict DER Encoding (BIP66)

---

## 3. Network Parameters

### 3.1 Chain Identifiers

```
Network:        Hylium Mainnet
Magic Bytes:    0xf3c2e1b0
P2P Port:       9333
RPC Port:       9332
```

### 3.2 Address Formats

| Type | Prefix | Example |
|------|--------|---------|
| P2PKH | K | K1abc... |
| P2SH | N | N2xyz... |
| Bech32 | hyl1 | hyl1qp... |
| Bech32m (Taproot) | hyl1p | hyl1p... |

### 3.3 Genesis Block

```
Timestamp:    2025-12-16 12:00:00 UTC
Message:      "Hylium 2025-12-16: Verify, don't trust."
nBits:        0x1d00ffff (Difficulty 1)
Output:       OP_RETURN (No premine)
```

The genesis coinbase uses OP_RETURN, making the initial 50 HYL **provably unspendable**. This demonstrates commitment to fair distribution.

---

## 4. Consensus Mechanism

### 4.1 Proof of Work

Hylium uses the same SHA-256d proof-of-work algorithm as Bitcoin:

```
block_hash = SHA256(SHA256(block_header))
Valid if: block_hash ≤ target
```

### 4.2 Difficulty Adjustment

| Parameter | Value |
|-----------|-------|
| Target Block Time | 10 minutes |
| Adjustment Period | 2016 blocks (~2 weeks) |
| Maximum Adjustment | 4x per period |

### 4.3 Block Limits

```
Maximum Block Weight:  4,000,000 WU
Maximum Block Size:    4,000,000 bytes
Maximum Sigops:        80,000
```

---

## 5. Monetary Policy

### 5.1 Supply Schedule

| Parameter | Value |
|-----------|-------|
| Maximum Supply | 21,000,000 HYL |
| Initial Block Reward | 50 HYL |
| Halving Interval | 210,000 blocks (~4 years) |
| Smallest Unit | 1 satoshi = 0.00000001 HYL |

### 5.2 Emission Curve

| Era | Block Range | Reward | Cumulative Supply |
|-----|-------------|--------|-------------------|
| 0 | 0-209,999 | 50 HYL | 10,500,000 HYL |
| 1 | 210,000-419,999 | 25 HYL | 15,750,000 HYL |
| 2 | 420,000-629,999 | 12.5 HYL | 18,375,000 HYL |
| 3 | 630,000-839,999 | 6.25 HYL | 19,687,500 HYL |
| ... | ... | ... | ... |
| 64+ | 13,230,000+ | 0 HYL | 21,000,000 HYL |

### 5.3 Fair Launch Commitment

- ✅ **No premine**: Genesis output is OP_RETURN (unspendable)
- ✅ **No instamine**: Standard difficulty from block 1
- ✅ **No ICO/IEO**: No pre-sale of any kind
- ✅ **Open source**: MIT license from day one

---

## 6. Security Model

### 6.1 Threat Model

Hylium inherits Bitcoin's threat model, defending against:

- **51% Attacks**: Mitigated by growing hashrate and decentralization
- **Eclipse Attacks**: Diverse peer selection, ASN-aware connections
- **Sybil Attacks**: Proof-of-work consensus, not identity-based
- **Double Spends**: 6-confirmation standard (~1 hour)

### 6.2 Cryptographic Primitives

| Function | Algorithm | Security Level |
|----------|-----------|----------------|
| Digital Signatures | ECDSA + Schnorr | secp256k1 |
| Hash Function | SHA-256, RIPEMD-160 | 128-bit |
| Address Encoding | Bech32/Bech32m | Error detection |

### 6.3 Security Practices

- Third-party security audits (pre-mainnet)
- Responsible disclosure policy
- 90-day coordinated disclosure timeline

---

## 7. Roadmap

### Phase 1: Foundation (Current)

- [x] Fork Bitcoin Core codebase
- [x] Customize chain parameters
- [x] Generate genesis blocks
- [x] Basic documentation

### Phase 2: Pre-Launch

- [ ] Security audit
- [ ] Seed node deployment
- [ ] Block explorer
- [ ] Wallet releases

### Phase 3: Mainnet Launch

- [ ] Genesis block mining
- [ ] Initial node distribution
- [ ] Community formation

### Phase 4: Growth

- [ ] Exchange listings
- [ ] Ecosystem development
- [ ] Protocol improvements

---

## 8. Conclusion

Hylium represents a fresh start built on proven fundamentals. By combining Bitcoin's battle-tested codebase with a clean launch that includes all modern improvements from genesis, Hylium offers a secure, transparent, and decentralized cryptocurrency for the next generation.

The project is open source and community-driven. We invite developers, miners, and users to join us in building a trustworthy digital currency.

---

## References

1. Nakamoto, S. (2008). "Bitcoin: A Peer-to-Peer Electronic Cash System"
2. BIP141: Segregated Witness
3. BIP340/341/342: Taproot
4. Bitcoin Core Documentation

---

## Appendix A: Technical Specifications

Detailed specifications available at:
- Consensus Rules: `/doc/consensus.md`
- P2P Security: `/doc/p2p-security.md`
- Build Instructions: `/doc/build-*.md`

---

**Disclaimer**: This whitepaper is for informational purposes only. Hylium is experimental software. Use at your own risk.

**License**: This document is released under CC-BY-4.0.

---

*Hylium Development Team*  
*December 2025*
