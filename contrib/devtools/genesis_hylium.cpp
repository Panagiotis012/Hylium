// Genesis finder for Hylium networks (main/test/test4/regtest).
// Builds a Bitcoin-style genesis block but with an OP_RETURN coinbase output
// to avoid any spendable premine. Uses OpenSSL for SHA-256.
//
// Compile:
//   c++ -O3 -std=c++17 contrib/devtools/genesis_hylium.cpp -lcrypto -o genesis_hylium
//
// Run:
//   ./genesis_hylium
//
// This will search for a valid nonce for each network using the target encoded
// in nBits (defaults: 0x1d00ffff for main, 0x207fffff for testnet/test4/regtest).
// Difficulty 1 still requires ~4e9 double-SHA attempts on average; expect
// minutes of CPU time. Use more threads by setting HYLIUM_GENESIS_THREADS=N.

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <openssl/sha.h>

namespace {

using ByteVec = std::vector<uint8_t>;

static std::array<uint8_t, 32> DoubleSHA256(const uint8_t* data, size_t len)
{
    std::array<uint8_t, 32> out{};
    uint8_t tmp[32];
    SHA256(data, len, tmp);
    SHA256(tmp, sizeof(tmp), out.data());
    return out;
}

static std::array<uint8_t, 32> DoubleSHA256(const ByteVec& data)
{
    return DoubleSHA256(data.data(), data.size());
}

static uint32_t ReadBE32(const uint8_t* p)
{
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

static std::array<uint8_t, 32> BitsToTarget(uint32_t nBits)
{
    std::array<uint8_t, 32> target{};
    uint32_t exp = nBits >> 24;
    uint32_t mant = nBits & 0x007fffff;
    if (exp <= 3) {
        mant >>= 8 * (3 - exp);
        target[28] = (mant >> 24) & 0xff;
        target[29] = (mant >> 16) & 0xff;
        target[30] = (mant >> 8) & 0xff;
        target[31] = mant & 0xff;
    } else {
        int offset = exp - 3;
        target[31 - offset] = (mant >> 0) & 0xff;
        target[30 - offset] = (mant >> 8) & 0xff;
        target[29 - offset] = (mant >> 16) & 0xff;
    }
    return target;
}

static bool HashMeetsTarget(const std::array<uint8_t, 32>& hash, const std::array<uint8_t, 32>& target)
{
    // Compare using the same byte order as the node: uint256 stores bytes as
    // produced by SHA256, but CheckProofOfWork interprets the hash as a
    // little-endian integer. This is equivalent to comparing the reversed
    // hash bytes to the target in big-endian form.
    for (size_t i = 0; i < 32; ++i) {
        const uint8_t hash_be = hash[31 - i];
        const uint8_t target_be = target[i];
        if (hash_be < target_be) return true;
        if (hash_be > target_be) return false;
    }
    return true; // equal
}

static ByteVec SerializeVarInt(uint64_t v)
{
    ByteVec out;
    if (v < 0xfd) {
        out.push_back(uint8_t(v));
    } else if (v <= 0xffff) {
        out.push_back(0xfd);
        out.push_back(uint8_t(v));
        out.push_back(uint8_t(v >> 8));
    } else if (v <= 0xffffffff) {
        out.push_back(0xfe);
        for (int i = 0; i < 4; ++i) out.push_back(uint8_t(v >> (8 * i)));
    } else {
        out.push_back(0xff);
        for (int i = 0; i < 8; ++i) out.push_back(uint8_t(v >> (8 * i)));
    }
    return out;
}

static ByteVec SerializeTx(const std::string& message,
                           int64_t reward,
                           const ByteVec& script_pubkey,
                           bool use_extra_nonce,
                           uint32_t extra_nonce)
{
    ByteVec out;
    // Version
    uint32_t version = 1;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&version), reinterpret_cast<uint8_t*>(&version) + 4);
    // vin count
    out.push_back(0x01);
    // prevout
    out.insert(out.end(), 32, 0x00);
    uint32_t prev_n = 0xffffffff;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&prev_n), reinterpret_cast<uint8_t*>(&prev_n) + 4);
    // scriptSig
    ByteVec scriptsig{0x04, 0xff, 0xff, 0x00, 0x1d, 0x01, 0x04};
    scriptsig.insert(scriptsig.end(), message.begin(), message.end());
    if (use_extra_nonce) {
        scriptsig.push_back(0x04); // push 4 bytes
        for (int i = 0; i < 4; ++i) {
            scriptsig.push_back(uint8_t(extra_nonce >> (8 * i)));
        }
    }
    ByteVec scriptsig_len = SerializeVarInt(scriptsig.size());
    out.insert(out.end(), scriptsig_len.begin(), scriptsig_len.end());
    out.insert(out.end(), scriptsig.begin(), scriptsig.end());
    // sequence
    uint32_t seq = 0xffffffff;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&seq), reinterpret_cast<uint8_t*>(&seq) + 4);
    // vout count
    out.push_back(0x01);
    // value
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&reward), reinterpret_cast<uint8_t*>(&reward) + 8);
    // scriptPubKey
    ByteVec spk_len = SerializeVarInt(script_pubkey.size());
    out.insert(out.end(), spk_len.begin(), spk_len.end());
    out.insert(out.end(), script_pubkey.begin(), script_pubkey.end());
    // locktime
    uint32_t lock = 0;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&lock), reinterpret_cast<uint8_t*>(&lock) + 4);
    return out;
}

