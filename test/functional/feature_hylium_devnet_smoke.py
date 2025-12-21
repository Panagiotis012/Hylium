#!/usr/bin/env python3

from test_framework.test_framework import HyliumTestFramework


class HyliumDevnetSmokeTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True
        self.extra_args = [
            ["-disablewallet=0"],
            ["-disablewallet=0"],
        ]


    def run_test(self):
        n0, n1 = self.nodes[0], self.nodes[1]

        # Wallets (χρειάζονται για getnewaddress / sendtoaddress / sign)
        n0.createwallet("minerA")
        n1.createwallet("minerB")

        w0 = n0.get_wallet_rpc("minerA")
        w1 = n1.get_wallet_rpc("minerB")

        # Mine 101 blocks on node 0 so coinbase is spendable
        addr0 = w0.getnewaddress()
        n0.generatetoaddress(101, addr0, called_by_framework=True)
        self.sync_all()

        # Send 1 coin from A -> B
        recv_addr = w1.getnewaddress()
        txid = w0.sendtoaddress(recv_addr, 1)

        # Wait for mempool sync (αν υπάρχει) αλλιώς sync_all αρκεί
        if hasattr(self, "sync_mempools"):
            self.sync_mempools()
        else:
            self.sync_all()

        # Mine 1 block on node 1 to confirm
        mine_addr = w1.getnewaddress()
        n1.generatetoaddress(1, mine_addr, called_by_framework=True)
        self.sync_all()

        # Verify confirmations on both wallets
        c0 = w0.gettransaction(txid)["confirmations"]
        c1 = w1.gettransaction(txid)["confirmations"]
        assert c0 >= 1, f"sender confirmations={c0}"
        assert c1 >= 1, f"receiver confirmations={c1}"

        # Verify receiver got +1.0 (amount semantics depend on wallet; keep strict for smoke)
        amt = w1.gettransaction(txid)["amount"]
        assert float(amt) == 1.0, f"receiver amount={amt}"

        self.log.info(f"OK: tx {txid} confirmed on both nodes.")


if __name__ == "__main__":
    HyliumDevnetSmokeTest(__file__).main()
