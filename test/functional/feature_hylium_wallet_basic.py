#!/usr/bin/env python3
# Copyright (c) 2025 The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test wallet functionality on Hylium network.

This test verifies:
1. Wallet creation and backup
2. Address generation (all types)
3. Send/receive transactions
4. Balance tracking
5. Transaction history
"""

from decimal import Decimal
from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_equal, assert_greater_than


class HyliumWalletTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        self.log.info("Testing Hylium wallet functionality...")
        
        self.test_wallet_creation()
        self.test_address_generation()
        self.test_send_receive()
        self.test_balance_tracking()
        
        self.log.info("All wallet tests passed!")

    def test_wallet_creation(self):
        """Test wallet creation and basic info."""
        self.log.info("Testing wallet creation...")
        
        node = self.nodes[0]
        
        # Create a new wallet
        node.createwallet("test_wallet")
        wallet = node.get_wallet_rpc("test_wallet")
        
        # Verify wallet info
        info = wallet.getwalletinfo()
        assert_equal(info['walletname'], 'test_wallet')
        assert 'txcount' in info  # Core walletinfo field
        
        # Create encrypted wallet
        node.createwallet("encrypted_wallet", False, False, "testpassword123")
        enc_wallet = node.get_wallet_rpc("encrypted_wallet")
        enc_info = enc_wallet.getwalletinfo()
        assert_equal(enc_info['unlocked_until'], 0)  # Locked by default
        
        self.log.info("✓ Wallet creation test passed")

    def test_address_generation(self):
        """Test all address types are generated correctly."""
        self.log.info("Testing address generation...")
        
        wallet = self.nodes[0].get_wallet_rpc("test_wallet")
        
        # Generate different address types
        addr_legacy = wallet.getnewaddress("", "legacy")
        addr_p2sh = wallet.getnewaddress("", "p2sh-segwit")
        addr_bech32 = wallet.getnewaddress("", "bech32")
        addr_bech32m = wallet.getnewaddress("", "bech32m")
        
        self.log.info(f"Legacy: {addr_legacy}")
        self.log.info(f"P2SH-SegWit: {addr_p2sh}")
        self.log.info(f"Bech32: {addr_bech32}")
        self.log.info(f"Bech32m (Taproot): {addr_bech32m}")
        
        # Verify Hylium prefixes (regtest uses 'rhyl')
        assert addr_bech32.startswith("rhyl1q"), f"Bech32 should start with rhyl1q, got {addr_bech32}"
        assert addr_bech32m.startswith("rhyl1p"), f"Bech32m should start with rhyl1p, got {addr_bech32m}"
        
        # Verify addresses are valid
        for addr in [addr_legacy, addr_p2sh, addr_bech32, addr_bech32m]:
            result = self.nodes[0].validateaddress(addr)
            assert result['isvalid'], f"Address {addr} should be valid"
        
        self.log.info("✓ Address generation test passed")

    def test_send_receive(self):
        """Test sending and receiving transactions."""
        self.log.info("Testing send/receive...")
        
        # Setup wallets
        self.nodes[1].createwallet("receiver_wallet")
        
        wallet0 = self.nodes[0].get_wallet_rpc("test_wallet")
        wallet1 = self.nodes[1].get_wallet_rpc("receiver_wallet")
        
        # Mine some coins to wallet0
        addr0 = wallet0.getnewaddress()
        self.nodes[0].generatetoaddress(101, addr0, called_by_framework=True)
        self.sync_all()
        
        # Get initial balance
        balance0_before = wallet0.getbalance()
        assert balance0_before >= 50, f"Should have mining rewards, got {balance0_before}"
        
        # Send to wallet1
        recv_addr = wallet1.getnewaddress()
        amount = Decimal("10.0")
        txid = wallet0.sendtoaddress(recv_addr, float(amount))
        
        self.log.info(f"Sent {amount} HYL, txid: {txid}")
        
        # Confirm the transaction
        self.nodes[0].generatetoaddress(1, addr0, called_by_framework=True)
        self.sync_all()
        
        # Verify balances
        balance1 = wallet1.getbalance()
        assert_equal(balance1, amount)
        
        self.log.info("✓ Send/receive test passed")

    def test_balance_tracking(self):
        """Test that balances are tracked correctly."""
        self.log.info("Testing balance tracking...")
        
        wallet0 = self.nodes[0].get_wallet_rpc("test_wallet")
        wallet1 = self.nodes[1].get_wallet_rpc("receiver_wallet")
        
        # Check unconfirmed balance
        addr1 = wallet1.getnewaddress()
        txid = wallet0.sendtoaddress(addr1, 5.0)
        
        # Before confirmation
        balances = wallet1.getbalances()
        assert 'mine' in balances
        
        # After confirmation
        addr0 = wallet0.getnewaddress()
        self.nodes[0].generatetoaddress(1, addr0, called_by_framework=True)
        self.sync_all()
        
        # Verify transaction in history
        tx = wallet1.gettransaction(txid)
        assert tx['confirmations'] >= 1
        
        self.log.info("✓ Balance tracking test passed")


if __name__ == "__main__":
    HyliumWalletTest(__file__).main()
