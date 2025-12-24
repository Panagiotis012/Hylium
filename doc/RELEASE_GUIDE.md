# Hylium Production Release Guide

## ✅ Pre-Release Checklist

### Code Quality
- [x] All Hylium tests passing
- [x] No BUG_BOUNTY references
- [x] Documentation updated
- [x] Clean codebase

### Security
- [x] SECURITY.md complete
- [ ] PGP keys generated
- [ ] security@hylium.org configured

### Infrastructure (Before Mainnet)
- [ ] Seed nodes deployed
- [ ] DNS seeds configured  
- [ ] Block explorer ready

---

## 🚀 Publishing to GitHub

### Step 1: Create GitHub Organization (Recommended)

1. Go to https://github.com/organizations/new
2. Name: `hylium` or `hylium-core`
3. Create new repository: `hylium`

### Step 2: Push Code

```bash
cd /Users/Apple/Desktop/Hylium

# Initialize git if needed
git init

# Add all files
git add .

# Commit
git commit -m "Hylium v0.1.0 - Initial Release"

# Add remote (change to your repo URL)
git remote add origin https://github.com/YOUR_ORG/hylium.git

# Push
git push -u origin master
```

### Step 3: Create Release

1. Go to repository → Releases → "Create new release"
2. Tag: `v0.1.0`
3. Title: `Hylium v0.1.0 - Testnet Release`
4. Description:
```markdown
## Hylium v0.1.0 - Testnet Release

**⚠️ TESTNET ONLY - Not for production use**

### What's Included
- Bitcoin Core based cryptocurrency
- Unique network parameters (port 9333)
- Bech32 addresses with `hyl1` prefix
- All soft forks active from genesis

### Quick Start
```bash
# Build
cmake -B build -G Ninja
cmake --build build -j4

# Run testnet
./build/bin/hyliumd -testnet
```

### Documentation
- [Whitepaper](doc/WHITEPAPER.md)
- [Build Guide](doc/build-unix.md)
- [Testing Guide](doc/testing.md)

### Security
Report vulnerabilities to security@hylium.org
See [SECURITY.md](SECURITY.md) for details.
```

### Step 4: Branch Protection (Recommended)

Settings → Branches → Add rule:
- Branch: `master`
- [x] Require pull request reviews
- [x] Require status checks
- [x] Require signed commits

---

## 📦 Build Binaries for Release

### macOS
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
# Binaries in build/bin/
```

### Linux (Ubuntu)
```bash
sudo apt install build-essential cmake ninja-build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Windows (Cross-compile from Linux)
```bash
# 1. Build dependencies (takes time)
make -C depends HOST=x86_64-w64-mingw32

# 2. Build Hylium using the generated toolchain
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=depends/x86_64-w64-mingw32/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## 🔐 GPG Signing (Recommended)

### Generate Key
```bash
gpg --full-generate-key
# RSA 4096, no expiry, your-email@hylium.org
```

### Sign Release
```bash
gpg --armor --detach-sign hylium-0.1.0-x86_64-linux.tar.gz
```

### Add to SECURITY.md
Update the PGP key section with your public key fingerprint.

---

## 📋 Post-Release Tasks

1. **Announce**: Twitter, Discord, cryptocurrency forums
2. **Monitor**: Watch GitHub issues for bug reports
3. **Documentation**: Update website with download links
4. **Community**: Set up Discord/Telegram for support

---

## ⚠️ Important Notes

- Current release is **testnet only**
- Do NOT promote as mainnet until:
  - Security audit complete
  - Seed nodes stable
  - Community established
- Always use GPG-signed releases for trust
