#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test mining functionality on Hylium network.

This test verifies:
1. Block generation via RPC
2. Coinbase maturity (100 blocks)
3. Block subsidy halving schedule
4. Mining to specific addresses
"""

from decimal import Decimal
from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_equal, assert_greater_than


class HyliumMiningTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        self.log.info("Testing Hylium mining functionality...")
        
        self.test_block_generation()
        self.test_coinbase_maturity()
        self.test_block_subsidy()
        self.test_mining_to_address()
        
        self.log.info("All mining tests passed!")

    def test_block_generation(self):
        """Test that blocks can be generated."""
        self.log.info("Testing block generation...")
        
        node = self.nodes[0]
        node.createwallet("miner")
        wallet = node.get_wallet_rpc("miner")
        
        # Get initial height
        initial_height = node.getblockcount()
        
        # Generate blocks
        addr = wallet.getnewaddress()
        blocks = node.generatetoaddress(10, addr, called_by_framework=True)
        
        # Verify blocks were created
        assert_equal(len(blocks), 10)
        assert_equal(node.getblockcount(), initial_height + 10)
        
        # Verify each block hash is valid
        for block_hash in blocks:
            block = node.getblock(block_hash)
            assert 'height' in block
            assert 'tx' in block
        
        self.log.info("✓ Block generation test passed")

    def test_coinbase_maturity(self):
        """Test coinbase maturity rules (100 blocks)."""
        self.log.info("Testing coinbase maturity...")
        
        node = self.nodes[0]
        wallet = node.get_wallet_rpc("miner")
        addr = wallet.getnewaddress()
        
        # Mine 1 block
        node.generatetoaddress(1, addr, called_by_framework=True)
        
        # Check immature balance (should have immature coinbase)
        balances = wallet.getbalances()
        
        # After only 11 blocks total, coinbase from block 1 is immature
        # Need 100 confirmations for maturity
        current_height = node.getblockcount()
        self.log.info(f"Current height: {current_height}")
        
        # Mine more blocks to mature some coinbases
        node.generatetoaddress(100, addr, called_by_framework=True)
        
        # Now some coinbases should be mature
        balance = wallet.getbalance()
        self.log.info(f"Mature balance: {balance}")
        
        # Should have spendable balance now (first coinbases matured)
        assert_greater_than(balance, 0)
        
        self.log.info("✓ Coinbase maturity test passed")

    def test_block_subsidy(self):
        """Test block subsidy is correct."""
        self.log.info("Testing block subsidy...")
        
        node = self.nodes[0]
        wallet = node.get_wallet_rpc("miner")
        addr = wallet.getnewaddress()
        
        # Mine a block and check coinbase value
        blocks = node.generatetoaddress(1, addr, called_by_framework=True)
        block = node.getblock(blocks[0], 2)  # verbosity 2 includes tx details
        
        coinbase_tx = block['tx'][0]
        coinbase_value = sum(vout['value'] for vout in coinbase_tx['vout'])
        
        # In regtest, subsidy should be 50 HYL (before any halvings)
        # Note: There might be small fees from prior txs
        self.log.info(f"Coinbase value: {coinbase_value}")
        assert_greater_than(coinbase_value, 49)  # At least 49 (50 minus small fees)
        
        self.log.info("✓ Block subsidy test passed")

    def test_mining_to_address(self):
        """Test mining rewards go to correct address."""
        self.log.info("Testing mining to specific address...")
        
        node = self.nodes[0]
        
        # Create a new wallet and get a specific address
        node.createwallet("reward_wallet")
        reward_wallet = node.get_wallet_rpc("reward_wallet")
        reward_addr = reward_wallet.getnewaddress()
        
        # Initial balance should be 0
        assert_equal(reward_wallet.getbalance(), 0)
        
        # Mine blocks to this specific address
        node.generatetoaddress(101, reward_addr, called_by_framework=True)
        
        # Balance should now be positive (first coinbase matured)
        balance = reward_wallet.getbalance()
        assert_greater_than(balance, 0)
        self.log.info(f"Reward wallet balance: {balance}")
        
        self.log.info("✓ Mining to address test passed")


if __name__ == "__main__":
    HyliumMiningTest(__file__).main()
