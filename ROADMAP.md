# Hylium Roadmap

This document outlines the planned development milestones for Hylium.

**Last Updated**: 2025-12-24  
**Status**: Active Development

---

## Overview

```
[Phase 1: Foundation] → [Phase 2: Pre-Launch] → [Phase 3: Mainnet] → [Phase 4: Growth]
      (Current)
```

---

## Phase 1: Foundation ✅

**Status**: Complete

| Milestone | Status | Description |
|-----------|--------|-------------|
| Fork Bitcoin Core | ✅ Done | Based on latest stable release |
| Chain Parameters | ✅ Done | Unique ports, addresses, genesis |
| Testnet | ✅ Done | Testnet3/Testnet4/Regtest operational |
| Basic Documentation | ✅ Done | Build guides, developer notes |
| Consensus Spec | ✅ Done | `doc/consensus.md` |
| P2P Security Spec | ✅ Done | `doc/p2p-security.md` |

---

## Phase 2: Pre-Launch 🔄

**Status**: In Progress  
**Target**: Q1 2026

### Security

| Milestone | Status | Target Date | Notes |
|-----------|--------|-------------|-------|
| Security Policy | ✅ Done | - | SECURITY.md updated |
| PGP Key Setup | 🔴 TODO | Jan 2026 | Maintainer keys needed |
| Security Audit | 🔴 TODO | Feb 2026 | Select auditor |
| Audit Remediation | 🔴 TODO | Mar 2026 | Fix identified issues |

### Infrastructure

| Milestone | Status | Target Date | Notes |
|-----------|--------|-------------|-------|
| Seed Node 1 (EU) | 🔴 TODO | Jan 2026 | Germany/Netherlands |
| Seed Node 2 (US) | 🔴 TODO | Jan 2026 | East/West coast |
| Seed Node 3 (Asia) | 🔴 TODO | Jan 2026 | Singapore/Japan |
| DNS Seeds | 🔴 TODO | Jan 2026 | seed.hylium.org |
| Block Explorer | 🔴 TODO | Feb 2026 | Self-hosted or partner |

### Documentation

| Milestone | Status | Target Date |
|-----------|--------|-------------|
| Whitepaper | ✅ Done | - |
| User Guide | 🔴 TODO | Feb 2026 |
| API Documentation | 🔴 TODO | Feb 2026 |
| Mining Guide | 🔴 TODO | Feb 2026 |

### Community

| Milestone | Status | Target Date |
|-----------|--------|-------------|
| Website | 🔴 TODO | Jan 2026 |
| Discord Server | 🔴 TODO | Jan 2026 |
| Twitter/X Account | 🔴 TODO | Jan 2026 |
| GitHub Organization | 🔴 TODO | Jan 2026 |

---

## Phase 3: Mainnet Launch 📅

**Status**: Planned  
**Target**: Q2 2026

### Launch Checklist

- [ ] All Phase 2 items complete
- [ ] Final testnet stress testing
- [ ] Release candidate builds
- [ ] GPG-signed releases
- [ ] Launch announcement
- [ ] Genesis block mining

### Launch Day Operations

1. **T-7 days**: Final release candidate
2. **T-3 days**: GPG-signed binaries published
3. **T-1 day**: Final go/no-go decision
4. **T-0**: Genesis block mined
5. **T+1 hour**: Confirm network stability
6. **T+24 hours**: Public announcement

---

## Phase 4: Growth 🌱

**Status**: Future  
**Target**: Q3 2026+

### Ecosystem Development

| Milestone | Priority | Target |
|-----------|----------|--------|
| Exchange Listing (Tier 2) | High | Q3 2026 |
| Hardware Wallet Support | High | Q3 2026 |
| Mobile Wallet | Medium | Q4 2026 |
| Payment Integrations | Medium | Q4 2026 |
| Exchange Listing (Tier 1) | High | 2027 |

### Protocol Improvements

| Feature | BIP | Priority | Target |
|---------|-----|----------|--------|
| BIP324 v2 Transport | 324 | High | v0.2.0 |
| ASMAP Peer Selection | - | Medium | v0.2.0 |
| Dandelion++ | - | Low | v0.3.0 |
| Silent Payments | 352 | Low | v0.4.0 |

---

## Version History

| Version | Target | Major Features |
|---------|--------|----------------|
| v0.1.0 | Q2 2026 | Mainnet launch |
| v0.2.0 | Q3 2026 | BIP324, ASMAP |
| v0.3.0 | Q4 2026 | Privacy improvements |
| v0.4.0 | 2027 | Silent Payments |

---

## Contributing

We welcome contributions to accelerate the roadmap:

- **Code**: See `CONTRIBUTING.md`
- **Testing**: Join testnet
- **Documentation**: Improve guides
- **Translation**: Help localize

---

## Disclaimer

This roadmap is subject to change based on:
- Security audit findings
- Community feedback  
- Technical discoveries
- Resource availability

No dates are guaranteed. Security always takes priority over schedule.

---

*Maintained by Hylium Development Team*
