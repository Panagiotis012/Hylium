// Copyright (c) 2022-present The Hylium Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define HYLIUMKERNEL_BUILD

#include <kernel/hyliumkernel.h>

#include <chain.h>
#include <coins.h>
#include <consensus/amount.h>
#include <consensus/validation.h>
#include <kernel/caches.h>
#include <kernel/chainparams.h>
#include <kernel/checks.h>
#include <kernel/context.h>
#include <kernel/cs_main.h>
#include <kernel/notifications_interface.h>
#include <kernel/warning.h>
#include <logging.h>
#include <node/blockstorage.h>
#include <node/chainstate.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <serialize.h>
#include <streams.h>
#include <sync.h>
#include <tinyformat.h>
#include <uint256.h>
#include <undo.h>
#include <util/fs.h>
#include <util/result.h>
#include <util/signalinterrupt.h>
#include <util/task_runner.h>
#include <util/translation.h>
#include <validation.h>
#include <validationinterface.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using util::ImmediateTaskRunner;

// Define G_TRANSLATION_FUN symbol in libhyliumkernel library so users of the
// library aren't required to export this symbol
extern const std::function<std::string(const char*)> G_TRANSLATION_FUN{nullptr};

static const kernel::Context hylk_context_static{};

namespace {

bool is_valid_flag_combination(script_verify_flags flags)
{
    if (flags & SCRIPT_VERIFY_CLEANSTACK && ~flags & (SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS)) return false;
    if (flags & SCRIPT_VERIFY_WITNESS && ~flags & SCRIPT_VERIFY_P2SH) return false;
    return true;
}

class WriterStream
{
private:
    hylk_WriteBytes m_writer;
    void* m_user_data;

public:
    WriterStream(hylk_WriteBytes writer, void* user_data)
        : m_writer{writer}, m_user_data{user_data} {}

    //
    // Stream subset
    //
    void write(std::span<const std::byte> src)
    {
        if (m_writer(std::data(src), src.size(), m_user_data) != 0) {
            throw std::runtime_error("Failed to write serialization data");
        }
    }

    template <typename T>
    WriterStream& operator<<(const T& obj)
    {
        ::Serialize(*this, obj);
        return *this;
    }
};

template <typename C, typename CPP>
struct Handle {
    static C* ref(CPP* cpp_type)
    {
        return reinterpret_cast<C*>(cpp_type);
    }

    static const C* ref(const CPP* cpp_type)
    {
        return reinterpret_cast<const C*>(cpp_type);
    }

