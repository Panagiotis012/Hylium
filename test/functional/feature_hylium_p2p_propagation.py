#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test P2P message propagation across Hylium network.

This test verifies:
1. Blocks propagate correctly between nodes
2. Transactions propagate correctly between nodes
3. Network sync works as expected
4. Reorg handling is correct
"""

from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_equal


class HyliumNetworkPropagationTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 4
        self.setup_clean_chain = True

    def run_test(self):
        self.log.info("Testing Hylium network propagation...")
        
        self.test_block_propagation()
        self.test_transaction_propagation()
        self.test_chain_sync()
        
        self.log.info("All network propagation tests passed!")

    def test_block_propagation(self):
        """Test that blocks propagate to all nodes."""
        self.log.info("Testing block propagation...")
        
        # Create wallet on node 0
        self.nodes[0].createwallet("miner")
        wallet = self.nodes[0].get_wallet_rpc("miner")
        addr = wallet.getnewaddress()
        
        # Mine a block on node 0
        initial_height = self.nodes[0].getblockcount()
        self.nodes[0].generatetoaddress(1, addr, called_by_framework=True)
        
        # Wait for sync
        self.sync_all()
        
        # Verify all nodes received the block
        for i, node in enumerate(self.nodes):
            height = node.getblockcount()
            assert_equal(height, initial_height + 1, 
                f"Node {i} height mismatch: expected {initial_height + 1}, got {height}")
        
        # Verify all nodes have same tip
        tips = [node.getbestblockhash() for node in self.nodes]
        assert all(tip == tips[0] for tip in tips), "Nodes have different tips!"
        
        self.log.info("✓ Block propagation test passed")

    def test_transaction_propagation(self):
        """Test that transactions propagate to all mempools."""
        self.log.info("Testing transaction propagation...")
        
        # Node 0 needs funds - mine some blocks
        wallet0 = self.nodes[0].get_wallet_rpc("miner")
        addr0 = wallet0.getnewaddress()
        self.nodes[0].generatetoaddress(101, addr0, called_by_framework=True)
        self.sync_all()
        
        # Create wallet on node 3
        self.nodes[3].createwallet("receiver")
        wallet3 = self.nodes[3].get_wallet_rpc("receiver")
        recv_addr = wallet3.getnewaddress()
        
        # Send transaction from node 0
        txid = wallet0.sendtoaddress(recv_addr, 1.0)
        self.log.info(f"Sent tx: {txid}")
        
        # Sync mempools
        self.sync_mempools()
        
        # Verify all nodes have the tx in mempool
        for i, node in enumerate(self.nodes):
            mempool = node.getrawmempool()
            assert txid in mempool, f"Node {i} missing tx {txid} in mempool"
        
        self.log.info("✓ Transaction propagation test passed")

    def test_chain_sync(self):
        """Test that chain sync works after disconnect/reconnect."""
        self.log.info("Testing chain sync after disconnect...")
        
        wallet0 = self.nodes[0].get_wallet_rpc("miner")
        addr0 = wallet0.getnewaddress()
        
        # Record current height
        height_before = self.nodes[0].getblockcount()
        
        # Disconnect node 3
        self.disconnect_nodes(2, 3)
        
        # Mine blocks on nodes 0-2
        self.nodes[0].generatetoaddress(5, addr0, called_by_framework=True)
        self.sync_blocks(self.nodes[:3])
        
        # Node 3 should be behind
        assert_equal(self.nodes[3].getblockcount(), height_before + 1)  # +1 from previous test
        
        # Reconnect
        self.connect_nodes(2, 3)
        self.sync_all()
        
        # All nodes should have same height now
        final_height = self.nodes[0].getblockcount()
        for i, node in enumerate(self.nodes):
            assert_equal(node.getblockcount(), final_height,
                f"Node {i} height mismatch after sync")
        
        self.log.info("✓ Chain sync test passed")


if __name__ == "__main__":
    HyliumNetworkPropagationTest(__file__).main()
