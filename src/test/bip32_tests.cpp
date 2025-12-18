// Copyright (c) 2013-2022 The Hylium Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include <clientversion.h>
#include <key.h>
#include <key_io.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>

#include <string>
#include <vector>

namespace {

struct TestDerivation {
    std::string pub;
    std::string prv;
    unsigned int nChild;
};

struct TestVector {
    std::string strHexMaster;
    std::vector<TestDerivation> vDerive;

    explicit TestVector(std::string strHexMasterIn) : strHexMaster(strHexMasterIn) {}

    TestVector& operator()(std::string pub, std::string prv, unsigned int nChild) {
        vDerive.emplace_back();
        TestDerivation &der = vDerive.back();
        der.pub = pub;
        der.prv = prv;
        der.nChild = nChild;
        return *this;
    }
};

TestVector test1 =
  TestVector("000102030405060708090a0b0c0d0e0f")
    ("ypub6QqdH2c5z7967BioGSfAWFHM1EHzHPBZK7wrND3ZpEWFtzmCqvsD1bgpaE6pSAPkiSKhkuWPCJV6mZTSNMd2tK8xYTcJ48585pZecmSUzWp",
     "ypsts3dbp1Ts4bvm3ZNbf6vZePrwF1mMQVBcRpXqdw87CQCsEnYRSPrzzx4mVmCrdwkRqGKjWSGm2GPwxPsWuBnyBkBd7vYCtRWNe2SEfBuGvVd",
     0x80000000)
    ("ypub6T73GjuZ5NG5FnrWUCXoPHPTL3rLfTfZzjNkLJRgnRhYGH4PGAQJ8k3EMVfXBUJHiecGd93ovwZBjxRaKPMQxCbgk6QYyRyLbkhCvXJ8PtA",
     "ypsts5u1oimL9s3kCAWJrroCXRy3ZqKhnZfd7Rxjc2WEAbQ9c4qbrdQ686RBH2ZC3f8PYKDRsjbdQdEpg9uEYn59ypG9BoDM8PDGM3566KS27AX",
     1)
    ("ypub6VHAUXTSU5996EtxCTRfz2H7PRqSfVPrw43avaCVj4VkVpZWyxpc6JwTma86bR1dgNEKQZVv3Y3fLGAPniXJdrRd3JFQT4ycEVDzDSkyEek",
     "ypsts8591WKDYZvp2cYkb7h58ArhdDJonbPv3kdaCJH37ECMqcLjaRpQ5fKQh5kFeauRzPy7gZC4VLCEp6RTg7s8kykahaB5C8z81qPTdGa12d9",
     0x80000002)
    ("ypub6XtSX4HJAwzYxQh3T3q3M2KFcahxw2yrGbHGAuVetvsBufkdnif2zkVPKbJw3HSx2tL41wUXRUcdnbJgVp4qhJaTdCDYY23HxB98JgFA4d6",
     "ypstsAgR4395FSnDtnLqqi6SVAtqrNBL48yuPHsFSdaCH6ZoFTXrPBepz6sLF7xscpQvVrzWH28yuk293JADG8nKt5CF4BjdM5E4d78gFsiqwxL",
     2)
    ("ypub6a7qMVQFMRfY2tAm9K38EXUZhPzhZdJ3i5AZRs28DNRoGmNzrVMQCKxJzvJKT5bgWCTE9wxUUDWkM1egb7W3Qu3iXymjNCw7xs3yTGrM6Ah",
     "ypstsCuotUG2RvTCyFpZXyJXNg49wBU4gjJ6pmkYhb6fbY8QcZADSxMCBgLFvSNL5UpMZf8CtwNYpZUJWV4fTZKg5TnHTRpiuA3mn5bqmKgxmAQ",
     1000000000)
    ("ypub6bqbqB1VUZ3jZDxDFrbrYEUZVda1P875WKZ7g9QGRSpHytnPNuc4TS7NdkDGJLgPytjmfHtH1RDLwS3hgxztGACqQXBpAgWkmpfthRsw3F3",
     "ypstsEdaN9sGZ3qQVbc1eWsFgP49jR3NWE78d296wsUoocWuKgZbyNbrSnVKZJ9XzPwSB4XTgJMVfc1RWf52zdLQCRRpnvax24oV4RACWQ5tcvD",
     0);

TestVector test2 =
  TestVector("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542")
    ("ypub6QqdH2c5z7966oE8NbjSxT17FRSQy1SNzhjaf9nPncd8BynQ25e9SVrQvLBo9QkiczZVfaAYbNzMVMBfVYYC2E8uQPoktcZnQCEktj3kkN2",
     "ypsts3dbp1Ts4bvm3AsvmFzr6bahVCun67SS7QKZvsrwAnKjXmZccYdwRrEMqpyupTJX5K5AFijDJNS93vbcwcBfo7WPaLcT2mR7DH4U25ztoBQ",
     0)
    ("ypub6U7NYkJ3U7yAd4pRWP7L2FQdcPBffyvfYsUwpxoqs2V5sxENbLzDbLRfVo9gwVm9qK3fXoAHzTqKeWcMNhEcMpoBgSYRnxiAXgMRKyCnZnr",
     "ypsts6uM5j9pYckqZSUDu3NjAPzDrAf2o5vifa4w6gtPFCBhDk1bBoz1agocRLFiTX9MUyJz9j8wjXrGQSESVtgijLfnaX2WRFarLgy2PMDr5zk",
     0xFFFFFFFF)
    ("ypub6VGRoMKZr29Gnw2bvq6yVb6C18QaDU6Me2ZUkMicWmFULFTqDEQXoQDG7BPohENFcpyDBe1uHgFvKBUh9bij5yFpDXAV9gyBQ4d2AhQYrSN",
     "ypsts84QLLBLvWvwjJgQKVNNdjfnEuswLa6Qkj9U25o9tvx5g3F3ohQKnkbD2gjhBmsqPCdsvkLanYMNqfyDEGHrJe7M1FFrBmK78UJZ9qddeJf",
     1)
    ("ypub6Y5QDNJW1fSTtYhjzVFYTY2RLfH5Kf1cxU4LbRNwpebv15KzWjwEhJbdwyawV74YD1Rm2H78voZkPyji4W9gMJiiooNLNEQPKqHyRAH3bGK",
     "ypstsAsNkMAH6AE8pvMYP9Wwbgc1aSkSSm1g5AeKs9TVCpJXLs7D7Cw2geyasUwKDV5Xh9zag3QyC62bANkNH9pEK7ve91nf2sHVcGjHdS6QAcT",
     0xFFFFFFFE)
    ("ypub6ZFS8LErd4QBBVwLiy55q3icFPkSNde7zAnE895MfQsATRevcQsFDpxjadiSgUazAT5PpnAqyQtc3cu6kPGC4fRB5hgpvXVoC4NYx7B2Uih",
     "ypstsC3QfK6dhZBr7sb97dLUyCJCVBDoVjeB6sNDPs9u3aZmoDS9Css3DBLgWBfhquTpEjHGysBxaTcAyVqjo6iizxboxxKyB5K7wrjohCTCJDV",
     2)
    ("ypub6acU5mTN9EiQvm8ET3Cmoo6aeTHYcxZCeLQ7XvRrJCikXczVn3o1QPujf2nip3wggWHd6hgFs7ubY3xbaUx6NvGkisWurtnXsMZjcq97WJh",
     "ypstsDQSckK9DjW5s8n2qhUAwwgAtEkuk4ZFm2z6oeWPgNRMsQmiNWnoPkHgabLhzvYtH391Gcpn2ZSs23jFXR5x3wLXmoqUEVSqeE5Mmopw3UX",
     0);

TestVector test3 =
  TestVector("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be")
    ("ypub6QqdH2c5z7965rgHtzEFASG984mWisbei3W94Sox4JhvAE1fgDt7Pm5wQKimy1LMo5KMVa3Yz3pCvepaLFadSZo3XdhydB1dBukTRDQdQtm",
     "ypsts3dbp1Ts4bvm2EL6HeVeJaqjMrEsqybhpk68LAtVSUQXW1ntGgsuP7TtKpi5mUXrA1jupeRKCoArQaCwB3L6ab1e6dB6ECm5Vzipg4UGri6",
      0x80000000)
    ("ypub6TCq1zSDfUWSHPmh88ZkDh1opRNtsj1Z7Xk5sgVnzcdHRF98FieyUDsei3ZHhMw6zSpCGXoHq11hZ7kvGVWp1p4zCjesB2WRg67N5d6C6UQ",
     "ypsts5zoYyHzjyJ7DmRVWnq9MqbQ4CrFzq1cEEL59QaLNnKtm2vLrBemTaFbdb4S7RrmcAPKqbDBrXoK47r5kFurLMDGXm68yN2td1gUdNtPdpn",
      0);

TestVector test4 =
  TestVector("3ddd5602285899a946114506157c7997e5444528f3003f6134712147db19b678")
    ("ypub6QqdH2c5z7967vBrjib3yBj2kJMge81PxBCYSBVXvqYc17pzt8Usbye36SySzSvRNcSueFGjvpawjXbhkTbYXRexJ414BUyS6WRgW5uBSVq",
     "ypsts3dbp1Ts4bvm4Hqf8NrT7LJcz5q3mE1T4snXhua5K1FDLucDUbUfbL1z1yzCnCw2jUhXvqJT98Nd3aqDq8b1fQg2bs38H5ejn8aySDRDyLS",
     0x80000000)
    ("ypub6TzjfQikMsFXkoCzN3WpyapoCEExMRXwhUpCn9fWaHcaVet7STubk2KPG3FD25HYniDGbHfMXkXCGTHmTF3wcjhzLNS24J4p9JyjJVJd4ix",
     "ypsts6niCPaXSN3ChArnkhnE7jQPS1iKUXXzpBQC3sk3xTKBqSfL2vuPjNhLBXuSq27PhTiyTQUgTaUhzF2KSWFeg3YxTFLoV16jmPcnB6oE69E",
     0x80000001)
    ("ypub6W8RKQ7ksbJ8Rx4dWKRj5TQeNAR2rMEkMQs2YuBiEUi2GkCdJxPJy9HG9z1jsFvENVpdV4bTzCsBEwcfuqs6L2eMgAH522Ym1jLdN2B8Eyw",
     "ypsts8vPrNyXx65oNKiRtyh8DbzEbwtPyTEoU7T1pdGFceQdcXyquRP6xVfD5VsXtR9DnE85e1JeEDaahCr2qSCmxS2Bhb3pb3M4d5AUGdPe8Dv",
     0);

const std::vector<std::string> TEST5 = {
    "xpub661MyMwAqRbcEYS8w7XLSVeEsBXy79zSzH1J8vCdxAZningWLdN3zgtU6LBpB85b3D2yc8sfvZU521AAwdZafEz7mnzBBsz4wKY5fTtTQBm",
    "xprv9s21ZrQH143K24Mfq5zL5MhWK9hUhhGbd45hLXo2Pq2oqzMMo63oStZzFGTQQD3dC4H2D5GBj7vWvSQaaBv5cxi9gafk7NF3pnBju6dwKvH",
    "xpub661MyMwAqRbcEYS8w7XLSVeEsBXy79zSzH1J8vCdxAZningWLdN3zgtU6Txnt3siSujt9RCVYsx4qHZGc62TG4McvMGcAUjeuwZdduYEvFn",
    "xprv9s21ZrQH143K24Mfq5zL5MhWK9hUhhGbd45hLXo2Pq2oqzMMo63oStZzFGpWnsj83BHtEy5Zt8CcDr1UiRXuWCmTQLxEK9vbz5gPstX92JQ",
    "xpub661MyMwAqRbcEYS8w7XLSVeEsBXy79zSzH1J8vCdxAZningWLdN3zgtU6N8ZMMXctdiCjxTNq964yKkwrkBJJwpzZS4HS2fxvyYUA4q2Xe4",
    "xprv9s21ZrQH143K24Mfq5zL5MhWK9hUhhGbd45hLXo2Pq2oqzMMo63oStZzFAzHGBP2UuGCqWLTAPLcMtD9y5gkZ6Eq3Rjuahrv17fEQ3Qen6J",
    "xprv9s2SPatNQ9Vc6GTbVMFPFo7jsaZySyzk7L8n2uqKXJen3KUmvQNTuLh3fhZMBoG3G4ZW1N2kZuHEPY53qmbZzCHshoQnNf4GvELZfqTUrcv",
    "xpub661no6RGEX3uJkY4bNnPcw4URcQTrSibUZ4NqJEw5eBkv7ovTwgiT91XX27VbEXGENhYRCf7hyEbWrR3FewATdCEebj6znwMfQkhRYHRLpJ",
    "xprv9s21ZrQH4r4TsiLvyLXqM9P7k1K3EYhA1kkD6xuquB5i39AU8KF42acDyL3qsDbU9NmZn6MsGSUYZEsuoePmjzsB3eFKSUEh3Gu1N3cqVUN",
    "xpub661MyMwAuDcm6CRQ5N4qiHKrJ39Xe1R1NyfouMKTTWcguwVcfrZJaNvhpebzGerh7gucBvzEQWRugZDuDXjNDRmXzSZe4c7mnTK97pTvGS8",
    "DMwo58pR1QLEFihHiXPVykYB6fJmsTeHvyTp7hRThAtCX8CvYzgPcn8XnmdfHGMQzT7ayAmfo4z3gY5KfbrZWZ6St24UVf2Qgo6oujFktLHdHY4",
    "DMwo58pR1QLEFihHiXPVykYB6fJmsTeHvyTp7hRThAtCX8CvYzgPcn8XnmdfHPmHJiEDXkTiJTVV9rHEBUem2mwVbbNfvT2MTcAqj3nesx8uBf9",
    "xprv9s21ZrQH143K24Mfq5zL5MhWK9hUhhGbd45hLXo2Pq2oqzMMo63oStZzF93Y5wvzdUayhgkkFoicQZcP3y52uPPxFnfoLZB21Teqt1VvEHx",
    "xprv9s21ZrQH143K24Mfq5zL5MhWK9hUhhGbd45hLXo2Pq2oqzMMo63oStZzFAzHGBP2UuGCqWLTAPLcMtD5SDKr24z3aiUvKr9bJpdrcLg1y3G",
    "xpub661MyMwAqRbcEYS8w7XLSVeEsBXy79zSzH1J8vCdxAZningWLdN3zgtU6Q5JXayek4PRsn35jii4veMimro1xefsM58PgBMrvdYre8QyULY",
    "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHL"
};

void RunTest(const TestVector& test)
{
    std::vector<std::byte> seed{ParseHex<std::byte>(test.strHexMaster)};
    CExtKey key;
    CExtPubKey pubkey;
    key.SetSeed(seed);
    pubkey = key.Neuter();
    for (const TestDerivation &derive : test.vDerive) {
        unsigned char data[74];
        key.Encode(data);
        pubkey.Encode(data);

        // Test private key
        BOOST_CHECK(EncodeExtKey(key) == derive.prv);
        BOOST_CHECK(DecodeExtKey(derive.prv) == key); //ensure a base58 decoded key also matches

        // Test public key
        BOOST_CHECK(EncodeExtPubKey(pubkey) == derive.pub);
        BOOST_CHECK(DecodeExtPubKey(derive.pub) == pubkey); //ensure a base58 decoded pubkey also matches

        // Derive new keys
        CExtKey keyNew;
        BOOST_CHECK(key.Derive(keyNew, derive.nChild));
        CExtPubKey pubkeyNew = keyNew.Neuter();
        if (!(derive.nChild & 0x80000000)) {
            // Compare with public derivation
            CExtPubKey pubkeyNew2;
            BOOST_CHECK(pubkey.Derive(pubkeyNew2, derive.nChild));
            BOOST_CHECK(pubkeyNew == pubkeyNew2);
        }
        key = keyNew;
        pubkey = pubkeyNew;
    }
}

}  // namespace

