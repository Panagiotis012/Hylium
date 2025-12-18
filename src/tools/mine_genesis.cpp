// Mine Hylium genesis blocks (main/testnet/test4/regtest).
// Compile (macOS/Homebrew OpenSSL):
//   c++ -O3 -march=native -funroll-loops -std=c++17 \
//     -I/opt/homebrew/opt/openssl/include -L/opt/homebrew/opt/openssl/lib \
//     src/tools/mine_genesis.cpp -lcrypto -o /tmp/mine_genesis
//
// Run (defaults to all networks, uses hardware_concurrency threads):
//   /tmp/mine_genesis
//
// Optional env:
//   HYLIUM_GENESIS_THREADS=N
//   HYLIUM_GENESIS_NETS=main,testnet,test4,regtest
//   HYLIUM_GENESIS_START=<nonce_offset>
//   HYLIUM_GENESIS_EXTRANONCE_START=<extra_nonce_offset>
//
// Note: Difficulty-1 search may take minutes to hours.

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
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
    // Match CheckProofOfWork: uint256 is interpreted as little-endian, so compare
    // the reversed hash bytes against the big-endian target.
    for (size_t i = 0; i < 32; ++i) {
        const uint8_t hash_be = hash[31 - i];
        const uint8_t target_be = target[i];
        if (hash_be < target_be) return true;
        if (hash_be > target_be) return false;
    }
    return true;
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
    uint32_t version = 1;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&version), reinterpret_cast<uint8_t*>(&version) + 4);
    out.push_back(0x01); // vin count
    out.insert(out.end(), 32, 0x00); // prevout hash
    uint32_t prev_n = 0xffffffff;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&prev_n), reinterpret_cast<uint8_t*>(&prev_n) + 4);

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

    uint32_t seq = 0xffffffff;
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&seq), reinterpret_cast<uint8_t*>(&seq) + 4);
    out.push_back(0x01); // vout count
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&reward), reinterpret_cast<uint8_t*>(&reward) + 8);
    ByteVec spk_len = SerializeVarInt(script_pubkey.size());
    out.insert(out.end(), spk_len.begin(), spk_len.end());
    out.insert(out.end(), script_pubkey.begin(), script_pubkey.end());
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

struct NetSpec {
    std::string name;
    std::string message;
    uint32_t nTime;
    uint32_t nBits;
    bool use_extra_nonce{false};
    uint32_t extra_nonce{0};
};

static void MineGenesis(const NetSpec& spec, unsigned threads)
{
    ByteVec script_pubkey = MakeOpReturn("Hylium genesis");
    auto target = BitsToTarget(spec.nBits);
    const uint32_t extra_start = GetEnvUint("HYLIUM_GENESIS_EXTRANONCE_START", spec.extra_nonce);
    const uint32_t nonce_start = GetEnvUint("HYLIUM_GENESIS_START", 0);

    std::cout << "== " << spec.name << " ==\n";
    std::cout << " nTime=" << spec.nTime << " bits=0x" << std::hex << spec.nBits << std::dec
              << " target=" << HexRev(target) << "\n";
    if (spec.use_extra_nonce) {
        std::cout << " extraNonce start=" << extra_start << "\n";
    }

    for (uint32_t extra_nonce = spec.use_extra_nonce ? extra_start : spec.extra_nonce;; ++extra_nonce) {
        ByteVec tx = SerializeTx(spec.message,
                                 50 * int64_t(100000000),
                                 script_pubkey,
                                 spec.use_extra_nonce,
                                 extra_nonce);
        auto txhash = DoubleSHA256(tx);
        auto mrkl = txhash; // single tx => merkle root equals tx hash

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
            std::cout << " nTime=" << spec.nTime << "\n";
            std::cout << " nNonce=" << nonce << "\n";
            std::cout << " hashGenesisBlock=" << HexRev(found_hash) << "\n";
            std::cout << " merkleRoot=" << HexRev(mrkl) << "\n";
            std::cout << " txhash=" << HexRev(txhash) << "\n";
            std::cout << std::endl;
            return;
        }

        if (!spec.use_extra_nonce) {
            std::cout << "\nNo solution found for fixed coinbase; adjust nonce start or enable extra nonce.\n";
            return;
        }
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
