# Hylium Changelog

All notable changes to the Hylium project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive consensus specification in `doc/consensus.md`
- Genesis block deterministic generation tooling
- Functional tests for smoke testing, P2P propagation, and basic reorg scenarios
- Regtest helper scripts for local development
- CHANGELOG.md for tracking project changes
- Documentation for P2P security measures
- Documentation for RPC API versioning policy

### Changed
- GitHub Actions CI workflow: Fixed repository name reference for Cirrus runners
- GitHub Actions CI workflow: Commented out non-existent qa-assets repository clone
- Updated `.gitignore` to exclude macOS junk files (.DS_Store, .AppleDouble, .LSOverride)

### Fixed
- GitHub Actions CI configuration errors preventing builds
- Removed .DS_Store files from repository

### Security
- Documented P2P DoS protection measures
- Established consensus rules to prevent chain splits

## [0.1.0] - 2025-12-16

### Added
- Initial fork from Bitcoin Core
- Hylium-specific genesis blocks for all networks (main/testnet/test4/regtest)
- Custom Bech32 HRP: `hyl` (mainnet), `thyl` (testnet), `hylrt` (regtest)
- Genesis block mining tools in `contrib/devtools/`
- Deterministic genesis parameters documented in `doc/genesis-values.md`

### Changed
- Rebranded from Bitcoin to Hylium throughout codebase
- Updated chain parameters for Hylium network
- Set mainnet genesis timestamp to 2025-12-16 12:00:00 UTC
- Provably unspendable coinbase output (OP_RETURN) - NO PREMINE

### Technical Specifications
- Block time: 10 minutes (600 seconds)
- Total supply: 21,000,000 HYL
- Halving interval: 210,000 blocks (~4 years)
- Initial block reward: 50 HYL
- Difficulty adjustment: Every 2016 blocks
- Max block weight: 4,000,000 weight units
- Coinbase maturity: 100 blocks

---

## Release Philosophy

### Version Numbering

Hylium follows **Semantic Versioning** (MAJOR.MINOR.PATCH):

- **MAJOR**: Consensus-breaking changes (hard forks)
- **MINOR**: New features, soft forks (backward compatible)
- **PATCH**: Bug fixes, performance improvements

### Release Types

#### Major Releases (X.0.0)
- Consensus-breaking changes requiring network-wide upgrade
- Extensive testing period (minimum 3 months on testnet)
- Clear migration path and upgrade deadline
- **AVOID** unless absolutely necessary for security or critical fixes

#### Minor Releases (0.X.0)
- New features that don't break consensus
- Soft fork activations (BIP9)
- RPC API additions (with versioning)
- Performance improvements
- Regular cadence: every 3-6 months

#### Patch Releases (0.0.X)
- Bug fixes only
- Security patches
- No new features
- Released as needed (typically within 1-2 weeks of bug discovery)

### Release Process

1. **Development**: Feature branches merged to `develop`
2. **Testing**: Extensive testing on testnet/test4
3. **Release Candidate**: Tag `vX.Y.Z-rc1`, community testing (2-4 weeks)
4. **Final Release**: Tag `vX.Y.Z`, binaries published, release notes
5. **Post-Release**: Monitor network, prepare hotfixes if needed

### Upgrade Policy

- **Mandatory upgrades**: Only for security-critical issues
- **Recommended upgrades**: For new features and improvements
- **Network activation**: Soft forks use BIP9 signaling (95% threshold)
- **Deprecation notice**: Minimum 6 months before removing features

---

## Security Disclosure

Security vulnerabilities should be reported privately to: security@hylium.org

**DO NOT** open public issues for security vulnerabilities.

We follow responsible disclosure:
1. Report received and acknowledged (24-48 hours)
2. Issue verified and fix developed (1-2 weeks)
3. Patch released quietly (no details in changelog initially)
4. Public disclosure after 90 days or after network upgrade (whichever is longer)

---

## Contribution Guidelines

All changes must:
1. Pass CI tests (builds, unit tests, functional tests)
2. Follow code style guidelines (linters)
3. Include appropriate documentation updates
4. Update this CHANGELOG under [Unreleased] section
5. Have clear, descriptive commit messages

### Changelog Entry Format

```markdown
### Added
- New feature description (#PR)

### Changed
- Modified behavior description (#PR)

### Deprecated
- Feature marked for future removal (#PR)

### Removed
- Deleted feature description (#PR)

### Fixed
- Bug fix description (#PR)

### Security
- Security improvement description (#PR)
```

---

## Historical Note

Hylium is a fork of Bitcoin Core, honoring the original vision of decentralized, sound money while establishing its own distinct network and community.

[Unreleased]: https://github.com/Panagiotis012/Hylium/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/Panagiotis012/Hylium/releases/tag/v0.1.0