BOOST_FIXTURE_TEST_SUITE(bip32_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bip32_test1) {
    RunTest(test1);
}

BOOST_AUTO_TEST_CASE(bip32_test2) {
    RunTest(test2);
}

BOOST_AUTO_TEST_CASE(bip32_test3) {
    RunTest(test3);
}

BOOST_AUTO_TEST_CASE(bip32_test4) {
    RunTest(test4);
}

BOOST_AUTO_TEST_CASE(bip32_test5) {
    for (const auto& str : TEST5) {
        auto dec_extkey = DecodeExtKey(str);
        auto dec_extpubkey = DecodeExtPubKey(str);
        BOOST_CHECK_MESSAGE(!dec_extkey.key.IsValid(), "Decoding '" + str + "' as xprv should fail");
        BOOST_CHECK_MESSAGE(!dec_extpubkey.pubkey.IsValid(), "Decoding '" + str + "' as xpub should fail");
    }
}

BOOST_AUTO_TEST_CASE(bip32_max_depth) {
    CExtKey key_parent{DecodeExtKey(test1.vDerive[0].prv)}, key_child;
    CExtPubKey pubkey_parent{DecodeExtPubKey(test1.vDerive[0].pub)}, pubkey_child;

    // We can derive up to the 255th depth..
    for (auto i = 0; i++ < 255;) {
        BOOST_CHECK(key_parent.Derive(key_child, 0));
        std::swap(key_parent, key_child);
        BOOST_CHECK(pubkey_parent.Derive(pubkey_child, 0));
        std::swap(pubkey_parent, pubkey_child);
    }

    // But trying to derive a non-existent 256th depth will fail!
    BOOST_CHECK(key_parent.nDepth == 255 && pubkey_parent.nDepth == 255);
    BOOST_CHECK(!key_parent.Derive(key_child, 0));
    BOOST_CHECK(!pubkey_parent.Derive(pubkey_child, 0));
}

BOOST_AUTO_TEST_SUITE_END()