static ByteVec MakeOpReturn(const std::string& data)
{
    ByteVec out;
    out.push_back(0x6a); // OP_RETURN
    if (data.size() >= 0x4c) {
        throw std::runtime_error("op_return too long");
    }
    out.push_back(uint8_t(data.size()));
    out.insert(out.end(), data.begin(), data.end());
    return out;
}

static std::array<uint8_t, 32> MerkleRoot(const std::array<uint8_t, 32>& txhash)
{
    // Single tx => merkle root = tx hash
    return txhash;
}

struct NetSpec {
    std::string name;
    std::string message;
    uint32_t nTime;
    uint32_t nBits;
    bool use_extra_nonce{false};
    uint32_t extra_nonce{0};
};

struct Result {
    uint32_t nonce{0};
    std::array<uint8_t, 32> hash{};
    std::array<uint8_t, 32> merkle{};
    std::array<uint8_t, 32> txhash{};
};

static std::string HexRev(const std::array<uint8_t, 32>& h)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 31; i >= 0; --i) {
        oss << std::setw(2) << int(h[i]);
    }
    return oss.str();
}

static uint32_t GetEnvUint(const char* name, uint32_t def)
{
    if (const char* v = std::getenv(name)) {
        uint64_t val = std::strtoull(v, nullptr, 0);
        if (val <= 0xffffffffULL) return static_cast<uint32_t>(val);
    }
    return def;
}

