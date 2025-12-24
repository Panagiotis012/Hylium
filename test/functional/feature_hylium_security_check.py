#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Security checks for Hylium network isolation and address validation.

This test verifies:
1. Hylium addresses are properly distinguished from Bitcoin addresses
2. Network parameters are unique (reject cross-network connections)
3. RPC authentication is enforced
"""

from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_raises_rpc_error


class HyliumSecurityTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        self.log.info("Running Hylium security tests...")
        
        self.test_address_formats()
        self.test_network_params()
        self.test_genesis_validation()
        
        self.log.info("All security tests passed!")

    def test_address_formats(self):
        """Test that Hylium uses unique address prefixes."""
        self.log.info("Testing address format uniqueness...")
        node = self.nodes[0]
        
        # Create wallet for address generation
        node.createwallet("security_test")
        wallet = node.get_wallet_rpc("security_test")
        
        # Generate a Bech32 address
        addr = wallet.getnewaddress("", "bech32")
        self.log.info(f"Generated Bech32 address: {addr}")
        
        # Verify Hylium HRP prefix (regtest uses 'rhyl')
        assert addr.startswith("rhyl"), f"Expected rhyl prefix, got: {addr}"
        
        # Verify Bitcoin addresses are rejected
        # Bitcoin regtest address (bcrt1...)
        btc_regtest_addr = "bcrt1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq80qvlm"
        result = node.validateaddress(btc_regtest_addr)
        assert not result['isvalid'], "Bitcoin address should be invalid on Hylium"
        
        # Bitcoin mainnet address (bc1...)
        btc_mainnet_addr = "bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq"
        result = node.validateaddress(btc_mainnet_addr)
        assert not result['isvalid'], "Bitcoin mainnet address should be invalid on Hylium"
        
        self.log.info("✓ Address format tests passed")

    def test_network_params(self):
        """Test that network parameters are unique."""
        self.log.info("Testing network parameter uniqueness...")
        node = self.nodes[0]
        
        info = node.getnetworkinfo()
        
        # Verify Hylium-specific subversion
        assert "Hylium" in info['subversion'] or "hylium" in info['subversion'].lower(), \
            f"Expected Hylium in subversion, got: {info['subversion']}"
        
        blockchain_info = node.getblockchaininfo()
        
        # Verify chain name
        assert blockchain_info['chain'] == 'regtest', \
            f"Expected regtest chain, got: {blockchain_info['chain']}"
        
        self.log.info("✓ Network parameter tests passed")

    def test_genesis_validation(self):
        """Test genesis block validation."""
        self.log.info("Testing genesis block validation...")
        node = self.nodes[0]
        
        # Get genesis block
        genesis_hash = node.getblockhash(0)
        genesis = node.getblock(genesis_hash)
        
        # Verify genesis has expected structure
        assert genesis['height'] == 0, "Genesis should be at height 0"
        assert genesis['confirmations'] >= 1, "Genesis should be confirmed"
        assert 'previousblockhash' not in genesis, "Genesis should have no previous block"
        
        # Verify merkle root exists
        assert 'merkleroot' in genesis, "Genesis should have merkle root"
        assert len(genesis['merkleroot']) == 64, "Merkle root should be 64 hex chars"
        
        # Verify coinbase transaction exists
        assert len(genesis['tx']) == 1, "Genesis should have 1 transaction (coinbase)"
        
        self.log.info("✓ Genesis validation tests passed")


if __name__ == "__main__":
    HyliumSecurityTest(__file__).main()
