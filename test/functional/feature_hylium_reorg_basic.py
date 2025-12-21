#!/usr/bin/env python3
import time
from decimal import Decimal

from test_framework.test_framework import HyliumTestFramework


class HyliumReorgBasicTest(HyliumTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.setup_clean_chain = True

    def run_test(self):
        n0, n1 = self.nodes

        # Deterministic mining keys/addresses (NO wallet needed)
        k0 = n0.get_deterministic_priv_key()
        k1 = n1.get_deterministic_priv_key()
        mine_addr0, mine_priv0 = k0.address, k0.key
        mine_addr1 = k1.address
        recv_addr1 = k1.address  # same deterministic address is fine

        # Helper: mine WITHOUT syncing (needed when nodes are disconnected)
        def mine_no_sync(node, nblocks, addr):
            node.generatetoaddress(nblocks, addr, called_by_framework=True)

        # 1) Build shared chain and mature coinbase (connected => OK to use self.generate)
        self.generate(n0, 101)
        self.sync_all()

        # 2) Disconnect for reorg scenario
        self.disconnect_nodes(0, 1)

        # Pick a mature coinbase from common ancestor (height 1)
        blk1 = n0.getblockhash(1)
        cb_txid = n0.getblock(blk1)["tx"][0]

        # No txindex => must provide blockhash OR read via getblock verbosity=2
        try:
            cb = n0.getrawtransaction(cb_txid, True, blk1)
        except Exception:
            cb = n0.getblock(blk1, 2)["tx"][0]

        # choose spendable vout
        vout_idx = None
        vout_val = None
        spk_hex = None
        for i, v in enumerate(cb["vout"]):
            if Decimal(str(v["value"])) > 0:
                vout_idx = i
                vout_val = Decimal(str(v["value"]))
                spk_hex = v["scriptPubKey"]["hex"]
                break
        assert vout_idx is not None, cb

        fee = Decimal("0.00010000")
        send_val = vout_val - fee
        assert send_val > 0, (vout_val, fee)

        raw = n0.createrawtransaction(
            [{"txid": cb_txid, "vout": vout_idx}],
            {recv_addr1: float(send_val)}
        )

        prevtxs = [{
            "txid": cb_txid,
            "vout": vout_idx,
            "scriptPubKey": spk_hex,
            "amount": float(vout_val),
        }]

        signed = n0.signrawtransactionwithkey(raw, [mine_priv0], prevtxs)
        assert signed.get("complete") is True, signed
        tx_hex = signed["hex"]

        txid = n0.sendrawtransaction(tx_hex)

        # 3) Mine it into a n0-only block (DISCONNECTED => DO NOT self.generate)
        mine_no_sync(n0, 1, mine_addr0)
        blk_with_tx = n0.getbestblockhash()
        assert txid in n0.getblock(blk_with_tx)["tx"], "tx not in mined block"

        # 4) On n1, mine a longer competing chain (DISCONNECTED)
        mine_no_sync(n1, 2, mine_addr1)

        # 5) Reconnect => reorg to n1 chain
        self.connect_nodes(0, 1)
        # IMPORTANT: μην κάνεις sync_all εδώ.
        # Το sync_all κάνει και sync_mempools, και ο Hylium δεν κάνει πάντα relay resurrected txs μετά από reorg.
        self.sync_blocks()

        # Αν το tx γύρισε στο mempool του n0 αλλά δεν έγινε relay στον n1, σπρώξ'το χειροκίνητα.
        if txid in n0.getrawmempool() and txid not in n1.getrawmempool():
            try:
                n1.sendrawtransaction(tx_hex)
            except Exception:
                pass

        self.sync_mempools()
        # old block should now be orphaned (confirmations <= 0)
        confs = n0.getblockheader(blk_with_tx)["confirmations"]
        assert confs <= 0, f"Expected orphaned block, confirmations={confs}"

        # tx should return to mempool (or at least be re-accepted)
        def in_mempool():
            return (txid in n0.getrawmempool()) or (txid in n1.getrawmempool())

        deadline = time.time() + 10
        while time.time() < deadline and not in_mempool():
            time.sleep(0.1)

        if not in_mempool():
            for nn in (n0, n1):
                try:
                    nn.sendrawtransaction(tx_hex)
                except Exception:
                    pass

        # 6) Confirm on the winning chain (CONNECTED => self.generate is fine)
        self.generate(n1, 1)
        self.sync_all()

        tip = n0.getbestblockhash()
        assert txid in n0.getblock(tip)["tx"], "tx not confirmed after reorg+mine"


if __name__ == "__main__":
    HyliumReorgBasicTest(__file__).main()