static void MineGenesis(const NetSpec& spec, unsigned threads)
{
    ByteVec script_pubkey = MakeOpReturn("Hylium genesis");
    auto target = BitsToTarget(spec.nBits);
    bool use_extra_nonce = spec.use_extra_nonce;
    uint32_t extra_start = spec.extra_nonce;
    if (const char* v = std::getenv("HYLIUM_GENESIS_EXTRANONCE")) {
        use_extra_nonce = std::strtoul(v, nullptr, 0) != 0;
    }
    if (const char* v = std::getenv("HYLIUM_GENESIS_EXTRANONCE_START")) {
        extra_start = static_cast<uint32_t>(std::strtoul(v, nullptr, 0));
    }
    const uint32_t nonce_start = GetEnvUint("HYLIUM_GENESIS_START", 0);

    std::cout << "== " << spec.name << " ==\n";
    std::cout << " nTime=" << spec.nTime << " bits=0x" << std::hex << spec.nBits << std::dec << " target=" << HexRev(target) << "\n";
    if (use_extra_nonce) {
        std::cout << " extraNonce start=" << extra_start << "\n";
    }

    for (uint32_t extra_nonce = use_extra_nonce ? extra_start : spec.extra_nonce;; ++extra_nonce) {
        ByteVec tx = SerializeTx(spec.message,
                                 50 * int64_t(100000000),
                                 script_pubkey,
                                 use_extra_nonce,
                                 extra_nonce);
        auto txhash = DoubleSHA256(tx);
        auto mrkl = MerkleRoot(txhash);

        std::cout << " extraNonce=" << extra_nonce
                  << " txhash=" << HexRev(txhash)
                  << " merkle=" << HexRev(mrkl) << "\n";

        std::atomic<bool> found{false};
        std::atomic<uint32_t> found_nonce{0};
        std::atomic<uint64_t> total_hashes{0};
        std::atomic<uint64_t> interval_hashes{0};
        std::atomic<uint32_t> max_nonce{0};
        std::atomic<int> active{static_cast<int>(threads)};
        std::array<uint8_t, 32> found_hash{};

        auto worker = [&](unsigned tid) {
            if (use_extra_nonce) {
                uint64_t start64 = uint64_t(nonce_start) + tid;
                if (start64 > 0xffffffffULL) {
                    active.fetch_sub(1, std::memory_order_relaxed);
                    return;
                }
                uint64_t max_i = (0xffffffffULL - start64) / threads;
                for (uint64_t i = 0; i <= max_i && !found.load(std::memory_order_relaxed); ++i) {
                    uint32_t nonce = static_cast<uint32_t>(start64 + i * threads);
                    uint8_t header[80];
                    uint32_t version = 1;
                    std::memcpy(header, &version, 4);
                    std::memset(header + 4, 0, 32);
                    std::memcpy(header + 36, mrkl.data(), 32);
                    std::memcpy(header + 68, &spec.nTime, 4);
                    std::memcpy(header + 72, &spec.nBits, 4);
                    std::memcpy(header + 76, &nonce, 4);

                    auto h = DoubleSHA256(header, sizeof(header));
                    if (HashMeetsTarget(h, target)) {
                        if (!found.exchange(true, std::memory_order_relaxed)) {
                            found_nonce.store(nonce, std::memory_order_relaxed);
                            found_hash = h;
                        }
                        break;
                    }
                    total_hashes.fetch_add(1, std::memory_order_relaxed);
                    interval_hashes.fetch_add(1, std::memory_order_relaxed);
                    uint32_t prev = max_nonce.load(std::memory_order_relaxed);
                    while (nonce > prev && !max_nonce.compare_exchange_weak(prev, nonce, std::memory_order_relaxed)) {}
                }
                active.fetch_sub(1, std::memory_order_relaxed);
            } else {
                uint32_t nonce = nonce_start + tid;
                while (!found.load(std::memory_order_relaxed)) {
                    uint8_t header[80];
                    uint32_t version = 1;
                    std::memcpy(header, &version, 4);
                    std::memset(header + 4, 0, 32);
                    std::memcpy(header + 36, mrkl.data(), 32);
                    std::memcpy(header + 68, &spec.nTime, 4);
                    std::memcpy(header + 72, &spec.nBits, 4);
                    std::memcpy(header + 76, &nonce, 4);

                    auto h = DoubleSHA256(header, sizeof(header));
                    if (HashMeetsTarget(h, target)) {
                        if (!found.exchange(true, std::memory_order_relaxed)) {
                            found_nonce.store(nonce, std::memory_order_relaxed);
                            found_hash = h;
                        }
                        break;
                    }
                    nonce += threads;
                    total_hashes.fetch_add(1, std::memory_order_relaxed);
                    interval_hashes.fetch_add(1, std::memory_order_relaxed);
                    uint32_t prev = max_nonce.load(std::memory_order_relaxed);
                    while (nonce > prev && !max_nonce.compare_exchange_weak(prev, nonce, std::memory_order_relaxed)) {}
                }
                active.fetch_sub(1, std::memory_order_relaxed);
            }
        };

        std::vector<std::thread> pool;
        pool.reserve(threads);
        auto t0 = std::chrono::steady_clock::now();
        auto last_stat = t0;
        for (unsigned t = 0; t < threads; ++t) pool.emplace_back(worker, t);

        for (;;) {
            auto now = std::chrono::steady_clock::now();
            double stat_secs = std::chrono::duration<double>(now - last_stat).count();
            if (stat_secs >= 1.0) {
                uint64_t total = total_hashes.load(std::memory_order_relaxed);
                uint64_t delta = interval_hashes.exchange(0, std::memory_order_relaxed);
                double hash_rate = delta / stat_secs;
                uint32_t mx = max_nonce.load(std::memory_order_relaxed);
                std::cout << spec.name << " status: extraNonce=" << extra_nonce
                          << " total=" << total
                          << " rate=" << hash_rate << "/s"
                          << " max_nonce=" << mx << "\r" << std::flush;
                last_stat = now;
            }
            if (found.load(std::memory_order_relaxed) || active.load(std::memory_order_relaxed) == 0) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        for (auto& th : pool) if (th.joinable()) th.join();
        auto t1 = std::chrono::steady_clock::now();

        if (found.load(std::memory_order_relaxed)) {
            double secs = std::chrono::duration<double>(t1 - t0).count();
            uint32_t nonce = found_nonce.load();
            std::cout << "\nFound genesis for " << spec.name << " in " << secs << "s\n";
            std::cout << " extraNonce=" << (use_extra_nonce ? extra_nonce : 0) << "\n";
            std::cout << " nNonce=" << nonce << "\n";
            std::cout << " hashGenesisBlock=" << HexRev(found_hash) << "\n";
            std::cout << " merkleRoot=" << HexRev(mrkl) << "\n";
            std::cout << " txhash=" << HexRev(txhash) << "\n";
            std::cout << std::endl;
            return;
        }

        if (!use_extra_nonce) {
            std::cout << "\nNo solution found for fixed coinbase; set HYLIUM_GENESIS_EXTRANONCE=1 to expand search.\n";
            return;
        }
        // Move to next extraNonce if no solution in this nonce space.
    }
}

} // namespace

