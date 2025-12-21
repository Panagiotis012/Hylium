#!/usr/bin/env python3
# Copyright (c) Hylium
# Distributed under the MIT software license.

import time

from test_framework.test_framework import HyliumTestFramework
from test_framework.util import assert_equal
from test_framework.authproxy import JSONRPCException


def wait_for(predicate, timeout=10, step=0.05, msg="wait_for timeout"):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(step)
    raise AssertionError(msg)


class HyliumP2PPropagationTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

        # IMPORTANT: wallet RPCs must exist for createwallet/getnewaddress/sendtoaddress
        # If the framework adds -disablewallet, this overrides it (last one wins).
        self.extra_args = [
            ["-disablewallet=0"],
            ["-disablewallet=0"],
        ]

    def _create_wallet_if_needed(self, node, name: str):
        try:
            node.createwallet(name)
        except JSONRPCException as e:
            # Ignore "already exists" style errors, fail on everything else
            code = (e.error or {}).get("code")
            if code in (-4, -18):
                return
            raise

    def run_test(self):
        n0, n1 = self.nodes

        self._create_wallet_if_needed(n0, "minerA")
        self._create_wallet_if_needed(n1, "minerB")

        w0 = n0.get_wallet_rpc("minerA")
        w1 = n1.get_wallet_rpc("minerB")

        # Fund node0
        addr0 = w0.getnewaddress()
        n0.generatetoaddress(101, addr0, called_by_framework=True)
        self.sync_all()

        # Send from node0 -> node1 and check mempool propagation
        addr1 = w1.getnewaddress()
        txid = w0.sendtoaddress(addr1, 1)

        wait_for(lambda: txid in n1.getrawmempool(),
                 timeout=10,
                 msg="tx did not propagate to node1 mempool")

        # Confirm by mining on node1
        mine_addr = w1.getnewaddress()
        n1.generatetoaddress(1, mine_addr, called_by_framework=True)
        self.sync_all()

        conf0 = w0.gettransaction(txid)["confirmations"]
        conf1 = w1.gettransaction(txid)["confirmations"]
        assert_equal(conf0, 1)
        assert_equal(conf1, 1)


if __name__ == "__main__":
    HyliumP2PPropagationTest(__file__).main()
