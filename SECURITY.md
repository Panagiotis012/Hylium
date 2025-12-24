# Security Policy

## Supported Versions

| Version | Supported          | Notes |
| ------- | ------------------ | ----- |
| 0.x.x   | :white_check_mark: | Development/Testnet phase |

> **Note**: Hylium is currently in testnet/development phase. Full security support and SLA will be established once mainnet launches.

## Reporting a Vulnerability

**Please report security issues responsibly.**

### Contact Information

| Method | Details |
|--------|---------|
| Email | security@hylium.org |
| Response Time | Within 48 hours |
| PGP Encryption | Strongly recommended |

### PGP Keys

The following keys may be used to communicate sensitive information to developers:

| Name | Fingerprint | Role |
|------|-------------|------|
| Hylium Security Team | `[TO BE ADDED BEFORE MAINNET]` | Primary contact |
| Lead Maintainer | `[TO BE ADDED BEFORE MAINNET]` | Secondary contact |
| Release Manager | `[TO BE ADDED BEFORE MAINNET]` | Release signing |

Import keys from a keyserver:
```bash
gpg --keyserver hkps://keys.openpgp.org --recv-keys "<fingerprint>"
```

> [!IMPORTANT]
> **Before mainnet launch**, all placeholder fingerprints must be replaced with actual PGP keys.
> Generate keys using: `gpg --full-generate-key` (RSA 4096-bit recommended)

## Responsible Disclosure Policy

We follow a **90-day disclosure policy**:

1. **Day 0**: Vulnerability reported to security@hylium.org
2. **Day 1-2**: Initial acknowledgment and triage
3. **Day 3-30**: Investigation and fix development
4. **Day 31-60**: Testing and coordination with reporter
5. **Day 61-90**: Coordinated disclosure (if applicable)

### We Request That Researchers:

- ✅ Give us reasonable time (90 days) to investigate and fix issues before public disclosure
- ✅ Avoid privacy violations, data destruction, or service interruption
- ✅ Do not exploit discovered vulnerabilities beyond proof-of-concept
- ✅ Provide sufficient detail to reproduce the issue
- ❌ Do not publicly disclose before coordinated disclosure date

## Vulnerability Classifications

| Severity | Description | Example |
|----------|-------------|---------|
| **Critical** | Network-wide impact, consensus failure, fund theft | Double-spend, remote code execution |
| **High** | Significant impact, node crashes, DoS | Memory corruption, P2P exploits |
| **Medium** | Limited impact, requires specific conditions | Privacy leaks, non-standard tx handling |
| **Low** | Minor issues, theoretical attacks | Timing side-channels, minor information disclosure |

## Security Audit Status

| Audit Type | Status | Date | Auditor |
|------------|--------|------|---------|
| Full Consensus Audit | 🔴 Not Started | - | - |
| Cryptographic Review | 🔴 Not Started | - | - |
| P2P Network Audit | 🔴 Not Started | - | - |

> [!CAUTION]
> **Pre-mainnet**: No formal third-party security audit has been conducted yet.
> A comprehensive security audit is **required** before mainnet launch.
> 
> Recommended auditors: NCC Group, Trail of Bits, Cure53, Halborn


## Security Best Practices for Node Operators

### Mandatory
- [ ] Keep Hylium Core updated to latest version
- [ ] Use firewall to restrict RPC access (`rpcallowip`)
- [ ] Enable RPC authentication (`rpcuser`/`rpcpassword`)
- [ ] Run on dedicated hardware or isolated VM

### Recommended
- [ ] Use Tor for privacy (`-onion=127.0.0.1:9050`)
- [ ] Enable encrypted P2P (when available)
- [ ] Monitor logs for suspicious activity
- [ ] Regular backups of wallet.dat

## Incident Response

In case of a critical security incident:

1. **Immediate**: Core team assesses impact
2. **1-4 hours**: Emergency patches prepared (if needed)
3. **4-24 hours**: Coordinated disclosure to major stakeholders
4. **24-72 hours**: Public advisory and patch release
5. **Post-incident**: Root cause analysis and lessons learned

## Contact

- **Security Issues**: security@hylium.org
- **General Questions**: Use GitHub Discussions
- **NOT for Support**: Do not email security@ for general help

---

**Last Updated**: 2025-12-24  
**Next Review**: Before Mainnet Launch