    template <typename... Args>
    static C* create(Args&&... args)
    {
        auto cpp_obj{std::make_unique<CPP>(std::forward<Args>(args)...)};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static C* copy(const C* ptr)
    {
        auto cpp_obj{std::make_unique<CPP>(get(ptr))};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static const CPP& get(const C* ptr)
    {
        return *reinterpret_cast<const CPP*>(ptr);
    }

    static CPP& get(C* ptr)
    {
        return *reinterpret_cast<CPP*>(ptr);
    }

    static void operator delete(void* ptr)
    {
        delete reinterpret_cast<CPP*>(ptr);
    }
};

} // namespace

struct hylk_BlockTreeEntry: Handle<hylk_BlockTreeEntry, CBlockIndex> {};
struct hylk_Block : Handle<hylk_Block, std::shared_ptr<const CBlock>> {};
struct hylk_BlockValidationState : Handle<hylk_BlockValidationState, BlockValidationState> {};

namespace {

BCLog::Level get_bclog_level(hylk_LogLevel level)
{
    switch (level) {
    case hylk_LogLevel_INFO: {
        return BCLog::Level::Info;
    }
    case hylk_LogLevel_DEBUG: {
        return BCLog::Level::Debug;
    }
    case hylk_LogLevel_TRACE: {
        return BCLog::Level::Trace;
    }
    }
    assert(false);
}

BCLog::LogFlags get_bclog_flag(hylk_LogCategory category)
{
    switch (category) {
    case hylk_LogCategory_BENCH: {
        return BCLog::LogFlags::BENCH;
    }
    case hylk_LogCategory_BLOCKSTORAGE: {
        return BCLog::LogFlags::BLOCKSTORAGE;
    }
    case hylk_LogCategory_COINDB: {
        return BCLog::LogFlags::COINDB;
    }
    case hylk_LogCategory_LEVELDB: {
        return BCLog::LogFlags::LEVELDB;
    }
    case hylk_LogCategory_MEMPOOL: {
        return BCLog::LogFlags::MEMPOOL;
    }
    case hylk_LogCategory_PRUNE: {
        return BCLog::LogFlags::PRUNE;
    }
    case hylk_LogCategory_RAND: {
        return BCLog::LogFlags::RAND;
    }
    case hylk_LogCategory_REINDEX: {
        return BCLog::LogFlags::REINDEX;
    }
    case hylk_LogCategory_VALIDATION: {
        return BCLog::LogFlags::VALIDATION;
    }
    case hylk_LogCategory_KERNEL: {
        return BCLog::LogFlags::KERNEL;
    }
    case hylk_LogCategory_ALL: {
        return BCLog::LogFlags::ALL;
    }
    }
    assert(false);
}

hylk_SynchronizationState cast_state(SynchronizationState state)
{
    switch (state) {
    case SynchronizationState::INIT_REINDEX:
        return hylk_SynchronizationState_INIT_REINDEX;
    case SynchronizationState::INIT_DOWNLOAD:
        return hylk_SynchronizationState_INIT_DOWNLOAD;
    case SynchronizationState::POST_INIT:
        return hylk_SynchronizationState_POST_INIT;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

hylk_Warning cast_hylk_warning(kernel::Warning warning)
{
    switch (warning) {
    case kernel::Warning::UNKNOWN_NEW_RULES_ACTIVATED:
        return hylk_Warning_UNKNOWN_NEW_RULES_ACTIVATED;
    case kernel::Warning::LARGE_WORK_INVALID_CHAIN:
        return hylk_Warning_LARGE_WORK_INVALID_CHAIN;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

struct LoggingConnection {
    std::unique_ptr<std::list<std::function<void(const std::string&)>>::iterator> m_connection;
    void* m_user_data;
    std::function<void(void* user_data)> m_deleter;

    LoggingConnection(hylk_LogCallback callback, void* user_data, hylk_DestroyCallback user_data_destroy_callback)
    {
        LOCK(cs_main);

        auto connection{LogInstance().PushBackCallback([callback, user_data](const std::string& str) { callback(user_data, str.c_str(), str.length()); })};

        // Only start logging if we just added the connection.
        if (LogInstance().NumConnections() == 1 && !LogInstance().StartLogging()) {
            LogError("Logger start failed.");
            LogInstance().DeleteCallback(connection);
            if (user_data && user_data_destroy_callback) {
                user_data_destroy_callback(user_data);
            }
            throw std::runtime_error("Failed to start logging");
        }

        m_connection = std::make_unique<std::list<std::function<void(const std::string&)>>::iterator>(connection);
        m_user_data = user_data;
        m_deleter = user_data_destroy_callback;

        LogDebug(BCLog::KERNEL, "Logger connected.");
    }

    ~LoggingConnection()
    {
        LOCK(cs_main);
        LogDebug(BCLog::KERNEL, "Logger disconnecting.");

        // Switch back to buffering by calling DisconnectTestLogger if the
        // connection that we are about to remove is the last one.
        if (LogInstance().NumConnections() == 1) {
            LogInstance().DisconnectTestLogger();
        } else {
            LogInstance().DeleteCallback(*m_connection);
        }

        m_connection.reset();
        if (m_user_data && m_deleter) {
            m_deleter(m_user_data);
        }
    }
};

class KernelNotifications final : public kernel::Notifications
{
private:
    hylk_NotificationInterfaceCallbacks m_cbs;

public:
    KernelNotifications(hylk_NotificationInterfaceCallbacks cbs)
        : m_cbs{cbs}
    {
    }

    ~KernelNotifications()
    {
        if (m_cbs.user_data && m_cbs.user_data_destroy) {
            m_cbs.user_data_destroy(m_cbs.user_data);
        }
        m_cbs.user_data_destroy = nullptr;
        m_cbs.user_data = nullptr;
    }

    kernel::InterruptResult blockTip(SynchronizationState state, const CBlockIndex& index, double verification_progress) override
    {
        if (m_cbs.block_tip) m_cbs.block_tip(m_cbs.user_data, cast_state(state), hylk_BlockTreeEntry::ref(&index), verification_progress);
        return {};
    }
    void headerTip(SynchronizationState state, int64_t height, int64_t timestamp, bool presync) override
    {
        if (m_cbs.header_tip) m_cbs.header_tip(m_cbs.user_data, cast_state(state), height, timestamp, presync ? 1 : 0);
    }
    void progress(const bilingual_str& title, int progress_percent, bool resume_possible) override
    {
        if (m_cbs.progress) m_cbs.progress(m_cbs.user_data, title.original.c_str(), title.original.length(), progress_percent, resume_possible ? 1 : 0);
    }
    void warningSet(kernel::Warning id, const bilingual_str& message) override
    {
        if (m_cbs.warning_set) m_cbs.warning_set(m_cbs.user_data, cast_hylk_warning(id), message.original.c_str(), message.original.length());
    }
    void warningUnset(kernel::Warning id) override
    {
        if (m_cbs.warning_unset) m_cbs.warning_unset(m_cbs.user_data, cast_hylk_warning(id));
    }
    void flushError(const bilingual_str& message) override
    {
        if (m_cbs.flush_error) m_cbs.flush_error(m_cbs.user_data, message.original.c_str(), message.original.length());
    }
    void fatalError(const bilingual_str& message) override
    {
        if (m_cbs.fatal_error) m_cbs.fatal_error(m_cbs.user_data, message.original.c_str(), message.original.length());
    }
};

class KernelValidationInterface final : public CValidationInterface
{
public:
    hylk_ValidationInterfaceCallbacks m_cbs;

    explicit KernelValidationInterface(const hylk_ValidationInterfaceCallbacks vi_cbs) : m_cbs{vi_cbs} {}

    ~KernelValidationInterface()
    {
        if (m_cbs.user_data && m_cbs.user_data_destroy) {
            m_cbs.user_data_destroy(m_cbs.user_data);
        }
        m_cbs.user_data = nullptr;
        m_cbs.user_data_destroy = nullptr;
    }

protected:
    void BlockChecked(const std::shared_ptr<const CBlock>& block, const BlockValidationState& stateIn) override
    {
        if (m_cbs.block_checked) {
            m_cbs.block_checked(m_cbs.user_data,
                                hylk_Block::copy(hylk_Block::ref(&block)),
                                hylk_BlockValidationState::ref(&stateIn));
        }
    }

    void NewPoWValidBlock(const CBlockIndex* pindex, const std::shared_ptr<const CBlock>& block) override
    {
        if (m_cbs.pow_valid_block) {
            m_cbs.pow_valid_block(m_cbs.user_data,
                                  hylk_Block::copy(hylk_Block::ref(&block)),
                                  hylk_BlockTreeEntry::ref(pindex));
        }
    }

    void BlockConnected(ChainstateRole role, const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override
    {
        if (m_cbs.block_connected) {
            m_cbs.block_connected(m_cbs.user_data,
                                  hylk_Block::copy(hylk_Block::ref(&block)),
                                  hylk_BlockTreeEntry::ref(pindex));
        }
    }

    void BlockDisconnected(const std::shared_ptr<const CBlock>& block, const CBlockIndex* pindex) override
    {
        if (m_cbs.block_disconnected) {
            m_cbs.block_disconnected(m_cbs.user_data,
                                     hylk_Block::copy(hylk_Block::ref(&block)),
                                     hylk_BlockTreeEntry::ref(pindex));
        }
    }
};

struct ContextOptions {
    mutable Mutex m_mutex;
    std::unique_ptr<const CChainParams> m_chainparams GUARDED_BY(m_mutex);
    std::shared_ptr<KernelNotifications> m_notifications GUARDED_BY(m_mutex);
    std::shared_ptr<KernelValidationInterface> m_validation_interface GUARDED_BY(m_mutex);
};

class Context
{
public:
    std::unique_ptr<kernel::Context> m_context;

    std::shared_ptr<KernelNotifications> m_notifications;

    std::unique_ptr<util::SignalInterrupt> m_interrupt;

    std::unique_ptr<ValidationSignals> m_signals;

    std::unique_ptr<const CChainParams> m_chainparams;

    std::shared_ptr<KernelValidationInterface> m_validation_interface;

    Context(const ContextOptions* options, bool& sane)
        : m_context{std::make_unique<kernel::Context>()},
          m_interrupt{std::make_unique<util::SignalInterrupt>()}
    {
        if (options) {
            LOCK(options->m_mutex);
            if (options->m_chainparams) {
                m_chainparams = std::make_unique<const CChainParams>(*options->m_chainparams);
            }
            if (options->m_notifications) {
                m_notifications = options->m_notifications;
            }
            if (options->m_validation_interface) {
                m_signals = std::make_unique<ValidationSignals>(std::make_unique<ImmediateTaskRunner>());
                m_validation_interface = options->m_validation_interface;
                m_signals->RegisterSharedValidationInterface(m_validation_interface);
            }
        }

        if (!m_chainparams) {
            m_chainparams = CChainParams::Main();
        }
        if (!m_notifications) {
            m_notifications = std::make_shared<KernelNotifications>(hylk_NotificationInterfaceCallbacks{
                nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        }

        if (!kernel::SanityChecks(*m_context)) {
            sane = false;
        }
    }

    ~Context()
    {
        if (m_signals) {
            m_signals->UnregisterSharedValidationInterface(m_validation_interface);
        }
    }
};

//! Helper struct to wrap the ChainstateManager-related Options
struct ChainstateManagerOptions {
    mutable Mutex m_mutex;
    ChainstateManager::Options m_chainman_options GUARDED_BY(m_mutex);
    node::BlockManager::Options m_blockman_options GUARDED_BY(m_mutex);
    std::shared_ptr<const Context> m_context;
    node::ChainstateLoadOptions m_chainstate_load_options GUARDED_BY(m_mutex);

    ChainstateManagerOptions(const std::shared_ptr<const Context>& context, const fs::path& data_dir, const fs::path& blocks_dir)
        : m_chainman_options{ChainstateManager::Options{
              .chainparams = *context->m_chainparams,
              .datadir = data_dir,
              .notifications = *context->m_notifications,
              .signals = context->m_signals.get()}},
          m_blockman_options{node::BlockManager::Options{
              .chainparams = *context->m_chainparams,
              .blocks_dir = blocks_dir,
              .notifications = *context->m_notifications,
              .block_tree_db_params = DBParams{
                  .path = data_dir / "blocks" / "index",
                  .cache_bytes = kernel::CacheSizes{DEFAULT_KERNEL_CACHE}.block_tree_db,
              }}},
          m_context{context}, m_chainstate_load_options{node::ChainstateLoadOptions{}}
    {
    }
};

struct ChainMan {
    std::unique_ptr<ChainstateManager> m_chainman;
    std::shared_ptr<const Context> m_context;

    ChainMan(std::unique_ptr<ChainstateManager> chainman, std::shared_ptr<const Context> context)
        : m_chainman(std::move(chainman)), m_context(std::move(context)) {}
};

} // namespace

struct hylk_Transaction : Handle<hylk_Transaction, std::shared_ptr<const CTransaction>> {};
struct hylk_TransactionOutput : Handle<hylk_TransactionOutput, CTxOut> {};
struct hylk_ScriptPubkey : Handle<hylk_ScriptPubkey, CScript> {};
struct hylk_LoggingConnection : Handle<hylk_LoggingConnection, LoggingConnection> {};
struct hylk_ContextOptions : Handle<hylk_ContextOptions, ContextOptions> {};
struct hylk_Context : Handle<hylk_Context, std::shared_ptr<const Context>> {};
struct hylk_ChainParameters : Handle<hylk_ChainParameters, CChainParams> {};
struct hylk_ChainstateManagerOptions : Handle<hylk_ChainstateManagerOptions, ChainstateManagerOptions> {};
struct hylk_ChainstateManager : Handle<hylk_ChainstateManager, ChainMan> {};
struct hylk_Chain : Handle<hylk_Chain, CChain> {};
struct hylk_BlockSpentOutputs : Handle<hylk_BlockSpentOutputs, std::shared_ptr<CBlockUndo>> {};
struct hylk_TransactionSpentOutputs : Handle<hylk_TransactionSpentOutputs, CTxUndo> {};
struct hylk_Coin : Handle<hylk_Coin, Coin> {};
struct hylk_BlockHash : Handle<hylk_BlockHash, uint256> {};
struct hylk_TransactionInput : Handle<hylk_TransactionInput, CTxIn> {};
struct hylk_TransactionOutPoint: Handle<hylk_TransactionOutPoint, COutPoint> {};
struct hylk_Txid: Handle<hylk_Txid, Txid> {};

hylk_Transaction* hylk_transaction_create(const void* raw_transaction, size_t raw_transaction_len)
{
    if (raw_transaction == nullptr && raw_transaction_len != 0) {
        return nullptr;
    }
    try {
        DataStream stream{std::span{reinterpret_cast<const std::byte*>(raw_transaction), raw_transaction_len}};
        return hylk_Transaction::create(std::make_shared<const CTransaction>(deserialize, TX_WITH_WITNESS, stream));
    } catch (...) {
        return nullptr;
    }
}

size_t hylk_transaction_count_outputs(const hylk_Transaction* transaction)
{
    return hylk_Transaction::get(transaction)->vout.size();
}

const hylk_TransactionOutput* hylk_transaction_get_output_at(const hylk_Transaction* transaction, size_t output_index)
{
    const CTransaction& tx = *hylk_Transaction::get(transaction);
    assert(output_index < tx.vout.size());
    return hylk_TransactionOutput::ref(&tx.vout[output_index]);
}

size_t hylk_transaction_count_inputs(const hylk_Transaction* transaction)
{
    return hylk_Transaction::get(transaction)->vin.size();
}

const hylk_TransactionInput* hylk_transaction_get_input_at(const hylk_Transaction* transaction, size_t input_index)
{
    assert(input_index < hylk_Transaction::get(transaction)->vin.size());
    return hylk_TransactionInput::ref(&hylk_Transaction::get(transaction)->vin[input_index]);
}

const hylk_Txid* hylk_transaction_get_txid(const hylk_Transaction* transaction)
{
    return hylk_Txid::ref(&hylk_Transaction::get(transaction)->GetHash());
}

hylk_Transaction* hylk_transaction_copy(const hylk_Transaction* transaction)
{
    return hylk_Transaction::copy(transaction);
}

int hylk_transaction_to_bytes(const hylk_Transaction* transaction, hylk_WriteBytes writer, void* user_data)
{
    try {
        WriterStream ws{writer, user_data};
        ws << TX_WITH_WITNESS(hylk_Transaction::get(transaction));
        return 0;
    } catch (...) {
        return -1;
    }
}

void hylk_transaction_destroy(hylk_Transaction* transaction)
{
    delete transaction;
}

hylk_ScriptPubkey* hylk_script_pubkey_create(const void* script_pubkey, size_t script_pubkey_len)
{
    if (script_pubkey == nullptr && script_pubkey_len != 0) {
        return nullptr;
    }
    auto data = std::span{reinterpret_cast<const uint8_t*>(script_pubkey), script_pubkey_len};
    return hylk_ScriptPubkey::create(data.begin(), data.end());
}

int hylk_script_pubkey_to_bytes(const hylk_ScriptPubkey* script_pubkey_, hylk_WriteBytes writer, void* user_data)
{
    const auto& script_pubkey{hylk_ScriptPubkey::get(script_pubkey_)};
    return writer(script_pubkey.data(), script_pubkey.size(), user_data);
}

hylk_ScriptPubkey* hylk_script_pubkey_copy(const hylk_ScriptPubkey* script_pubkey)
{
    return hylk_ScriptPubkey::copy(script_pubkey);
}

void hylk_script_pubkey_destroy(hylk_ScriptPubkey* script_pubkey)
{
    delete script_pubkey;
}

hylk_TransactionOutput* hylk_transaction_output_create(const hylk_ScriptPubkey* script_pubkey, int64_t amount)
{
    return hylk_TransactionOutput::create(amount, hylk_ScriptPubkey::get(script_pubkey));
}

hylk_TransactionOutput* hylk_transaction_output_copy(const hylk_TransactionOutput* output)
{
    return hylk_TransactionOutput::copy(output);
}

const hylk_ScriptPubkey* hylk_transaction_output_get_script_pubkey(const hylk_TransactionOutput* output)
{
    return hylk_ScriptPubkey::ref(&hylk_TransactionOutput::get(output).scriptPubKey);
}

int64_t hylk_transaction_output_get_amount(const hylk_TransactionOutput* output)
{
    return hylk_TransactionOutput::get(output).nValue;
}

void hylk_transaction_output_destroy(hylk_TransactionOutput* output)
{
    delete output;
}

int hylk_script_pubkey_verify(const hylk_ScriptPubkey* script_pubkey,
                              const int64_t amount,
                              const hylk_Transaction* tx_to,
                              const hylk_TransactionOutput** spent_outputs_, size_t spent_outputs_len,
                              const unsigned int input_index,
                              const hylk_ScriptVerificationFlags flags,
                              hylk_ScriptVerifyStatus* status)
{
    // Assert that all specified flags are part of the interface before continuing
    assert((flags & ~hylk_ScriptVerificationFlags_ALL) == 0);

    if (!is_valid_flag_combination(script_verify_flags::from_int(flags))) {
        if (status) *status = hylk_ScriptVerifyStatus_ERROR_INVALID_FLAGS_COMBINATION;
        return 0;
    }

    if (flags & hylk_ScriptVerificationFlags_TAPROOT && spent_outputs_ == nullptr) {
        if (status) *status = hylk_ScriptVerifyStatus_ERROR_SPENT_OUTPUTS_REQUIRED;
        return 0;
    }

    if (status) *status = hylk_ScriptVerifyStatus_OK;

    const CTransaction& tx{*hylk_Transaction::get(tx_to)};
    std::vector<CTxOut> spent_outputs;
    if (spent_outputs_ != nullptr) {
        assert(spent_outputs_len == tx.vin.size());
        spent_outputs.reserve(spent_outputs_len);
        for (size_t i = 0; i < spent_outputs_len; i++) {
            const CTxOut& tx_out{hylk_TransactionOutput::get(spent_outputs_[i])};
            spent_outputs.push_back(tx_out);
        }
    }

    assert(input_index < tx.vin.size());
    PrecomputedTransactionData txdata{tx};

    if (spent_outputs_ != nullptr && flags & hylk_ScriptVerificationFlags_TAPROOT) {
        txdata.Init(tx, std::move(spent_outputs));
    }

    bool result = VerifyScript(tx.vin[input_index].scriptSig,
                               hylk_ScriptPubkey::get(script_pubkey),
                               &tx.vin[input_index].scriptWitness,
                               script_verify_flags::from_int(flags),
                               TransactionSignatureChecker(&tx, input_index, amount, txdata, MissingDataBehavior::FAIL),
                               nullptr);
    return result ? 1 : 0;
}

hylk_TransactionInput* hylk_transaction_input_copy(const hylk_TransactionInput* input)
{
    return hylk_TransactionInput::copy(input);
}

const hylk_TransactionOutPoint* hylk_transaction_input_get_out_point(const hylk_TransactionInput* input)
{
    return hylk_TransactionOutPoint::ref(&hylk_TransactionInput::get(input).prevout);
}

void hylk_transaction_input_destroy(hylk_TransactionInput* input)
{
    delete input;
}

hylk_TransactionOutPoint* hylk_transaction_out_point_copy(const hylk_TransactionOutPoint* out_point)
{
    return hylk_TransactionOutPoint::copy(out_point);
}

uint32_t hylk_transaction_out_point_get_index(const hylk_TransactionOutPoint* out_point)
{
    return hylk_TransactionOutPoint::get(out_point).n;
}

const hylk_Txid* hylk_transaction_out_point_get_txid(const hylk_TransactionOutPoint* out_point)
{
    return hylk_Txid::ref(&hylk_TransactionOutPoint::get(out_point).hash);
}

void hylk_transaction_out_point_destroy(hylk_TransactionOutPoint* out_point)
{
    delete out_point;
}

hylk_Txid* hylk_txid_copy(const hylk_Txid* txid)
{
    return hylk_Txid::copy(txid);
}

void hylk_txid_to_bytes(const hylk_Txid* txid, unsigned char output[32])
{
    std::memcpy(output, hylk_Txid::get(txid).begin(), 32);
}

int hylk_txid_equals(const hylk_Txid* txid1, const hylk_Txid* txid2)
{
    return hylk_Txid::get(txid1) == hylk_Txid::get(txid2);
}

void hylk_txid_destroy(hylk_Txid* txid)
{
    delete txid;
}

void hylk_logging_set_options(const hylk_LoggingOptions options)
{
    LOCK(cs_main);
    LogInstance().m_log_timestamps = options.log_timestamps;
    LogInstance().m_log_time_micros = options.log_time_micros;
    LogInstance().m_log_threadnames = options.log_threadnames;
    LogInstance().m_log_sourcelocations = options.log_sourcelocations;
    LogInstance().m_always_print_category_level = options.always_print_category_levels;
}

void hylk_logging_set_level_category(hylk_LogCategory category, hylk_LogLevel level)
{
    LOCK(cs_main);
    if (category == hylk_LogCategory_ALL) {
        LogInstance().SetLogLevel(get_bclog_level(level));
    }

    LogInstance().AddCategoryLogLevel(get_bclog_flag(category), get_bclog_level(level));
}

void hylk_logging_enable_category(hylk_LogCategory category)
{
    LogInstance().EnableCategory(get_bclog_flag(category));
}

void hylk_logging_disable_category(hylk_LogCategory category)
{
    LogInstance().DisableCategory(get_bclog_flag(category));
}

void hylk_logging_disable()
{
    LogInstance().DisableLogging();
}

hylk_LoggingConnection* hylk_logging_connection_create(hylk_LogCallback callback, void* user_data, hylk_DestroyCallback user_data_destroy_callback)
{
    try {
        return hylk_LoggingConnection::create(callback, user_data, user_data_destroy_callback);
    } catch (const std::exception&) {
        return nullptr;
    }
}

void hylk_logging_connection_destroy(hylk_LoggingConnection* connection)
{
    delete connection;
}

hylk_ChainParameters* hylk_chain_parameters_create(const hylk_ChainType chain_type)
{
    switch (chain_type) {
    case hylk_ChainType_MAINNET: {
        return hylk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::Main().release()));
    }
    case hylk_ChainType_TESTNET: {
        return hylk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::TestNet().release()));
    }
    case hylk_ChainType_TESTNET_4: {
        return hylk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::TestNet4().release()));
    }
    case hylk_ChainType_SIGNET: {
        return hylk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::SigNet({}).release()));
    }
    case hylk_ChainType_REGTEST: {
        return hylk_ChainParameters::ref(const_cast<CChainParams*>(CChainParams::RegTest({}).release()));
    }
    }
    assert(false);
}

hylk_ChainParameters* hylk_chain_parameters_copy(const hylk_ChainParameters* chain_parameters)
{
    return hylk_ChainParameters::copy(chain_parameters);
}

void hylk_chain_parameters_destroy(hylk_ChainParameters* chain_parameters)
{
    delete chain_parameters;
}

hylk_ContextOptions* hylk_context_options_create()
{
    return hylk_ContextOptions::create();
}

void hylk_context_options_set_chainparams(hylk_ContextOptions* options, const hylk_ChainParameters* chain_parameters)
{
    // Copy the chainparams, so the caller can free it again
    LOCK(hylk_ContextOptions::get(options).m_mutex);
    hylk_ContextOptions::get(options).m_chainparams = std::make_unique<const CChainParams>(hylk_ChainParameters::get(chain_parameters));
}

void hylk_context_options_set_notifications(hylk_ContextOptions* options, hylk_NotificationInterfaceCallbacks notifications)
{
    // The KernelNotifications are copy-initialized, so the caller can free them again.
    LOCK(hylk_ContextOptions::get(options).m_mutex);
    hylk_ContextOptions::get(options).m_notifications = std::make_shared<KernelNotifications>(notifications);
}

void hylk_context_options_set_validation_interface(hylk_ContextOptions* options, hylk_ValidationInterfaceCallbacks vi_cbs)
{
    LOCK(hylk_ContextOptions::get(options).m_mutex);
    hylk_ContextOptions::get(options).m_validation_interface = std::make_shared<KernelValidationInterface>(vi_cbs);
}

void hylk_context_options_destroy(hylk_ContextOptions* options)
{
    delete options;
}

hylk_Context* hylk_context_create(const hylk_ContextOptions* options)
{
    bool sane{true};
    const ContextOptions* opts = options ? &hylk_ContextOptions::get(options) : nullptr;
    auto context{std::make_shared<const Context>(opts, sane)};
    if (!sane) {
        LogError("Kernel context sanity check failed.");
        return nullptr;
    }
    return hylk_Context::create(context);
}

hylk_Context* hylk_context_copy(const hylk_Context* context)
{
    return hylk_Context::copy(context);
}

int hylk_context_interrupt(hylk_Context* context)
{
    return (*hylk_Context::get(context)->m_interrupt)() ? 0 : -1;
}

void hylk_context_destroy(hylk_Context* context)
{
    delete context;
}

const hylk_BlockTreeEntry* hylk_block_tree_entry_get_previous(const hylk_BlockTreeEntry* entry)
{
    if (!hylk_BlockTreeEntry::get(entry).pprev) {
        LogInfo("Genesis block has no previous.");
        return nullptr;
    }

    return hylk_BlockTreeEntry::ref(hylk_BlockTreeEntry::get(entry).pprev);
}

hylk_ValidationMode hylk_block_validation_state_get_validation_mode(const hylk_BlockValidationState* block_validation_state_)
{
    auto& block_validation_state = hylk_BlockValidationState::get(block_validation_state_);
    if (block_validation_state.IsValid()) return hylk_ValidationMode_VALID;
    if (block_validation_state.IsInvalid()) return hylk_ValidationMode_INVALID;
    return hylk_ValidationMode_INTERNAL_ERROR;
}

hylk_BlockValidationResult hylk_block_validation_state_get_block_validation_result(const hylk_BlockValidationState* block_validation_state_)
{
    auto& block_validation_state = hylk_BlockValidationState::get(block_validation_state_);
    switch (block_validation_state.GetResult()) {
    case BlockValidationResult::BLOCK_RESULT_UNSET:
        return hylk_BlockValidationResult_UNSET;
    case BlockValidationResult::BLOCK_CONSENSUS:
        return hylk_BlockValidationResult_CONSENSUS;
    case BlockValidationResult::BLOCK_CACHED_INVALID:
        return hylk_BlockValidationResult_CACHED_INVALID;
    case BlockValidationResult::BLOCK_INVALID_HEADER:
        return hylk_BlockValidationResult_INVALID_HEADER;
    case BlockValidationResult::BLOCK_MUTATED:
        return hylk_BlockValidationResult_MUTATED;
    case BlockValidationResult::BLOCK_MISSING_PREV:
        return hylk_BlockValidationResult_MISSING_PREV;
    case BlockValidationResult::BLOCK_INVALID_PREV:
        return hylk_BlockValidationResult_INVALID_PREV;
    case BlockValidationResult::BLOCK_TIME_FUTURE:
        return hylk_BlockValidationResult_TIME_FUTURE;
    case BlockValidationResult::BLOCK_HEADER_LOW_WORK:
        return hylk_BlockValidationResult_HEADER_LOW_WORK;
    } // no default case, so the compiler can warn about missing cases
    assert(false);
}

hylk_ChainstateManagerOptions* hylk_chainstate_manager_options_create(const hylk_Context* context, const char* data_dir, size_t data_dir_len, const char* blocks_dir, size_t blocks_dir_len)
{
    if (data_dir == nullptr || data_dir_len == 0 || blocks_dir == nullptr || blocks_dir_len == 0) {
        LogError("Failed to create chainstate manager options: dir must be non-null and non-empty");
        return nullptr;
    }
    try {
        fs::path abs_data_dir{fs::absolute(fs::PathFromString({data_dir, data_dir_len}))};
        fs::create_directories(abs_data_dir);
        fs::path abs_blocks_dir{fs::absolute(fs::PathFromString({blocks_dir, blocks_dir_len}))};
        fs::create_directories(abs_blocks_dir);
        return hylk_ChainstateManagerOptions::create(hylk_Context::get(context), abs_data_dir, abs_blocks_dir);
    } catch (const std::exception& e) {
        LogError("Failed to create chainstate manager options: %s", e.what());
        return nullptr;
    }
}

void hylk_chainstate_manager_options_set_worker_threads_num(hylk_ChainstateManagerOptions* opts, int worker_threads)
{
    LOCK(hylk_ChainstateManagerOptions::get(opts).m_mutex);
    hylk_ChainstateManagerOptions::get(opts).m_chainman_options.worker_threads_num = worker_threads;
}

void hylk_chainstate_manager_options_destroy(hylk_ChainstateManagerOptions* options)
{
    delete options;
}

int hylk_chainstate_manager_options_set_wipe_dbs(hylk_ChainstateManagerOptions* chainman_opts, int wipe_block_tree_db, int wipe_chainstate_db)
{
    if (wipe_block_tree_db == 1 && wipe_chainstate_db != 1) {
        LogError("Wiping the block tree db without also wiping the chainstate db is currently unsupported.");
        return -1;
    }
    auto& opts{hylk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_blockman_options.block_tree_db_params.wipe_data = wipe_block_tree_db == 1;
    opts.m_chainstate_load_options.wipe_chainstate_db = wipe_chainstate_db == 1;
    return 0;
}

void hylk_chainstate_manager_options_update_block_tree_db_in_memory(
    hylk_ChainstateManagerOptions* chainman_opts,
    int block_tree_db_in_memory)
{
    auto& opts{hylk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_blockman_options.block_tree_db_params.memory_only = block_tree_db_in_memory == 1;
}

void hylk_chainstate_manager_options_update_chainstate_db_in_memory(
    hylk_ChainstateManagerOptions* chainman_opts,
    int chainstate_db_in_memory)
{
    auto& opts{hylk_ChainstateManagerOptions::get(chainman_opts)};
    LOCK(opts.m_mutex);
    opts.m_chainstate_load_options.coins_db_in_memory = chainstate_db_in_memory == 1;
}

hylk_ChainstateManager* hylk_chainstate_manager_create(
    const hylk_ChainstateManagerOptions* chainman_opts)
{
    auto& opts{hylk_ChainstateManagerOptions::get(chainman_opts)};
    std::unique_ptr<ChainstateManager> chainman;
    try {
        LOCK(opts.m_mutex);
        chainman = std::make_unique<ChainstateManager>(*opts.m_context->m_interrupt, opts.m_chainman_options, opts.m_blockman_options);
    } catch (const std::exception& e) {
        LogError("Failed to create chainstate manager: %s", e.what());
        return nullptr;
    }

    try {
        const auto chainstate_load_opts{WITH_LOCK(opts.m_mutex, return opts.m_chainstate_load_options)};

        kernel::CacheSizes cache_sizes{DEFAULT_KERNEL_CACHE};
        auto [status, chainstate_err]{node::LoadChainstate(*chainman, cache_sizes, chainstate_load_opts)};
        if (status != node::ChainstateLoadStatus::SUCCESS) {
            LogError("Failed to load chain state from your data directory: %s", chainstate_err.original);
            return nullptr;
        }
        std::tie(status, chainstate_err) = node::VerifyLoadedChainstate(*chainman, chainstate_load_opts);
        if (status != node::ChainstateLoadStatus::SUCCESS) {
            LogError("Failed to verify loaded chain state from your datadir: %s", chainstate_err.original);
            return nullptr;
        }

        for (Chainstate* chainstate : WITH_LOCK(chainman->GetMutex(), return chainman->GetAll())) {
            BlockValidationState state;
            if (!chainstate->ActivateBestChain(state, nullptr)) {
                LogError("Failed to connect best block: %s", state.ToString());
                return nullptr;
            }
        }
    } catch (const std::exception& e) {
        LogError("Failed to load chainstate: %s", e.what());
        return nullptr;
    }

    return hylk_ChainstateManager::create(std::move(chainman), opts.m_context);
}

const hylk_BlockTreeEntry* hylk_chainstate_manager_get_block_tree_entry_by_hash(const hylk_ChainstateManager* chainman, const hylk_BlockHash* block_hash)
{
    auto block_index = WITH_LOCK(hylk_ChainstateManager::get(chainman).m_chainman->GetMutex(),
                                 return hylk_ChainstateManager::get(chainman).m_chainman->m_blockman.LookupBlockIndex(hylk_BlockHash::get(block_hash)));
    if (!block_index) {
        LogDebug(BCLog::KERNEL, "A block with the given hash is not indexed.");
        return nullptr;
    }
    return hylk_BlockTreeEntry::ref(block_index);
}

void hylk_chainstate_manager_destroy(hylk_ChainstateManager* chainman)
{
    {
        LOCK(hylk_ChainstateManager::get(chainman).m_chainman->GetMutex());
        for (Chainstate* chainstate : hylk_ChainstateManager::get(chainman).m_chainman->GetAll()) {
            if (chainstate->CanFlushToDisk()) {
                chainstate->ForceFlushStateToDisk();
                chainstate->ResetCoinsViews();
            }
        }
    }

    delete chainman;
}

int hylk_chainstate_manager_import_blocks(hylk_ChainstateManager* chainman, const char** block_file_paths_data, size_t* block_file_paths_lens, size_t block_file_paths_data_len)
{
    try {
        std::vector<fs::path> import_files;
        import_files.reserve(block_file_paths_data_len);
        for (uint32_t i = 0; i < block_file_paths_data_len; i++) {
            if (block_file_paths_data[i] != nullptr) {
                import_files.emplace_back(std::string{block_file_paths_data[i], block_file_paths_lens[i]}.c_str());
            }
        }
        node::ImportBlocks(*hylk_ChainstateManager::get(chainman).m_chainman, import_files);
    } catch (const std::exception& e) {
        LogError("Failed to import blocks: %s", e.what());
        return -1;
    }
    return 0;
}

hylk_Block* hylk_block_create(const void* raw_block, size_t raw_block_length)
{
    if (raw_block == nullptr && raw_block_length != 0) {
        return nullptr;
    }
    auto block{std::make_shared<CBlock>()};

    DataStream stream{std::span{reinterpret_cast<const std::byte*>(raw_block), raw_block_length}};

    try {
        stream >> TX_WITH_WITNESS(*block);
    } catch (...) {
        LogDebug(BCLog::KERNEL, "Block decode failed.");
        return nullptr;
    }

    return hylk_Block::create(block);
}

hylk_Block* hylk_block_copy(const hylk_Block* block)
{
    return hylk_Block::copy(block);
}

size_t hylk_block_count_transactions(const hylk_Block* block)
{
    return hylk_Block::get(block)->vtx.size();
}

const hylk_Transaction* hylk_block_get_transaction_at(const hylk_Block* block, size_t index)
{
    assert(index < hylk_Block::get(block)->vtx.size());
    return hylk_Transaction::ref(&hylk_Block::get(block)->vtx[index]);
}

int hylk_block_to_bytes(const hylk_Block* block, hylk_WriteBytes writer, void* user_data)
{
    try {
        WriterStream ws{writer, user_data};
        ws << TX_WITH_WITNESS(*hylk_Block::get(block));
        return 0;
    } catch (...) {
        return -1;
    }
}

hylk_BlockHash* hylk_block_get_hash(const hylk_Block* block)
{
    return hylk_BlockHash::create(hylk_Block::get(block)->GetHash());
}

void hylk_block_destroy(hylk_Block* block)
{
    delete block;
}

hylk_Block* hylk_block_read(const hylk_ChainstateManager* chainman, const hylk_BlockTreeEntry* entry)
{
    auto block{std::make_shared<CBlock>()};
    if (!hylk_ChainstateManager::get(chainman).m_chainman->m_blockman.ReadBlock(*block, hylk_BlockTreeEntry::get(entry))) {
        LogError("Failed to read block.");
        return nullptr;
    }
    return hylk_Block::create(block);
}

int32_t hylk_block_tree_entry_get_height(const hylk_BlockTreeEntry* entry)
{
    return hylk_BlockTreeEntry::get(entry).nHeight;
}

const hylk_BlockHash* hylk_block_tree_entry_get_block_hash(const hylk_BlockTreeEntry* entry)
{
    return hylk_BlockHash::ref(hylk_BlockTreeEntry::get(entry).phashBlock);
}

int hylk_block_tree_entry_equals(const hylk_BlockTreeEntry* entry1, const hylk_BlockTreeEntry* entry2)
{
    return &hylk_BlockTreeEntry::get(entry1) == &hylk_BlockTreeEntry::get(entry2);
}

hylk_BlockHash* hylk_block_hash_create(const unsigned char block_hash[32])
{
    return hylk_BlockHash::create(std::span<const unsigned char>{block_hash, 32});
}

hylk_BlockHash* hylk_block_hash_copy(const hylk_BlockHash* block_hash)
{
    return hylk_BlockHash::copy(block_hash);
}

void hylk_block_hash_to_bytes(const hylk_BlockHash* block_hash, unsigned char output[32])
{
    std::memcpy(output, hylk_BlockHash::get(block_hash).begin(), 32);
}

int hylk_block_hash_equals(const hylk_BlockHash* hash1, const hylk_BlockHash* hash2)
{
    return hylk_BlockHash::get(hash1) == hylk_BlockHash::get(hash2);
}

void hylk_block_hash_destroy(hylk_BlockHash* hash)
{
    delete hash;
}

hylk_BlockSpentOutputs* hylk_block_spent_outputs_read(const hylk_ChainstateManager* chainman, const hylk_BlockTreeEntry* entry)
{
    auto block_undo{std::make_shared<CBlockUndo>()};
    if (hylk_BlockTreeEntry::get(entry).nHeight < 1) {
        LogDebug(BCLog::KERNEL, "The genesis block does not have any spent outputs.");
        return hylk_BlockSpentOutputs::create(block_undo);
    }
    if (!hylk_ChainstateManager::get(chainman).m_chainman->m_blockman.ReadBlockUndo(*block_undo, hylk_BlockTreeEntry::get(entry))) {
        LogError("Failed to read block spent outputs data.");
        return nullptr;
    }
    return hylk_BlockSpentOutputs::create(block_undo);
}

hylk_BlockSpentOutputs* hylk_block_spent_outputs_copy(const hylk_BlockSpentOutputs* block_spent_outputs)
{
    return hylk_BlockSpentOutputs::copy(block_spent_outputs);
}

size_t hylk_block_spent_outputs_count(const hylk_BlockSpentOutputs* block_spent_outputs)
{
    return hylk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.size();
}

const hylk_TransactionSpentOutputs* hylk_block_spent_outputs_get_transaction_spent_outputs_at(const hylk_BlockSpentOutputs* block_spent_outputs, size_t transaction_index)
{
    assert(transaction_index < hylk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.size());
    const auto* tx_undo{&hylk_BlockSpentOutputs::get(block_spent_outputs)->vtxundo.at(transaction_index)};
    return hylk_TransactionSpentOutputs::ref(tx_undo);
}

void hylk_block_spent_outputs_destroy(hylk_BlockSpentOutputs* block_spent_outputs)
{
    delete block_spent_outputs;
}

hylk_TransactionSpentOutputs* hylk_transaction_spent_outputs_copy(const hylk_TransactionSpentOutputs* transaction_spent_outputs)
{
    return hylk_TransactionSpentOutputs::copy(transaction_spent_outputs);
}

size_t hylk_transaction_spent_outputs_count(const hylk_TransactionSpentOutputs* transaction_spent_outputs)
{
    return hylk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.size();
}

void hylk_transaction_spent_outputs_destroy(hylk_TransactionSpentOutputs* transaction_spent_outputs)
{
    delete transaction_spent_outputs;
}

const hylk_Coin* hylk_transaction_spent_outputs_get_coin_at(const hylk_TransactionSpentOutputs* transaction_spent_outputs, size_t coin_index)
{
    assert(coin_index < hylk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.size());
    const Coin* coin{&hylk_TransactionSpentOutputs::get(transaction_spent_outputs).vprevout.at(coin_index)};
    return hylk_Coin::ref(coin);
}

hylk_Coin* hylk_coin_copy(const hylk_Coin* coin)
{
    return hylk_Coin::copy(coin);
}

uint32_t hylk_coin_confirmation_height(const hylk_Coin* coin)
{
    return hylk_Coin::get(coin).nHeight;
}

int hylk_coin_is_coinbase(const hylk_Coin* coin)
{
    return hylk_Coin::get(coin).IsCoinBase() ? 1 : 0;
}

const hylk_TransactionOutput* hylk_coin_get_output(const hylk_Coin* coin)
{
    return hylk_TransactionOutput::ref(&hylk_Coin::get(coin).out);
}

void hylk_coin_destroy(hylk_Coin* coin)
{
    delete coin;
}

int hylk_chainstate_manager_process_block(
    hylk_ChainstateManager* chainman,
    const hylk_Block* block,
    int* _new_block)
{
    bool new_block;
    auto result = hylk_ChainstateManager::get(chainman).m_chainman->ProcessNewBlock(hylk_Block::get(block), /*force_processing=*/true, /*min_pow_checked=*/true, /*new_block=*/&new_block);
    if (_new_block) {
        *_new_block = new_block ? 1 : 0;
    }
    return result ? 0 : -1;
}

const hylk_Chain* hylk_chainstate_manager_get_active_chain(const hylk_ChainstateManager* chainman)
{
    return hylk_Chain::ref(&WITH_LOCK(hylk_ChainstateManager::get(chainman).m_chainman->GetMutex(), return hylk_ChainstateManager::get(chainman).m_chainman->ActiveChain()));
}

int hylk_chain_get_height(const hylk_Chain* chain)
{
    LOCK(::cs_main);
    return hylk_Chain::get(chain).Height();
}

const hylk_BlockTreeEntry* hylk_chain_get_by_height(const hylk_Chain* chain, int height)
{
    LOCK(::cs_main);
    return hylk_BlockTreeEntry::ref(hylk_Chain::get(chain)[height]);
}

int hylk_chain_contains(const hylk_Chain* chain, const hylk_BlockTreeEntry* entry)
{
    LOCK(::cs_main);
    return hylk_Chain::get(chain).Contains(&hylk_BlockTreeEntry::get(entry)) ? 1 : 0;
}
