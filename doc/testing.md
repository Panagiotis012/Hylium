# Hylium Testing Guide

This guide explains how to build and test Hylium Core.

## Quick Start

### Prerequisites

**macOS:**
```bash
brew install cmake ninja boost libevent zeromq qt@6 qrencode
```

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential cmake ninja-build pkg-config \
    libevent-dev libboost-dev libsqlite3-dev libzmq3-dev \
    qt6-base-dev qt6-tools-dev libqrencode-dev
```

### Build

```bash
cd /Users/Apple/Desktop/Hylium

# Configure
cmake -B build -G Ninja

# Build (use -j for parallel)
cmake --build build -j$(nproc)
```

### Run Unit Tests

```bash
# All unit tests
./build/bin/test_hylium

# Specific test suite
./build/bin/test_hylium -t wallet_tests

# List available tests
./build/bin/test_hylium -l test_suite
```

---

## Functional Tests

### Run All Tests

```bash
# Run all functional tests
python3 test/functional/test_runner.py

# Parallel execution (faster)
python3 test/functional/test_runner.py --jobs 4

# Extended tests (takes longer)
python3 test/functional/test_runner.py --extended
```

### Run Hylium-Specific Tests

```bash
# Security checks
python3 test/functional/feature_hylium_security_check.py

# Devnet smoke test
python3 test/functional/feature_hylium_devnet_smoke.py

# P2P propagation
python3 test/functional/feature_hylium_p2p_propagation.py

# Reorg handling
python3 test/functional/feature_hylium_reorg_basic.py

# Wallet functionality
python3 test/functional/feature_hylium_wallet_basic.py

# Mining functionality
python3 test/functional/feature_hylium_mining_basic.py
```

### Run Categories of Tests

```bash
# Wallet tests only
python3 test/functional/test_runner.py wallet_*

# P2P tests only
python3 test/functional/test_runner.py p2p_*

# RPC tests only
python3 test/functional/test_runner.py rpc_*

# Mining tests only
python3 test/functional/test_runner.py mining_*
```

### Test Options

```bash
# More verbose output
python3 test/functional/test_runner.py --loglevel DEBUG

# Keep temp directories on failure
python3 test/functional/test_runner.py --nocleanup

# Use BIP324 v2 encrypted transport
python3 test/functional/test_runner.py --v2transport

# Trace all RPC calls
python3 test/functional/feature_hylium_security_check.py --tracerpc
```

---

## Regtest Quick Start

Start a local regtest node for development:

```bash
# Start hyliumd in regtest mode
./build/bin/hyliumd -regtest -daemon

# Create a wallet
./build/bin/hylium-cli -regtest createwallet "dev"

# Get a new address
./build/bin/hylium-cli -regtest getnewaddress

# Mine 101 blocks (to have spendable coins)
./build/bin/hylium-cli -regtest generatetoaddress 101 "YOUR_ADDRESS"

# Check balance
./build/bin/hylium-cli -regtest getbalance

# Stop the node
./build/bin/hylium-cli -regtest stop
```

---

## CI Integration

The project uses GitHub Actions for CI. Main test configurations:

| Test Suite | Description |
|------------|-------------|
| `ci-matrix` | Cross-platform builds |
| `lint` | Code style checks |
| `ASan + LSan + UBSan` | Memory/undefined behavior sanitizers |
| `TSan` | Thread sanitizer |
| `MSan` | Memory sanitizer |
| `Valgrind, fuzz` | Memory debugging + fuzzing |
| `tidy` | clang-tidy static analysis |

---

## Test File Locations

| Category | Location |
|----------|----------|
| Unit tests | `src/test/` |
| Functional tests | `test/functional/` |
| Fuzz tests | `src/test/fuzz/` |
| Benchmark tests | `src/bench/` |
| Test framework | `test/functional/test_framework/` |

---

## Writing New Tests

### Functional Test Template

```python
#!/usr/bin/env python3
from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_equal

class MyNewTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

    def run_test(self):
        self.log.info("Starting test...")
        # Your test code here
        self.log.info("Test passed!")

if __name__ == "__main__":
    MyNewTest(__file__).main()
```

Save as `test/functional/feature_my_test.py` and run:

```bash
python3 test/functional/feature_my_test.py
```

---

## Troubleshooting

### Tests Fail to Start

```bash
# Clean build directory
rm -rf build
cmake -B build -G Ninja
cmake --build build -j4
```

### Port Conflicts

```bash
# Use different port seed
python3 test/functional/test_runner.py --portseed 12345
```

### View Test Logs

```bash
# After test failure, logs are in:
ls /tmp/hylium_func_test_*/test_framework.log

# Combine logs for debugging
python3 test/functional/combine_logs.py /tmp/hylium_func_test_XXXXX/
```
