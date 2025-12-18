# Hylium Genesis Values (WIP)

Goal: derive final genesis parameters for all networks (main/testnet/test4/regtest) using `contrib/devtools/genesis_hylium.cpp`. Once hashes/nonces are found, copy them here and into `chainparams.cpp` later.

## Common parameters
- Block time: 600s
- Supply/halving: 21M cap, halving every 210,000 blocks
- Coinbase output: OP_RETURN "Hylium genesis" (provably unspendable, no premine)
- nBits: main 0x1d00ffff, testnet/test4/regtest 0x207fffff (easy target for iteration)
- nTime (main/test/test4): 1765886400 (2025-12-16 12:00:00 UTC)
- Messages:
  - main:    `Hylium 2025-12-16: Verify, don't trust.`
  - testnet: `Hylium testnet 2025-12-16: break safely.`
  - test4:   `Hylium test4 2025-12-16: staging chain.`
  - regtest: `Hylium regtest: local sandbox.`

## How to compute
Compile:
```sh
c++ -O3 -march=native -funroll-loops -std=c++17 \
  -I/opt/homebrew/opt/openssl/include -L/opt/homebrew/opt/openssl/lib \
  contrib/devtools/genesis_hylium.cpp -lcrypto -o /tmp/genesis_hylium
```

Run (per network; uses all CPUs by default):
```sh
HYLIUM_GENESIS_THREADS=$(sysctl -n hw.ncpu) HYLIUM_GENESIS_NETS=main /tmp/genesis_hylium
HYLIUM_GENESIS_THREADS=$(sysctl -n hw.ncpu) HYLIUM_GENESIS_NETS=testnet /tmp/genesis_hylium
HYLIUM_GENESIS_THREADS=$(sysctl -n hw.ncpu) HYLIUM_GENESIS_NETS=test4 /tmp/genesis_hylium
HYLIUM_GENESIS_THREADS=$(sysctl -n hw.ncpu) HYLIUM_GENESIS_NETS=regtest /tmp/genesis_hylium
```

To resume from a higher nonce range (e.g. after abort), set:
```sh
HYLIUM_GENESIS_START=<nonce_offset>
```
Example: `HYLIUM_GENESIS_START=800000000`.

Expected work: difficulty-1 search (~4.29e9 nonces on average). With ~10 threads CPU, expect multiple minutes to hours; let it run until it prints a “Found genesis” block.

## Record final values here (fill after discovery)

### main
```
nTime=1765886400
nBits=0x1d00ffff
extraNonce=0
nNonce=3320618862
hashGenesisBlock=7c4dde18bc60947862bbb36fa475e81638f23442cf6b70e799e21c2000000000
hashGenesisBlock_be=00000000201ce299e7706bcf4234f23816e875a46fb3bb62789460bc18de4d7c
merkleRoot=0dc9b3e16f9d34e8d9d698dd1c73c6a1ac40640b96ec3774bfad535d393dff44
merkleRoot_be=44ff3d395d53adbf7437ec960b6440aca1c6731cdd98d6d9e8349d6fe1b3c90d
txhash=0dc9b3e16f9d34e8d9d698dd1c73c6a1ac40640b96ec3774bfad535d393dff44
txhash_be=44ff3d395d53adbf7437ec960b6440aca1c6731cdd98d6d9e8349d6fe1b3c90d
```

### testnet
```
nTime=1765886400
nBits=0x207fffff
nNonce=1
hashGenesisBlock=20abf105f06c2e9be359faeedec956e458186d91a253dc53b579bf004f9d206e
hashGenesisBlock_be=6e209d4f00bf79b553dc53a2916d1858e456c9deeefa59e39b2e6cf005f1ab20
hashMerkleRoot=04a76e1b6d70a2470e65295d0a30045f5cc00f4758b0df4039faedc54253ee53
hashMerkleRoot_be=53ee5342c5edfa3940dfb058470fc05c5f04300a5d29650e47a2706d1b6ea704
txhash=04a76e1b6d70a2470e65295d0a30045f5cc00f4758b0df4039faedc54253ee53
txhash_be=53ee5342c5edfa3940dfb058470fc05c5f04300a5d29650e47a2706d1b6ea704
```

### test4
```
nTime=1765886400
nBits=0x207fffff
nNonce=0
hashGenesisBlock=9d1d330dab3676c79aaf133f5f536b7336a072a1f6f57d3fab392ad0cd50de3b
hashGenesisBlock_be=3bde50cdd02a39ab3f7df5f6a172a036736b535f3f13af9ac77636ab0d331d9d
hashMerkleRoot=ca27b679d9c2785c54094b0dfcba36a948fbf05373c432afbfffa90ecfb515f3
hashMerkleRoot_be=f315b5cf0ea9ffbfaf32c47353f0fb48a936bafc0d4b09545c78c2d979b627ca
txhash=ca27b679d9c2785c54094b0dfcba36a948fbf05373c432afbfffa90ecfb515f3
txhash_be=f315b5cf0ea9ffbfaf32c47353f0fb48a936bafc0d4b09545c78c2d979b627ca
```

### regtest
```
nTime=1296688602
nBits=0x207fffff
nNonce=3
hashGenesisBlock=4059e22aba6c64d5e973fb2416e926d7c5b750343548991385fb6ac8e9c1fc71
hashGenesisBlock_be=71fcc1e9c86afb85139948353450b7c5d726e91624fb73e9d5646cba2ae25940
hashMerkleRoot=348a580fe27f335304305faa3aa108ea85f1036ffc0ee6ee9f087bbf7e023ecf
hashMerkleRoot_be=cf3e027ebf7b089feee60efc6f03f185ea08a13aaa5f300453337fe20f588a34
txhash=348a580fe27f335304305faa3aa108ea85f1036ffc0ee6ee9f087bbf7e023ecf
txhash_be=cf3e027ebf7b089feee60efc6f03f185ea08a13aaa5f300453337fe20f588a34
```

Notes:
- Mainnet κρατά difficulty-1 (0x1d00ffff) για fair launch / αποφυγή “instamine optics”.
- Testnet/test4/regtest χρησιμοποιούν εύκολο target (0x207fffff) για γρήγορη εξόρυξη και developer iteration.