int main()
{
    unsigned threads = std::thread::hardware_concurrency();
    if (const char* env = std::getenv("HYLIUM_GENESIS_THREADS")) {
        threads = std::max<unsigned>(1, std::atoi(env));
    }
    if (threads == 0) threads = 1;
    std::cout << "Using " << threads << " threads\n";

    const uint32_t MAIN_TIME = 1765886400; // 2025-12-16 12:00:00 UTC
    const uint32_t BITS_STD = 0x1d00ffff;  // mainnet difficulty-1
    const uint32_t BITS_EZ  = 0x207fffff;  // easy target for test/dev (regtest style)

    std::vector<NetSpec> nets = {
        {"main",   "Hylium 2025-12-16: Verify, don't trust.", MAIN_TIME, BITS_STD, true, 0},
        {"testnet","Hylium testnet 2025-12-16: break safely.", MAIN_TIME, BITS_EZ, false, 0},
        {"test4",  "Hylium test4 2025-12-16: staging chain.", MAIN_TIME, BITS_EZ, false, 0},
        {"regtest","Hylium regtest: local sandbox.", 1296688602, BITS_EZ, false, 0},
    };

    // Optional filter: comma-separated list in HYLIUM_GENESIS_NETS
    std::vector<std::string> filter;
    if (const char* env = std::getenv("HYLIUM_GENESIS_NETS")) {
        std::stringstream ss(env);
        std::string item;
        while (std::getline(ss, item, ',')) {
            if (!item.empty()) filter.push_back(item);
        }
    }
    auto wanted = [&](const std::string& name) {
        if (filter.empty()) return true;
        for (const auto& f : filter) if (f == name) return true;
        return false;
    };

    for (const auto& n : nets) {
        if (!wanted(n.name)) continue;
        MineGenesis(n, threads);
    }
    return 0;
}
