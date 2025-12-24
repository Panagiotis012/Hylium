# Hylium Testing Strategy and Improvement Plan

**Version:** 1.0
**Last Updated:** 2025-12-24

This document outlines the comprehensive testing strategy for Hylium to ensure consensus correctness, prevent regressions, and maintain network stability.

---

## Table of Contents

1. [Testing Philosophy](#testing-philosophy)
2. [Current Test Coverage](#current-test-coverage)
3. [Test Categories](#test-categories)
4. [Consensus Critical Tests](#consensus-critical-tests)
5. [Reorganization Tests](#reorganization-tests)
6. [Mempool and Fee Tests](#mempool-and-fee-tests)
7. [Difficulty Adjustment Tests](#difficulty-adjustment-tests)
8. [Timestamp Validation Tests](#timestamp-validation-tests)
9. [CI/CD Integration](#cicd-integration)
10. [Test Improvement Roadmap](#test-improvement-roadmap)

---

## Testing Philosophy

### Core Principles

1. **Consensus is Sacred**: Any consensus bug can split the network
2. **Test Before Code**: Write tests for edge cases before implementing fixes
3. **Regression Prevention**: Every bug fix gets a regression test
4. **Continuous Validation**: All PRs must pass full test suite
5. **Determinism**: Tests must be reproducible across machines

### Testing Pyramid

```
         /\
        /  \  E2E/Integration Tests (~5%)
       /____\
      /      \  Functional Tests (~25%)
     /________\
    /          \ Unit Tests (~70%)
   /____________\
```

**Balance:**
- **70% Unit Tests**: Fast, focused, test individual components
- **25% Functional Tests**: Test node behavior, P2P interactions
- **5% Integration Tests**: Multi-node scenarios, network simulation

---

## Current Test Coverage

### Existing Test Suites

```bash
# Unit tests (C++)
./src/test/test_hylium

# Functional tests (Python)
./test/functional/test_runner.py

# Fuzz tests
./test/fuzz/test_runner.py
```

### Current Functional Tests

✅ **Already Implemented:**
- `feature_hylium_devnet_smoke.py` - Basic node startup and block generation
- `feature_hylium_p2p_propagation.py` - Transaction/block propagation
- `feature_hylium_reorg_basic.py` - Simple reorganization scenarios

### Coverage Gaps (To Be Addressed)

❌ **Missing Critical Tests:**
- Deep reorganization scenarios (>6 blocks)
- Competing chains with equal work
- Difficulty adjustment edge cases
- Time warp attack prevention
- Mempool eviction policies
- Fee estimation accuracy
- Signature validation edge cases
- Script validation boundary conditions

---

## Test Categories

### 1. Unit Tests (C++)

**Location:** `src/test/`

**Purpose:** Test individual functions and classes in isolation.

**Examples:**
```cpp
// src/test/consensus_tests.cpp
BOOST_AUTO_TEST_CASE(test_block_subsidy_halving)
{
    BOOST_CHECK_EQUAL(GetBlockSubsidy(0, params), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(209999, params), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(210000, params), 25 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(420000, params), 12.5 * COIN);
    // ...
}
```

**Coverage Goals:**
- [ ] Subsidy calculation (all halvings)
- [ ] Difficulty adjustment algorithm
- [ ] Merkle root calculation
- [ ] Transaction validation
- [ ] Script execution
- [ ] Signature verification
- [ ] Address encoding/decoding
- [ ] Network message serialization

### 2. Functional Tests (Python)

**Location:** `test/functional/`

**Purpose:** Test node behavior as a black box via RPC.

**Structure:**
```python
class MyTestCase(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

    def run_test(self):
        # Test logic here
        pass
```

**Coverage Goals:**
- [ ] Block validation rules
- [ ] Transaction relay
- [ ] Mempool management
- [ ] Wallet operations
- [ ] P2P protocol
- [ ] Reorganizations
- [ ] Network upgrades

### 3. Fuzz Tests

**Location:** `src/test/fuzz/`

**Purpose:** Find crashes and undefined behavior via random inputs.

**Coverage Goals:**
- [ ] Block deserialization
- [ ] Transaction parsing
- [ ] Script execution
- [ ] P2P message handling
- [ ] RPC input validation

### 4. Integration Tests

**Purpose:** Multi-node network scenarios.

**Coverage Goals:**
- [ ] Network partitions
- [ ] Mining competition
- [ ] Double-spend attempts
- [ ] Eclipse attack simulation

---

## Consensus Critical Tests

### Block Validation Tests

**File:** `test/functional/feature_block_validation.py` (to be created)

**Test Cases:**

```python
def test_block_size_limits(self):
    """Test blocks at and exceeding MAX_BLOCK_WEIGHT"""
    # Create block with exactly MAX_BLOCK_WEIGHT
    valid_block = create_block_at_weight_limit()
    assert node.submitblock(valid_block.serialize().hex()) is None

    # Create block exceeding limit by 1 byte
    invalid_block = create_block_over_weight_limit()
    assert node.submitblock(invalid_block.serialize().hex()) == "invalid"

def test_coinbase_maturity(self):
    """Test coinbase outputs can't be spent before 100 confirmations"""
    # Generate block, get coinbase
    blockhash = node.generatetoaddress(1, address)[0]
    coinbase_tx = node.getblock(blockhash, 2)['tx'][0]

    # Try to spend at height 1 (should fail)
    assert_raises_rpc_error(-26, "bad-txns-premature-spend-of-coinbase",
                            node.sendrawtransaction, spend_tx)

    # Generate 99 more blocks (total 100)
    node.generatetoaddress(99, address)

    # Should still fail (needs 100 confirms = 101 blocks total)
    assert_raises_rpc_error(-26, "bad-txns-premature-spend-of-coinbase",
                            node.sendrawtransaction, spend_tx)

    # Generate 1 more (101 total)
    node.generatetoaddress(1, address)

    # Now it should work
    txid = node.sendrawtransaction(spend_tx)
    assert txid

def test_duplicate_transaction_in_block(self):
    """Test blocks with duplicate transactions are rejected"""
    # Create block with same tx twice
    block = create_block_with_duplicate_tx()
    assert node.submitblock(block.serialize().hex()) == "bad-txns-duplicate"

def test_block_subsidy_amount(self):
    """Test coinbase subsidy is correct for each era"""
    for height in [0, 210000, 420000, 630000]:
        block = node.getblock(node.getblockhash(height), 2)
        coinbase = block['tx'][0]
        expected_subsidy = get_expected_subsidy(height)
        actual_subsidy = coinbase['vout'][0]['value']
        assert_equal(actual_subsidy, expected_subsidy)
```

**Priority:** 🔴 **CRITICAL** - Must implement before mainnet launch

---

## Reorganization Tests

### Deep Reorg Tests

**File:** `test/functional/feature_reorg_deep.py` (to be created)

**Test Cases:**

```python
def test_reorg_6_blocks(self):
    """Test 6-block reorganization (typical 'confirmed' threshold)"""
    # Node 0 mines 10 blocks
    self.nodes[0].generatetoaddress(10, addr0)

    # Disconnect nodes
    disconnect_nodes(self.nodes[0], self.nodes[1])

    # Node 0 mines 6 more blocks (chain A)
    self.nodes[0].generatetoaddress(6, addr0)

    # Node 1 mines 7 blocks with more work (chain B)
    self.nodes[1].generatetoaddress(7, addr1)

    # Reconnect - node 0 should reorg to chain B
    connect_nodes(self.nodes[0], self.nodes[1])
    self.sync_blocks()

    # Verify both on same chain
    assert_equal(self.nodes[0].getbestblockhash(),
                 self.nodes[1].getbestblockhash())

def test_reorg_competing_equal_work(self):
    """Test competing chains with equal work (should keep first seen)"""
    # Both nodes mine 5 blocks from same parent
    parent = self.nodes[0].getbestblockhash()
    disconnect_nodes(self.nodes[0], self.nodes[1])

    self.nodes[0].generatetoaddress(5, addr0)
    chain_a_tip = self.nodes[0].getbestblockhash()

    self.nodes[1].invalidateblock(self.nodes[1].getbestblockhash())  # Rewind
    self.nodes[1].generatetoaddress(5, addr1)
    chain_b_tip = self.nodes[1].getbestblockhash()

    # Connect - node 0 should keep chain A (first seen)
    connect_nodes(self.nodes[0], self.nodes[1])
    time.sleep(2)

    assert_equal(self.nodes[0].getbestblockhash(), chain_a_tip)

def test_reorg_transaction_resurrection(self):
    """Test that transactions in orphaned blocks return to mempool"""
    # Send tx in block that will be orphaned
    txid = self.nodes[0].sendtoaddress(addr1, 1.0)
    self.nodes[0].generatetoaddress(1, addr0)

    # Verify tx is confirmed
    assert_equal(self.nodes[0].gettransaction(txid)['confirmations'], 1)

    # Cause reorg that orphans this block
    self.nodes[0].invalidateblock(self.nodes[0].getbestblockhash())

    # Tx should be back in mempool
    assert txid in self.nodes[0].getrawmempool()

def test_reorg_double_spend_prevention(self):
    """Test that reorg doesn't allow double spends"""
    # Create and broadcast transaction
    utxo = self.nodes[0].listunspent()[0]
    tx1 = create_transaction(utxo, addr1, 1.0)
    txid1 = self.nodes[0].sendrawtransaction(tx1)

    # Mine it
    self.nodes[0].generatetoaddress(1, addr0)

    # Try to create conflicting tx spending same UTXO
    tx2 = create_transaction(utxo, addr2, 1.0)

    # Cause reorg that orphans block with tx1
    self.nodes[0].invalidateblock(self.nodes[0].getbestblockhash())

    # Both txs in mempool is invalid - only one should be accepted
    assert txid1 in self.nodes[0].getrawmempool()
    assert_raises_rpc_error(-26, "bad-txns-inputs-missingorspent",
                            self.nodes[0].sendrawtransaction, tx2)
```

**Priority:** 🔴 **CRITICAL**

---

## Mempool and Fee Tests

### Fee Policy Tests

**File:** `test/functional/feature_mempool_fees.py` (to be created)

**Test Cases:**

```python
def test_mempool_eviction_by_fee(self):
    """Test low-fee transactions are evicted when mempool is full"""
    # Fill mempool to capacity with low-fee transactions
    # Add high-fee transaction
    # Verify low-fee txs are evicted

def test_replace_by_fee(self):
    """Test RBF (BIP125) works correctly"""
    # Send tx with low fee and RBF signal
    # Replace with higher fee version
    # Verify replacement succeeds

def test_mempool_fee_estimation(self):
    """Test fee estimation accuracy"""
    # Generate various fee-rate transactions
    # Check estimatesmartfee returns reasonable values

def test_mempool_limits(self):
    """Test mempool size and count limits"""
    # Verify can't exceed maxmempool setting
    # Verify oldest transactions evicted first (by entry time)
```

**Priority:** 🟡 **HIGH**

---

## Difficulty Adjustment Tests

### Retarget Tests

**File:** `test/functional/feature_difficulty_adjustment.py` (to be created)

**Test Cases:**

```python
def test_difficulty_increase(self):
    """Test difficulty increases when blocks come too fast"""
    # Mine 2016 blocks faster than 10min average
    # Check next difficulty is higher

def test_difficulty_decrease(self):
    """Test difficulty decreases when blocks come too slow"""
    # Mine 2016 blocks slower than 10min average
    # Check next difficulty is lower

def test_difficulty_clamp_4x(self):
    """Test difficulty can't change more than 4x per period"""
    # Try to trigger >4x increase
    # Verify clamped to exactly 4x

def test_difficulty_at_boundaries(self):
    """Test difficulty at minimum and maximum values"""
    # Test minimum difficulty (regtest)
    # Test approaching maximum difficulty

def test_timewarp_attack_prevention(self):
    """Test median-time-past prevents timewarp attacks"""
    # Attempt to manipulate timestamps
    # Verify MTP rule prevents exploit
```

**Priority:** 🔴 **CRITICAL**

---

## Timestamp Validation Tests

### MTP and Future Block Tests

**File:** `test/functional/feature_block_timestamp.py` (to be created)

**Test Cases:**

```python
def test_median_time_past(self):
    """Test block timestamp must be > median of last 11 blocks"""
    # Get MTP
    mtp = node.getblockheader(node.getbestblockhash())['mediantime']

    # Try to mine block with timestamp <= MTP
    block = create_block_with_timestamp(mtp)
    assert node.submitblock(block.serialize().hex()) == "time-too-old"

    # Mine block with timestamp = MTP + 1
    block = create_block_with_timestamp(mtp + 1)
    assert node.submitblock(block.serialize().hex()) is None  # Success

def test_future_block_limit(self):
    """Test blocks can't be more than 2 hours in future"""
    # Try to mine block with timestamp > now + 2 hours
    future_time = int(time.time()) + 7201
    block = create_block_with_timestamp(future_time)
    assert node.submitblock(block.serialize().hex()) == "time-too-new"

    # Block exactly at limit should work
    future_time = int(time.time()) + 7200
    block = create_block_with_timestamp(future_time)
    assert node.submitblock(block.serialize().hex()) is None

def test_timestamp_order_not_required(self):
    """Test block timestamps don't need to be strictly increasing"""
    # Block N can have earlier timestamp than block N-1
    # As long as both satisfy MTP rule
```

**Priority:** 🔴 **CRITICAL**

---

## CI/CD Integration

### GitHub Actions Workflow

**File:** `.github/workflows/ci.yml` (already exists, needs enhancement)

**Required Checks:**

```yaml
jobs:
  unit-tests:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-24.04, macos-15, windows-2022]
    steps:
      - name: Run unit tests
        run: |
          cmake --build build --target test_hylium
          ./build/src/test/test_hylium

  functional-tests:
    runs-on: ubuntu-24.04
    steps:
      - name: Run functional tests
        run: |
          ./test/functional/test_runner.py --ci --jobs=4

  consensus-tests:
    runs-on: ubuntu-24.04
    steps:
      - name: Run consensus-critical tests
        run: |
          ./test/functional/test_runner.py \
            feature_block_validation.py \
            feature_reorg_deep.py \
            feature_difficulty_adjustment.py \
            feature_block_timestamp.py

  lint:
    runs-on: ubuntu-24.04
    steps:
      - name: Run linters
        run: python .github/ci-lint-exec.py
```

### Pre-Commit Hooks

**File:** `.git/hooks/pre-commit` (optional, recommended)

```bash
#!/bin/bash
# Run quick tests before allowing commit

# Run unit tests
echo "Running unit tests..."
./build/src/test/test_hylium --run_test=consensus_tests || exit 1

# Run linters
echo "Running linters..."
./test/lint/lint-all.sh || exit 1

echo "✓ All checks passed"
```

---

## Test Improvement Roadmap

### Phase 1: Critical Consensus Tests (Month 1-2)

**Goal:** Prevent consensus bugs that could split network

- [ ] Implement `feature_block_validation.py`
- [ ] Implement `feature_reorg_deep.py`
- [ ] Implement `feature_difficulty_adjustment.py`
- [ ] Implement `feature_block_timestamp.py`
- [ ] Add fuzz tests for block parsing
- [ ] Add fuzz tests for transaction parsing

**Success Criteria:**
- 90%+ coverage of consensus-critical code paths
- All tests passing on CI
- No consensus regressions in 100+ test runs

### Phase 2: Mempool and Network Tests (Month 3)

**Goal:** Ensure mempool behaves correctly under stress

- [ ] Implement `feature_mempool_fees.py`
- [ ] Implement `feature_mempool_eviction.py`
- [ ] Implement `feature_rbf.py` (replace-by-fee)
- [ ] Implement `feature_double_spend.py`
- [ ] Add P2P flood simulation tests

**Success Criteria:**
- Mempool handles 10,000+ transactions gracefully
- Fee estimation within 20% of actual next-block fee
- No memory leaks under sustained load

### Phase 3: Network Resilience Tests (Month 4)

**Goal:** Test behavior under adversarial conditions

- [ ] Implement `feature_network_partition.py`
- [ ] Implement `feature_eclipse_attack.py`
- [ ] Implement `feature_selfish_mining.py`
- [ ] Implement long-term regtest network simulation

**Success Criteria:**
- Network recovers from partitions automatically
- Eclipse attack mitigation verified
- Selfish mining is unprofitable

### Phase 4: Performance and Stress Tests (Month 5-6)

**Goal:** Ensure node can handle mainnet load

- [ ] Benchmark block validation speed
- [ ] Benchmark transaction validation speed
- [ ] Test mempool with 300 MB transactions
- [ ] Test syncing 1M+ blocks from scratch
- [ ] Profile and optimize hot paths

**Success Criteria:**
- Initial Block Download < 24 hours on typical hardware
- Validates 4 MB blocks in < 1 second
- Handles 100+ peers without degradation

---

## Testing Best Practices

### 1. Deterministic Tests

All tests MUST be deterministic:

```python
# ✓ Good - deterministic
def test_subsidy():
    assert get_block_subsidy(210000) == 25 * COIN

# ✗ Bad - depends on current time
def test_block_timestamp():
    block.nTime = int(time.time())  # Non-deterministic!
```

### 2. Test Isolation

Each test MUST clean up after itself:

```python
def run_test(self):
    try:
        # Test logic
        pass
    finally:
        # Clean up
        self.stop_nodes()
```

### 3. Descriptive Assertions

```python
# ✓ Good
assert_equal(node.getblockcount(), 101,
             "Expected 101 blocks after mining 100 + genesis")

# ✗ Bad
assert node.getblockcount() == 101
```

### 4. Test Data Organization

```python
# Put test data in test/functional/data/
GENESIS_BLOCK_HEX = open('test/functional/data/genesis.hex').read()
```

### 5. Comment Why, Not What

```python
# ✓ Good
# BIP34 requires block height in coinbase scriptSig
assert coinbase.vin[0].scriptSig[0] == height

# ✗ Bad
# Check scriptSig
assert coinbase.vin[0].scriptSig[0] == height
```

---

## Continuous Improvement

### Monthly Review

- Review test failures from CI
- Add regression tests for all bugs found
- Update this document with new test categories

### Quarterly Audit

- External security review of consensus code
- Fuzzing campaign (7 days continuous)
- Update test coverage metrics

### Community Contributions

Encourage community test contributions:

1. Label issues as `good-first-test`
2. Provide test templates
3. Review and merge quickly
4. Recognize contributors in release notes

---

## Metrics and Reporting

### Coverage Tracking

```bash
# Generate coverage report
./configure --enable-lcov
make cov

# View report
open coverage/index.html
```

**Target Coverage:**
- Consensus code: >95%
- Wallet code: >80%
- RPC handlers: >90%

### Test Statistics

Track in each release:
- Total number of tests
- Average test execution time
- Test stability (flake rate)
- Code coverage percentage

---

## Appendix: Running Tests Locally

### Quick Start

```bash
# All unit tests
./build/src/test/test_hylium

# All functional tests
./test/functional/test_runner.py

# Specific test
./test/functional/feature_hylium_reorg_basic.py

# With debug logging
./test/functional/test_runner.py --loglevel=DEBUG

# Extended tests (slow)
./test/functional/test_runner.py --extended

# Parallel execution
./test/functional/test_runner.py --jobs=4
```

### Test Helper Scripts

```bash
# Run only consensus tests
./test/functional/test_runner.py --ci --group=consensus

# Run only mempool tests
./test/functional/test_runner.py --ci --group=mempool

# Run all fast tests (< 30s each)
./test/functional/test_runner.py --ci --group=quick
```

---

**Last Updated:** 2025-12-24
**Next Review:** 2026-01-24
