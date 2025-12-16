// Copyright (c) 2024-present The Hylium Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HYLIUM_KERNEL_HYLIUMKERNEL_H
#define HYLIUM_KERNEL_HYLIUMKERNEL_H

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif // __cplusplus

#ifndef HYLIUMKERNEL_API
    #ifdef HYLIUMKERNEL_BUILD
        #if defined(_WIN32)
            #define HYLIUMKERNEL_API __declspec(dllexport)
        #else
            #define HYLIUMKERNEL_API __attribute__((visibility("default")))
        #endif
    #else
        #if defined(_WIN32) && !defined(HYLIUMKERNEL_STATIC)
            #define HYLIUMKERNEL_API __declspec(dllimport)
        #else
            #define HYLIUMKERNEL_API
        #endif
    #endif
#endif

/* Warning attributes */
#if defined(__GNUC__)
    #define HYLIUMKERNEL_WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#else
    #define HYLIUMKERNEL_WARN_UNUSED_RESULT
#endif

/**
 * HYLIUMKERNEL_ARG_NONNULL is a compiler attribute used to indicate that
 * certain pointer arguments to a function are not expected to be null.
 *
 * Callers must not pass a null pointer for arguments marked with this attribute,
 * as doing so may result in undefined behavior. This attribute should only be
 * used for arguments where a null pointer is unambiguously a programmer error,
 * such as for opaque handles, and not for pointers to raw input data that might
 * validly be null (e.g., from an empty std::span or std::string).
 */
#if !defined(HYLIUMKERNEL_BUILD) && defined(__GNUC__)
    #define HYLIUMKERNEL_ARG_NONNULL(...) __attribute__((__nonnull__(__VA_ARGS__)))
#else
    #define HYLIUMKERNEL_ARG_NONNULL(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @page remarks Remarks
 *
 * @section purpose Purpose
 *
 * This header currently exposes an API for interacting with parts of Hylium
 * Core's consensus code. Users can validate blocks, iterate the block index,
 * read block and undo data from disk, and validate scripts. The header is
 * unversioned and not stable yet. Users should expect breaking changes. It is
 * also not yet included in releases of Hylium Core.
 *
 * @section context Context
 *
 * The library provides a built-in static constant kernel context. This static
 * context offers only limited functionality. It detects and self-checks the
 * correct sha256 implementation, initializes the random number generator and
 * self-checks the secp256k1 static context. It is used internally for
 * otherwise "context-free" operations. This means that the user is not
 * required to initialize their own context before using the library.
 *
 * The user should create their own context for passing it to state-rich validation
 * functions and holding callbacks for kernel events.
 *
 * @section error Error handling
 *
 * Functions communicate an error through their return types, usually returning
 * a nullptr, 0, or false if an error is encountered. Additionally, verification
 * functions, e.g. for scripts, may communicate more detailed error information
 * through status code out parameters.
 *
 * Fine-grained validation information is communicated through the validation
 * interface.
 *
 * The kernel notifications issue callbacks for errors. These are usually
 * indicative of a system error. If such an error is issued, it is recommended
 * to halt and tear down the existing kernel objects. Remediating the error may
 * require system intervention by the user.
 *
 * @section pointer Pointer and argument conventions
 *
 * The user is responsible for de-allocating the memory owned by pointers
 * returned by functions. Typically pointers returned by *_create(...) functions
 * can be de-allocated by corresponding *_destroy(...) functions.
 *
 * A function that takes pointer arguments makes no assumptions on their
 * lifetime. Once the function returns the user can safely de-allocate the
 * passed in arguments.
 *
 * Const pointers represent views, and do not transfer ownership. Lifetime
 * guarantees of these objects are described in the respective documentation.
 * Ownership of these resources may be taken by copying. They are typically
 * used for iteration with minimal overhead and require some care by the
 * programmer that their lifetime is not extended beyond that of the original
 * object.
 *
 * Array lengths follow the pointer argument they describe.
 */

/**
 * Opaque data structure for holding a transaction.
 */
typedef struct hylk_Transaction hylk_Transaction;

/**
 * Opaque data structure for holding a script pubkey.
 */
typedef struct hylk_ScriptPubkey hylk_ScriptPubkey;

/**
 * Opaque data structure for holding a transaction output.
 */
typedef struct hylk_TransactionOutput hylk_TransactionOutput;

/**
 * Opaque data structure for holding a logging connection.
 *
 * The logging connection can be used to manually stop logging.
 *
 * Messages that were logged before a connection is created are buffered in a
 * 1MB buffer. Logging can alternatively be permanently disabled by calling
 * @ref hylk_logging_disable. Functions changing the logging settings are
 * global and change the settings for all existing hylk_LoggingConnection
 * instances.
 */
typedef struct hylk_LoggingConnection hylk_LoggingConnection;

/**
 * Opaque data structure for holding the chain parameters.
 *
 * These are eventually placed into a kernel context through the kernel context
 * options. The parameters describe the properties of a chain, and may be
 * instantiated for either mainnet, testnet, signet, or regtest.
 */
typedef struct hylk_ChainParameters hylk_ChainParameters;

/**
 * Opaque data structure for holding options for creating a new kernel context.
 *
 * Once a kernel context has been created from these options, they may be
 * destroyed. The options hold the notification and validation interface
 * callbacks as well as the selected chain type until they are passed to the
 * context. If no options are configured, the context will be instantiated with
 * no callbacks and for mainnet. Their content and scope can be expanded over
 * time.
 */
typedef struct hylk_ContextOptions hylk_ContextOptions;

/**
 * Opaque data structure for holding a kernel context.
 *
 * The kernel context is used to initialize internal state and hold the chain
 * parameters and callbacks for handling error and validation events. Once
 * other validation objects are instantiated from it, the context is kept in
 * memory for the duration of their lifetimes.
 *
 * The processing of validation events is done through an internal task runner
 * owned by the context. It passes events through the registered validation
 * interface callbacks.
 *
 * A constructed context can be safely used from multiple threads.
 */
typedef struct hylk_Context hylk_Context;

/**
 * Opaque data structure for holding a block tree entry.
 *
 * This is a pointer to an element in the block index currently in memory of
 * the chainstate manager. It is valid for the lifetime of the chainstate
 * manager it was retrieved from. The entry is part of a tree-like structure
 * that is maintained internally. Every entry, besides the genesis, points to a
 * single parent. Multiple entries may share a parent, thus forming a tree.
 * Each entry corresponds to a single block and may be used to retrieve its
 * data and validation status.
 */
typedef struct hylk_BlockTreeEntry hylk_BlockTreeEntry;

/**
 * Opaque data structure for holding options for creating a new chainstate
 * manager.
 *
 * The chainstate manager options are used to set some parameters for the
 * chainstate manager.
 */
typedef struct hylk_ChainstateManagerOptions hylk_ChainstateManagerOptions;

/**
 * Opaque data structure for holding a chainstate manager.
 *
 * The chainstate manager is the central object for doing validation tasks as
 * well as retrieving data from the chain. Internally it is a complex data
 * structure with diverse functionality.
 *
 * Its functionality will be more and more exposed in the future.
 */
typedef struct hylk_ChainstateManager hylk_ChainstateManager;

/**
 * Opaque data structure for holding a block.
 */
typedef struct hylk_Block hylk_Block;

/**
 * Opaque data structure for holding the state of a block during validation.
 *
 * Contains information indicating whether validation was successful, and if not
 * which step during block validation failed.
 */
typedef struct hylk_BlockValidationState hylk_BlockValidationState;

/**
 * Opaque data structure for holding the currently known best-chain associated
 * with a chainstate.
 */
typedef struct hylk_Chain hylk_Chain;

/**
 * Opaque data structure for holding a block's spent outputs.
 *
 * Contains all the previous outputs consumed by all transactions in a specific
 * block. Internally it holds a nested vector. The top level vector has an
 * entry for each transaction in a block (in order of the actual transactions
 * of the block and without the coinbase transaction). This is exposed through
 * @ref hylk_TransactionSpentOutputs. Each hylk_TransactionSpentOutputs is in
 * turn a vector of all the previous outputs of a transaction (in order of
 * their corresponding inputs).
 */
typedef struct hylk_BlockSpentOutputs hylk_BlockSpentOutputs;

/**
 * Opaque data structure for holding a transaction's spent outputs.
 *
 * Holds the coins consumed by a certain transaction. Retrieved through the
 * @ref hylk_BlockSpentOutputs. The coins are in the same order as the
 * transaction's inputs consuming them.
 */
typedef struct hylk_TransactionSpentOutputs hylk_TransactionSpentOutputs;

/**
 * Opaque data structure for holding a coin.
 *
 * Holds information on the @ref hylk_TransactionOutput held within,
 * including the height it was spent at and whether it is a coinbase output.
 */
typedef struct hylk_Coin hylk_Coin;

/**
 * Opaque data structure for holding a block hash.
 *
 * This is a type-safe identifier for a block.
 */
typedef struct hylk_BlockHash hylk_BlockHash;

/**
 * Opaque data structure for holding a transaction input.
 *
 * Holds information on the @ref hylk_TransactionOutPoint held within.
 */
typedef struct hylk_TransactionInput hylk_TransactionInput;

/**
 * Opaque data structure for holding a transaction out point.
 *
 * Holds the txid and output index it is pointing to.
 */
typedef struct hylk_TransactionOutPoint hylk_TransactionOutPoint;

typedef struct hylk_Txid hylk_Txid;

/** Current sync state passed to tip changed callbacks. */
typedef uint8_t hylk_SynchronizationState;
#define hylk_SynchronizationState_INIT_REINDEX ((hylk_SynchronizationState)(0))
#define hylk_SynchronizationState_INIT_DOWNLOAD ((hylk_SynchronizationState)(1))
#define hylk_SynchronizationState_POST_INIT ((hylk_SynchronizationState)(2))

/** Possible warning types issued by validation. */
typedef uint8_t hylk_Warning;
#define hylk_Warning_UNKNOWN_NEW_RULES_ACTIVATED ((hylk_Warning)(0))
#define hylk_Warning_LARGE_WORK_INVALID_CHAIN ((hylk_Warning)(1))

/** Callback function types */

/**
 * Function signature for the global logging callback. All hylium kernel
 * internal logs will pass through this callback.
 */
typedef void (*hylk_LogCallback)(void* user_data, const char* message, size_t message_len);

/**
 * Function signature for freeing user data.
 */
typedef void (*hylk_DestroyCallback)(void* user_data);

/**
 * Function signatures for the kernel notifications.
 */
typedef void (*hylk_NotifyBlockTip)(void* user_data, hylk_SynchronizationState state, const hylk_BlockTreeEntry* entry, double verification_progress);
typedef void (*hylk_NotifyHeaderTip)(void* user_data, hylk_SynchronizationState state, int64_t height, int64_t timestamp, int presync);
typedef void (*hylk_NotifyProgress)(void* user_data, const char* title, size_t title_len, int progress_percent, int resume_possible);
typedef void (*hylk_NotifyWarningSet)(void* user_data, hylk_Warning warning, const char* message, size_t message_len);
typedef void (*hylk_NotifyWarningUnset)(void* user_data, hylk_Warning warning);
typedef void (*hylk_NotifyFlushError)(void* user_data, const char* message, size_t message_len);
typedef void (*hylk_NotifyFatalError)(void* user_data, const char* message, size_t message_len);

/**
 * Function signatures for the validation interface.
 */
typedef void (*hylk_ValidationInterfaceBlockChecked)(void* user_data, hylk_Block* block, const hylk_BlockValidationState* state);
typedef void (*hylk_ValidationInterfacePoWValidBlock)(void* user_data, hylk_Block* block, const hylk_BlockTreeEntry* entry);
typedef void (*hylk_ValidationInterfaceBlockConnected)(void* user_data, hylk_Block* block, const hylk_BlockTreeEntry* entry);
typedef void (*hylk_ValidationInterfaceBlockDisconnected)(void* user_data, hylk_Block* block, const hylk_BlockTreeEntry* entry);

/**
 * Function signature for serializing data.
 */
typedef int (*hylk_WriteBytes)(const void* bytes, size_t size, void* userdata);

/**
 * Whether a validated data structure is valid, invalid, or an error was
 * encountered during processing.
 */
typedef uint8_t hylk_ValidationMode;
#define hylk_ValidationMode_VALID ((hylk_ValidationMode)(0))
#define hylk_ValidationMode_INVALID ((hylk_ValidationMode)(1))
#define hylk_ValidationMode_INTERNAL_ERROR ((hylk_ValidationMode)(2))

/**
 * A granular "reason" why a block was invalid.
 */
typedef uint32_t hylk_BlockValidationResult;
#define hylk_BlockValidationResult_UNSET ((hylk_BlockValidationResult)(0))           //!< initial value. Block has not yet been rejected
#define hylk_BlockValidationResult_CONSENSUS ((hylk_BlockValidationResult)(1))       //!< invalid by consensus rules (excluding any below reasons)
#define hylk_BlockValidationResult_CACHED_INVALID ((hylk_BlockValidationResult)(2))  //!< this block was cached as being invalid and we didn't store the reason why
#define hylk_BlockValidationResult_INVALID_HEADER ((hylk_BlockValidationResult)(3))  //!< invalid proof of work or time too old
#define hylk_BlockValidationResult_MUTATED ((hylk_BlockValidationResult)(4))         //!< the block's data didn't match the data committed to by the PoW
#define hylk_BlockValidationResult_MISSING_PREV ((hylk_BlockValidationResult)(5))    //!< We don't have the previous block the checked one is built on
#define hylk_BlockValidationResult_INVALID_PREV ((hylk_BlockValidationResult)(6))    //!< A block this one builds on is invalid
#define hylk_BlockValidationResult_TIME_FUTURE ((hylk_BlockValidationResult)(7))     //!< block timestamp was > 2 hours in the future (or our clock is bad)
#define hylk_BlockValidationResult_HEADER_LOW_WORK ((hylk_BlockValidationResult)(8)) //!< the block header may be on a too-little-work chain

/**
 * Holds the validation interface callbacks. The user data pointer may be used
 * to point to user-defined structures to make processing the validation
 * callbacks easier. Note that these callbacks block any further validation
 * execution when they are called.
 */
typedef struct {
    void* user_data;                                              //!< Holds a user-defined opaque structure that is passed to the validation
                                                                  //!< interface callbacks. If user_data_destroy is also defined ownership of the
                                                                  //!< user_data is passed to the created context options and subsequently context.
    hylk_DestroyCallback user_data_destroy;                       //!< Frees the provided user data structure.
    hylk_ValidationInterfaceBlockChecked block_checked;           //!< Called when a new block has been fully validated. Contains the
                                                                  //!< result of its validation.
    hylk_ValidationInterfacePoWValidBlock pow_valid_block;        //!< Called when a new block extends the header chain and has a valid transaction
                                                                  //!< and segwit merkle root.
    hylk_ValidationInterfaceBlockConnected block_connected;       //!< Called when a block is valid and has now been connected to the best chain.
    hylk_ValidationInterfaceBlockDisconnected block_disconnected; //!< Called during a re-org when a block has been removed from the best chain.
} hylk_ValidationInterfaceCallbacks;

/**
 * A struct for holding the kernel notification callbacks. The user data
 * pointer may be used to point to user-defined structures to make processing
 * the notifications easier.
 *
 * If user_data_destroy is provided, the kernel will automatically call this
 * callback to clean up user_data when the notification interface object is destroyed.
 * If user_data_destroy is NULL, it is the user's responsibility to ensure that
 * the user_data outlives the kernel objects. Notifications can
 * occur even as kernel objects are deleted, so care has to be taken to ensure
 * safe unwinding.
 */
typedef struct {
    void* user_data;                        //!< Holds a user-defined opaque structure that is passed to the notification callbacks.
                                            //!< If user_data_destroy is also defined ownership of the user_data is passed to the
                                            //!< created context options and subsequently context.
    hylk_DestroyCallback user_data_destroy; //!< Frees the provided user data structure.
    hylk_NotifyBlockTip block_tip;          //!< The chain's tip was updated to the provided block entry.
    hylk_NotifyHeaderTip header_tip;        //!< A new best block header was added.
    hylk_NotifyProgress progress;           //!< Reports on current block synchronization progress.
    hylk_NotifyWarningSet warning_set;      //!< A warning issued by the kernel library during validation.
    hylk_NotifyWarningUnset warning_unset;  //!< A previous condition leading to the issuance of a warning is no longer given.
    hylk_NotifyFlushError flush_error;      //!< An error encountered when flushing data to disk.
    hylk_NotifyFatalError fatal_error;      //!< An unrecoverable system error encountered by the library.
} hylk_NotificationInterfaceCallbacks;

/**
 * A collection of logging categories that may be encountered by kernel code.
 */
typedef uint8_t hylk_LogCategory;
#define hylk_LogCategory_ALL ((hylk_LogCategory)(0))
#define hylk_LogCategory_BENCH ((hylk_LogCategory)(1))
#define hylk_LogCategory_BLOCKSTORAGE ((hylk_LogCategory)(2))
#define hylk_LogCategory_COINDB ((hylk_LogCategory)(3))
#define hylk_LogCategory_LEVELDB ((hylk_LogCategory)(4))
#define hylk_LogCategory_MEMPOOL ((hylk_LogCategory)(5))
#define hylk_LogCategory_PRUNE ((hylk_LogCategory)(6))
#define hylk_LogCategory_RAND ((hylk_LogCategory)(7))
#define hylk_LogCategory_REINDEX ((hylk_LogCategory)(8))
#define hylk_LogCategory_VALIDATION ((hylk_LogCategory)(9))
#define hylk_LogCategory_KERNEL ((hylk_LogCategory)(10))

/**
 * The level at which logs should be produced.
 */
typedef uint8_t hylk_LogLevel;
#define hylk_LogLevel_TRACE ((hylk_LogLevel)(0))
#define hylk_LogLevel_DEBUG ((hylk_LogLevel)(1))
#define hylk_LogLevel_INFO ((hylk_LogLevel)(2))

/**
 * Options controlling the format of log messages.
 *
 * Set fields as non-zero to indicate true.
 */
typedef struct {
    int log_timestamps;               //!< Prepend a timestamp to log messages.
    int log_time_micros;              //!< Log timestamps in microsecond precision.
    int log_threadnames;              //!< Prepend the name of the thread to log messages.
    int log_sourcelocations;          //!< Prepend the source location to log messages.
    int always_print_category_levels; //!< Prepend the log category and level to log messages.
} hylk_LoggingOptions;

/**
 * A collection of status codes that may be issued by the script verify function.
 */
typedef uint8_t hylk_ScriptVerifyStatus;
#define hylk_ScriptVerifyStatus_OK ((hylk_ScriptVerifyStatus)(0))
#define hylk_ScriptVerifyStatus_ERROR_INVALID_FLAGS_COMBINATION ((hylk_ScriptVerifyStatus)(1)) //!< The flags were combined in an invalid way.
#define hylk_ScriptVerifyStatus_ERROR_SPENT_OUTPUTS_REQUIRED ((hylk_ScriptVerifyStatus)(2))    //!< The taproot flag was set, so valid spent_outputs have to be provided.

/**
 * Script verification flags that may be composed with each other.
 */
typedef uint32_t hylk_ScriptVerificationFlags;
#define hylk_ScriptVerificationFlags_NONE ((hylk_ScriptVerificationFlags)(0))
#define hylk_ScriptVerificationFlags_P2SH ((hylk_ScriptVerificationFlags)(1U << 0))                 //!< evaluate P2SH (BIP16) subscripts
#define hylk_ScriptVerificationFlags_DERSIG ((hylk_ScriptVerificationFlags)(1U << 2))               //!< enforce strict DER (BIP66) compliance
#define hylk_ScriptVerificationFlags_NULLDUMMY ((hylk_ScriptVerificationFlags)(1U << 4))            //!< enforce NULLDUMMY (BIP147)
#define hylk_ScriptVerificationFlags_CHECKLOCKTIMEVERIFY ((hylk_ScriptVerificationFlags)(1U << 9))  //!< enable CHECKLOCKTIMEVERIFY (BIP65)
#define hylk_ScriptVerificationFlags_CHECKSEQUENCEVERIFY ((hylk_ScriptVerificationFlags)(1U << 10)) //!< enable CHECKSEQUENCEVERIFY (BIP112)
#define hylk_ScriptVerificationFlags_WITNESS ((hylk_ScriptVerificationFlags)(1U << 11))             //!< enable WITNESS (BIP141)
#define hylk_ScriptVerificationFlags_TAPROOT ((hylk_ScriptVerificationFlags)(1U << 17))             //!< enable TAPROOT (BIPs 341 & 342)
#define hylk_ScriptVerificationFlags_ALL ((hylk_ScriptVerificationFlags)(hylk_ScriptVerificationFlags_P2SH |                \
                                                                         hylk_ScriptVerificationFlags_DERSIG |              \
                                                                         hylk_ScriptVerificationFlags_NULLDUMMY |           \
                                                                         hylk_ScriptVerificationFlags_CHECKLOCKTIMEVERIFY | \
                                                                         hylk_ScriptVerificationFlags_CHECKSEQUENCEVERIFY | \
                                                                         hylk_ScriptVerificationFlags_WITNESS |             \
                                                                         hylk_ScriptVerificationFlags_TAPROOT))

typedef uint8_t hylk_ChainType;
#define hylk_ChainType_MAINNET ((hylk_ChainType)(0))
#define hylk_ChainType_TESTNET ((hylk_ChainType)(1))
#define hylk_ChainType_TESTNET_4 ((hylk_ChainType)(2))
#define hylk_ChainType_SIGNET ((hylk_ChainType)(3))
#define hylk_ChainType_REGTEST ((hylk_ChainType)(4))

/** @name Transaction
 * Functions for working with transactions.
 */
///@{

/**
 * @brief Create a new transaction from the serialized data.
 *
 * @param[in] raw_transaction     Serialized transaction.
 * @param[in] raw_transaction_len Length of the serialized transaction.
 * @return                        The transaction, or null on error.
 */
HYLIUMKERNEL_API hylk_Transaction* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_create(
    const void* raw_transaction, size_t raw_transaction_len);

/**
 * @brief Copy a transaction. Transactions are reference counted, so this just
 * increments the reference count.
 *
 * @param[in] transaction Non-null.
 * @return                The copied transaction.
 */
HYLIUMKERNEL_API hylk_Transaction* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_copy(
    const hylk_Transaction* transaction) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the transaction through the passed in callback to bytes.
 * This is consensus serialization that is also used for the P2P network.
 *
 * @param[in] transaction Non-null.
 * @param[in] writer      Non-null, callback to a write bytes function.
 * @param[in] user_data   Holds a user-defined opaque structure that will be
 *                        passed back through the writer callback.
 * @return                0 on success.
 */
HYLIUMKERNEL_API int hylk_transaction_to_bytes(
    const hylk_Transaction* transaction,
    hylk_WriteBytes writer,
    void* user_data) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Get the number of outputs of a transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The number of outputs.
 */
HYLIUMKERNEL_API size_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_count_outputs(
    const hylk_Transaction* transaction) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction outputs at the provided index. The returned
 * transaction output is not owned and depends on the lifetime of the
 * transaction.
 *
 * @param[in] transaction  Non-null.
 * @param[in] output_index The index of the transaction output to be retrieved.
 * @return                 The transaction output
 */
HYLIUMKERNEL_API const hylk_TransactionOutput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_get_output_at(
    const hylk_Transaction* transaction, size_t output_index) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction input at the provided index. The returned
 * transaction input is not owned and depends on the lifetime of the
 * transaction.
 *
 * @param[in] transaction Non-null.
 * @param[in] input_index The index of the transaction input to be retrieved.
 * @return                 The transaction input
 */
HYLIUMKERNEL_API const hylk_TransactionInput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_get_input_at(
    const hylk_Transaction* transaction, size_t input_index) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the number of inputs of a transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The number of inputs.
 */
HYLIUMKERNEL_API size_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_count_inputs(
    const hylk_Transaction* transaction) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the txid of a transaction. The returned txid is not owned and
 * depends on the lifetime of the transaction.
 *
 * @param[in] transaction Non-null.
 * @return                The txid.
 */
HYLIUMKERNEL_API const hylk_Txid* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_get_txid(
    const hylk_Transaction* transaction) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction.
 */
HYLIUMKERNEL_API void hylk_transaction_destroy(hylk_Transaction* transaction);

///@}

/** @name ScriptPubkey
 * Functions for working with script pubkeys.
 */
///@{

/**
 * @brief Create a script pubkey from serialized data.
 * @param[in] script_pubkey     Serialized script pubkey.
 * @param[in] script_pubkey_len Length of the script pubkey data.
 * @return                      The script pubkey.
 */
HYLIUMKERNEL_API hylk_ScriptPubkey* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_script_pubkey_create(
    const void* script_pubkey, size_t script_pubkey_len);

/**
 * @brief Copy a script pubkey.
 *
 * @param[in] script_pubkey Non-null.
 * @return                  The copied script pubkey.
 */
HYLIUMKERNEL_API hylk_ScriptPubkey* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_script_pubkey_copy(
    const hylk_ScriptPubkey* script_pubkey) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Verify if the input at input_index of tx_to spends the script pubkey
 * under the constraints specified by flags. If the
 * `hylk_ScriptVerificationFlags_WITNESS` flag is set in the flags bitfield, the
 * amount parameter is used. If the taproot flag is set, the spent outputs
 * parameter is used to validate taproot transactions.
 *
 * @param[in] script_pubkey     Non-null, script pubkey to be spent.
 * @param[in] amount            Amount of the script pubkey's associated output. May be zero if
 *                              the witness flag is not set.
 * @param[in] tx_to             Non-null, transaction spending the script_pubkey.
 * @param[in] spent_outputs     Nullable if the taproot flag is not set. Points to an array of
 *                              outputs spent by the transaction.
 * @param[in] spent_outputs_len Length of the spent_outputs array.
 * @param[in] input_index       Index of the input in tx_to spending the script_pubkey.
 * @param[in] flags             Bitfield of hylk_ScriptVerificationFlags controlling validation constraints.
 * @param[out] status           Nullable, will be set to an error code if the operation fails, or OK otherwise.
 * @return                      1 if the script is valid, 0 otherwise.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_script_pubkey_verify(
    const hylk_ScriptPubkey* script_pubkey,
    int64_t amount,
    const hylk_Transaction* tx_to,
    const hylk_TransactionOutput** spent_outputs, size_t spent_outputs_len,
    unsigned int input_index,
    hylk_ScriptVerificationFlags flags,
    hylk_ScriptVerifyStatus* status) HYLIUMKERNEL_ARG_NONNULL(1, 3);

/**
 * @brief Serializes the script pubkey through the passed in callback to bytes.
 *
 * @param[in] script_pubkey Non-null.
 * @param[in] writer        Non-null, callback to a write bytes function.
 * @param[in] user_data     Holds a user-defined opaque structure that will be
 *                          passed back through the writer callback.
 * @return                  0 on success.
 */
HYLIUMKERNEL_API int hylk_script_pubkey_to_bytes(
    const hylk_ScriptPubkey* script_pubkey,
    hylk_WriteBytes writer,
    void* user_data) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the script pubkey.
 */
HYLIUMKERNEL_API void hylk_script_pubkey_destroy(hylk_ScriptPubkey* script_pubkey);

///@}

/** @name TransactionOutput
 * Functions for working with transaction outputs.
 */
///@{

/**
 * @brief Create a transaction output from a script pubkey and an amount.
 *
 * @param[in] script_pubkey Non-null.
 * @param[in] amount        The amount associated with the script pubkey for this output.
 * @return                  The transaction output.
 */
HYLIUMKERNEL_API hylk_TransactionOutput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_output_create(
    const hylk_ScriptPubkey* script_pubkey,
    int64_t amount) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the script pubkey of the output. The returned
 * script pubkey is not owned and depends on the lifetime of the
 * transaction output.
 *
 * @param[in] transaction_output Non-null.
 * @return                       The script pubkey.
 */
HYLIUMKERNEL_API const hylk_ScriptPubkey* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_output_get_script_pubkey(
    const hylk_TransactionOutput* transaction_output) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the amount in the output.
 *
 * @param[in] transaction_output Non-null.
 * @return                       The amount.
 */
HYLIUMKERNEL_API int64_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_output_get_amount(
    const hylk_TransactionOutput* transaction_output) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 *  @brief Copy a transaction output.
 *
 *  @param[in] transaction_output Non-null.
 *  @return                       The copied transaction output.
 */
HYLIUMKERNEL_API hylk_TransactionOutput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_output_copy(
    const hylk_TransactionOutput* transaction_output) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction output.
 */
HYLIUMKERNEL_API void hylk_transaction_output_destroy(hylk_TransactionOutput* transaction_output);

///@}

/** @name Logging
 * Logging-related functions.
 */
///@{

/**
 * @brief This disables the global internal logger. No log messages will be
 * buffered internally anymore once this is called and the buffer is cleared.
 * This function should only be called once and is not thread or re-entry safe.
 * Log messages will be buffered until this function is called, or a logging
 * connection is created. This must not be called while a logging connection
 * already exists.
 */
HYLIUMKERNEL_API void hylk_logging_disable();

/**
 * @brief Set some options for the global internal logger. This changes global
 * settings and will override settings for all existing @ref
 * hylk_LoggingConnection instances.
 *
 * @param[in] options Sets formatting options of the log messages.
 */
HYLIUMKERNEL_API void hylk_logging_set_options(const hylk_LoggingOptions options);

/**
 * @brief Set the log level of the global internal logger. This does not
 * enable the selected categories. Use @ref hylk_logging_enable_category to
 * start logging from a specific, or all categories. This changes a global
 * setting and will override settings for all existing
 * @ref hylk_LoggingConnection instances.
 *
 * @param[in] category If hylk_LogCategory_ALL is chosen, sets both the global fallback log level
 *                     used by all categories that don't have a specific level set, and also
 *                     sets the log level for messages logged with the hylk_LogCategory_ALL category itself.
 *                     For any other category, sets a category-specific log level that overrides
 *                     the global fallback for that category only.

 * @param[in] level    Log level at which the log category is set.
 */
HYLIUMKERNEL_API void hylk_logging_set_level_category(hylk_LogCategory category, hylk_LogLevel level);

/**
 * @brief Enable a specific log category for the global internal logger. This
 * changes a global setting and will override settings for all existing @ref
 * hylk_LoggingConnection instances.
 *
 * @param[in] category If hylk_LogCategory_ALL is chosen, all categories will be enabled.
 */
HYLIUMKERNEL_API void hylk_logging_enable_category(hylk_LogCategory category);

/**
 * @brief Disable a specific log category for the global internal logger. This
 * changes a global setting and will override settings for all existing @ref
 * hylk_LoggingConnection instances.
 *
 * @param[in] category If hylk_LogCategory_ALL is chosen, all categories will be disabled.
 */
HYLIUMKERNEL_API void hylk_logging_disable_category(hylk_LogCategory category);

/**
 * @brief Start logging messages through the provided callback. Log messages
 * produced before this function is first called are buffered and on calling this
 * function are logged immediately.
 *
 * @param[in] log_callback               Non-null, function through which messages will be logged.
 * @param[in] user_data                  Nullable, holds a user-defined opaque structure. Is passed back
 *                                       to the user through the callback. If the user_data_destroy_callback
 *                                       is also defined it is assumed that ownership of the user_data is passed
 *                                       to the created logging connection.
 * @param[in] user_data_destroy_callback Nullable, function for freeing the user data.
 * @return                               A new kernel logging connection, or null on error.
 */
HYLIUMKERNEL_API hylk_LoggingConnection* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_logging_connection_create(
    hylk_LogCallback log_callback,
    void* user_data,
    hylk_DestroyCallback user_data_destroy_callback) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Stop logging and destroy the logging connection.
 */
HYLIUMKERNEL_API void hylk_logging_connection_destroy(hylk_LoggingConnection* logging_connection);

///@}

/** @name ChainParameters
 * Functions for working with chain parameters.
 */
///@{

/**
 * @brief Creates a chain parameters struct with default parameters based on the
 * passed in chain type.
 *
 * @param[in] chain_type Controls the chain parameters type created.
 * @return               An allocated chain parameters opaque struct.
 */
HYLIUMKERNEL_API hylk_ChainParameters* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chain_parameters_create(
    const hylk_ChainType chain_type);

/**
 * Copy the chain parameters.
 */
HYLIUMKERNEL_API hylk_ChainParameters* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chain_parameters_copy(
    const hylk_ChainParameters* chain_parameters) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the chain parameters.
 */
HYLIUMKERNEL_API void hylk_chain_parameters_destroy(hylk_ChainParameters* chain_parameters);

///@}

/** @name ContextOptions
 * Functions for working with context options.
 */
///@{

/**
 * Creates an empty context options.
 */
HYLIUMKERNEL_API hylk_ContextOptions* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_context_options_create();

/**
 * @brief Sets the chain params for the context options. The context created
 * with the options will be configured for these chain parameters.
 *
 * @param[in] context_options  Non-null, previously created by @ref hylk_context_options_create.
 * @param[in] chain_parameters Is set to the context options.
 */
HYLIUMKERNEL_API void hylk_context_options_set_chainparams(
    hylk_ContextOptions* context_options,
    const hylk_ChainParameters* chain_parameters) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Set the kernel notifications for the context options. The context
 * created with the options will be configured with these notifications.
 *
 * @param[in] context_options Non-null, previously created by @ref hylk_context_options_create.
 * @param[in] notifications   Is set to the context options.
 */
HYLIUMKERNEL_API void hylk_context_options_set_notifications(
    hylk_ContextOptions* context_options,
    hylk_NotificationInterfaceCallbacks notifications) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Set the validation interface callbacks for the context options. The
 * context created with the options will be configured for these validation
 * interface callbacks. The callbacks will then be triggered from validation
 * events issued by the chainstate manager created from the same context.
 *
 * @param[in] context_options                Non-null, previously created with hylk_context_options_create.
 * @param[in] validation_interface_callbacks The callbacks used for passing validation information to the
 *                                           user.
 */
HYLIUMKERNEL_API void hylk_context_options_set_validation_interface(
    hylk_ContextOptions* context_options,
    hylk_ValidationInterfaceCallbacks validation_interface_callbacks) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the context options.
 */
HYLIUMKERNEL_API void hylk_context_options_destroy(hylk_ContextOptions* context_options);

///@}

/** @name Context
 * Functions for working with contexts.
 */
///@{

/**
 * @brief Create a new kernel context. If the options have not been previously
 * set, their corresponding fields will be initialized to default values; the
 * context will assume mainnet chain parameters and won't attempt to call the
 * kernel notification callbacks.
 *
 * @param[in] context_options Nullable, created by @ref hylk_context_options_create.
 * @return                    The allocated context, or null on error.
 */
HYLIUMKERNEL_API hylk_Context* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_context_create(
    const hylk_ContextOptions* context_options);

/**
 * Copy the context.
 */
HYLIUMKERNEL_API hylk_Context* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_context_copy(
    const hylk_Context* context) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Interrupt can be used to halt long-running validation functions like
 * when reindexing, importing or processing blocks.
 *
 * @param[in] context  Non-null.
 * @return             0 if the interrupt was successful, non-zero otherwise.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_context_interrupt(
    hylk_Context* context) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the context.
 */
HYLIUMKERNEL_API void hylk_context_destroy(hylk_Context* context);

///@}

/** @name BlockTreeEntry
 * Functions for working with block tree entries.
 */
///@{

/**
 * @brief Returns the previous block tree entry in the tree, or null if the current
 * block tree entry is the genesis block.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The previous block tree entry, or null on error or if the current block tree entry is the genesis block.
 */
HYLIUMKERNEL_API const hylk_BlockTreeEntry* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_tree_entry_get_previous(
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the height of a certain block tree entry.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The block height.
 */
HYLIUMKERNEL_API int32_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_tree_entry_get_height(
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the block hash associated with a block tree entry.
 *
 * @param[in] block_tree_entry Non-null.
 * @return                     The block hash.
 */
HYLIUMKERNEL_API const hylk_BlockHash* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_tree_entry_get_block_hash(
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two block tree entries are equal. Two block tree entries are equal when they
 * point to the same block.
 *
 * @param[in] entry1 Non-null.
 * @param[in] entry2 Non-null.
 * @return           1 if the block tree entries are equal, 0 otherwise.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_tree_entry_equals(
    const hylk_BlockTreeEntry* entry1, const hylk_BlockTreeEntry* entry2) HYLIUMKERNEL_ARG_NONNULL(1, 2);

///@}

/** @name ChainstateManagerOptions
 * Functions for working with chainstate manager options.
 */
///@{

/**
 * @brief Create options for the chainstate manager.
 *
 * @param[in] context          Non-null, the created options and through it the chainstate manager will
 *                             associate with this kernel context for the duration of their lifetimes.
 * @param[in] data_directory   Non-null, non-empty path string of the directory containing the
 *                             chainstate data. If the directory does not exist yet, it will be
 *                             created.
 * @param[in] blocks_directory Non-null, non-empty path string of the directory containing the block
 *                             data. If the directory does not exist yet, it will be created.
 * @return                     The allocated chainstate manager options, or null on error.
 */
HYLIUMKERNEL_API hylk_ChainstateManagerOptions* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_options_create(
    const hylk_Context* context,
    const char* data_directory,
    size_t data_directory_len,
    const char* blocks_directory,
    size_t blocks_directory_len) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Set the number of available worker threads used during validation.
 *
 * @param[in] chainstate_manager_options Non-null, options to be set.
 * @param[in] worker_threads             The number of worker threads that should be spawned in the thread pool
 *                                       used for validation. When set to 0 no parallel verification is done.
 *                                       The value range is clamped internally between 0 and 15.
 */
HYLIUMKERNEL_API void hylk_chainstate_manager_options_set_worker_threads_num(
    hylk_ChainstateManagerOptions* chainstate_manager_options,
    int worker_threads) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets wipe db in the options. In combination with calling
 * @ref hylk_chainstate_manager_import_blocks this triggers either a full reindex,
 * or a reindex of just the chainstate database.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref hylk_chainstate_manager_options_create.
 * @param[in] wipe_block_tree_db         Set wipe block tree db. Should only be 1 if wipe_chainstate_db is 1 too.
 * @param[in] wipe_chainstate_db         Set wipe chainstate db.
 * @return                               0 if the set was successful, non-zero if the set failed.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_options_set_wipe_dbs(
    hylk_ChainstateManagerOptions* chainstate_manager_options,
    int wipe_block_tree_db,
    int wipe_chainstate_db) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets block tree db in memory in the options.
 *
 * @param[in] chainstate_manager_options   Non-null, created by @ref hylk_chainstate_manager_options_create.
 * @param[in] block_tree_db_in_memory      Set block tree db in memory.
 */
HYLIUMKERNEL_API void hylk_chainstate_manager_options_update_block_tree_db_in_memory(
    hylk_ChainstateManagerOptions* chainstate_manager_options,
    int block_tree_db_in_memory) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Sets chainstate db in memory in the options.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref hylk_chainstate_manager_options_create.
 * @param[in] chainstate_db_in_memory    Set chainstate db in memory.
 */
HYLIUMKERNEL_API void hylk_chainstate_manager_options_update_chainstate_db_in_memory(
    hylk_ChainstateManagerOptions* chainstate_manager_options,
    int chainstate_db_in_memory) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the chainstate manager options.
 */
HYLIUMKERNEL_API void hylk_chainstate_manager_options_destroy(hylk_ChainstateManagerOptions* chainstate_manager_options);

///@}

/** @name ChainstateManager
 * Functions for chainstate management.
 */
///@{

/**
 * @brief Create a chainstate manager. This is the main object for many
 * validation tasks as well as for retrieving data from the chain and
 * interacting with its chainstate and indexes.
 *
 * @param[in] chainstate_manager_options Non-null, created by @ref hylk_chainstate_manager_options_create.
 * @return                               The allocated chainstate manager, or null on error.
 */
HYLIUMKERNEL_API hylk_ChainstateManager* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_create(
    const hylk_ChainstateManagerOptions* chainstate_manager_options) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Triggers the start of a reindex if the wipe options were previously
 * set for the chainstate manager. Can also import an array of existing block
 * files selected by the user.
 *
 * @param[in] chainstate_manager        Non-null.
 * @param[in] block_file_paths_data     Nullable, array of block files described by their full filesystem paths.
 * @param[in] block_file_paths_lens     Nullable, array containing the lengths of each of the paths.
 * @param[in] block_file_paths_data_len Length of the block_file_paths_data and block_file_paths_len arrays.
 * @return                              0 if the import blocks call was completed successfully, non-zero otherwise.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_import_blocks(
    hylk_ChainstateManager* chainstate_manager,
    const char** block_file_paths_data, size_t* block_file_paths_lens,
    size_t block_file_paths_data_len) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Process and validate the passed in block with the chainstate
 * manager. Processing first does checks on the block, and if these passed,
 * saves it to disk. It then validates the block against the utxo set. If it is
 * valid, the chain is extended with it. The return value is not indicative of
 * the block's validity. Detailed information on the validity of the block can
 * be retrieved by registering the `block_checked` callback in the validation
 * interface.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block              Non-null, block to be validated.
 *
 * @param[out] new_block         Nullable, will be set to 1 if this block was not processed before. Note that this means it
 *                               might also not be 1 if processing was attempted before, but the block was found invalid
 *                               before its data was persisted.
 * @return                       0 if processing the block was successful. Will also return 0 for valid, but duplicate blocks.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_process_block(
    hylk_ChainstateManager* chainstate_manager,
    const hylk_Block* block,
    int* new_block) HYLIUMKERNEL_ARG_NONNULL(1, 2, 3);

/**
 * @brief Returns the best known currently active chain. Its lifetime is
 * dependent on the chainstate manager. It can be thought of as a view on a
 * vector of block tree entries that form the best chain. The returned chain
 * reference always points to the currently active best chain. However, state
 * transitions within the chainstate manager (e.g., processing blocks) will
 * update the chain's contents. Data retrieved from this chain is only
 * consistent up to the point when new data is processed in the chainstate
 * manager. It is the user's responsibility to guard against these
 * inconsistencies.
 *
 * @param[in] chainstate_manager Non-null.
 * @return                       The chain.
 */
HYLIUMKERNEL_API const hylk_Chain* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_get_active_chain(
    const hylk_ChainstateManager* chainstate_manager) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Retrieve a block tree entry by its block hash.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_hash         Non-null.
 * @return                       The block tree entry of the block with the passed in hash, or null if
 *                               the block hash is not found.
 */
HYLIUMKERNEL_API const hylk_BlockTreeEntry* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chainstate_manager_get_block_tree_entry_by_hash(
    const hylk_ChainstateManager* chainstate_manager,
    const hylk_BlockHash* block_hash) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the chainstate manager.
 */
HYLIUMKERNEL_API void hylk_chainstate_manager_destroy(hylk_ChainstateManager* chainstate_manager);

///@}

/** @name Block
 * Functions for working with blocks.
 */
///@{

/**
 * @brief Reads the block the passed in block tree entry points to from disk and
 * returns it.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_tree_entry   Non-null.
 * @return                       The read out block, or null on error.
 */
HYLIUMKERNEL_API hylk_Block* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_read(
    const hylk_ChainstateManager* chainstate_manager,
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Parse a serialized raw block into a new block object.
 *
 * @param[in] raw_block     Serialized block.
 * @param[in] raw_block_len Length of the serialized block.
 * @return                  The allocated block, or null on error.
 */
HYLIUMKERNEL_API hylk_Block* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_create(
    const void* raw_block, size_t raw_block_len);

/**
 * @brief Copy a block. Blocks are reference counted, so this just increments
 * the reference count.
 *
 * @param[in] block Non-null.
 * @return          The copied block.
 */
HYLIUMKERNEL_API hylk_Block* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_copy(
    const hylk_Block* block) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Count the number of transactions contained in a block.
 *
 * @param[in] block Non-null.
 * @return          The number of transactions in the block.
 */
HYLIUMKERNEL_API size_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_count_transactions(
    const hylk_Block* block) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction at the provided index. The returned transaction
 * is not owned and depends on the lifetime of the block.
 *
 * @param[in] block             Non-null.
 * @param[in] transaction_index The index of the transaction to be retrieved.
 * @return                      The transaction.
 */
HYLIUMKERNEL_API const hylk_Transaction* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_get_transaction_at(
    const hylk_Block* block, size_t transaction_index) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Calculate and return the hash of a block.
 *
 * @param[in] block Non-null.
 * @return    The block hash.
 */
HYLIUMKERNEL_API hylk_BlockHash* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_get_hash(
    const hylk_Block* block) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the block through the passed in callback to bytes.
 * This is consensus serialization that is also used for the P2P network.
 *
 * @param[in] block     Non-null.
 * @param[in] writer    Non-null, callback to a write bytes function.
 * @param[in] user_data Holds a user-defined opaque structure that will be
 *                      passed back through the writer callback.
 * @return              0 on success.
 */
HYLIUMKERNEL_API int hylk_block_to_bytes(
    const hylk_Block* block,
    hylk_WriteBytes writer,
    void* user_data) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the block.
 */
HYLIUMKERNEL_API void hylk_block_destroy(hylk_Block* block);

///@}

/** @name BlockValidationState
 * Functions for working with block validation states.
 */
///@{

/**
 * Returns the validation mode from an opaque block validation state pointer.
 */
HYLIUMKERNEL_API hylk_ValidationMode hylk_block_validation_state_get_validation_mode(
    const hylk_BlockValidationState* block_validation_state) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Returns the validation result from an opaque block validation state pointer.
 */
HYLIUMKERNEL_API hylk_BlockValidationResult hylk_block_validation_state_get_block_validation_result(
    const hylk_BlockValidationState* block_validation_state) HYLIUMKERNEL_ARG_NONNULL(1);

///@}

/** @name Chain
 * Functions for working with the chain
 */
///@{

/**
 * @brief Return the height of the tip of the chain.
 *
 * @param[in] chain Non-null.
 * @return          The current height.
 */
HYLIUMKERNEL_API int32_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chain_get_height(
    const hylk_Chain* chain) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Retrieve a block tree entry by its height in the currently active chain.
 * Once retrieved there is no guarantee that it remains in the active chain.
 *
 * @param[in] chain        Non-null.
 * @param[in] block_height Height in the chain of the to be retrieved block tree entry.
 * @return                 The block tree entry at a certain height in the currently active chain, or null
 *                         if the height is out of bounds.
 */
HYLIUMKERNEL_API const hylk_BlockTreeEntry* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chain_get_by_height(
    const hylk_Chain* chain,
    int block_height) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Return true if the passed in chain contains the block tree entry.
 *
 * @param[in] chain            Non-null.
 * @param[in] block_tree_entry Non-null.
 * @return                     1 if the block_tree_entry is in the chain, 0 otherwise.
 *
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_chain_contains(
    const hylk_Chain* chain,
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1, 2);

///@}

/** @name BlockSpentOutputs
 * Functions for working with block spent outputs.
 */
///@{

/**
 * @brief Reads the block spent coins data the passed in block tree entry points to from
 * disk and returns it.
 *
 * @param[in] chainstate_manager Non-null.
 * @param[in] block_tree_entry   Non-null.
 * @return                       The read out block spent outputs, or null on error.
 */
HYLIUMKERNEL_API hylk_BlockSpentOutputs* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_spent_outputs_read(
    const hylk_ChainstateManager* chainstate_manager,
    const hylk_BlockTreeEntry* block_tree_entry) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Copy a block's spent outputs.
 *
 * @param[in] block_spent_outputs Non-null.
 * @return                        The copied block spent outputs.
 */
HYLIUMKERNEL_API hylk_BlockSpentOutputs* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_spent_outputs_copy(
    const hylk_BlockSpentOutputs* block_spent_outputs) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the number of transaction spent outputs whose data is contained in
 * block spent outputs.
 *
 * @param[in] block_spent_outputs Non-null.
 * @return                        The number of transaction spent outputs data in the block spent outputs.
 */
HYLIUMKERNEL_API size_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_spent_outputs_count(
    const hylk_BlockSpentOutputs* block_spent_outputs) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns a transaction spent outputs contained in the block spent
 * outputs at a certain index. The returned pointer is unowned and only valid
 * for the lifetime of block_spent_outputs.
 *
 * @param[in] block_spent_outputs             Non-null.
 * @param[in] transaction_spent_outputs_index The index of the transaction spent outputs within the block spent outputs.
 * @return                                    A transaction spent outputs pointer.
 */
HYLIUMKERNEL_API const hylk_TransactionSpentOutputs* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_spent_outputs_get_transaction_spent_outputs_at(
    const hylk_BlockSpentOutputs* block_spent_outputs,
    size_t transaction_spent_outputs_index) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the block spent outputs.
 */
HYLIUMKERNEL_API void hylk_block_spent_outputs_destroy(hylk_BlockSpentOutputs* block_spent_outputs);

///@}

/** @name TransactionSpentOutputs
 * Functions for working with the spent coins of a transaction
 */
///@{

/**
 * @brief Copy a transaction's spent outputs.
 *
 * @param[in] transaction_spent_outputs Non-null.
 * @return                              The copied transaction spent outputs.
 */
HYLIUMKERNEL_API hylk_TransactionSpentOutputs* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_spent_outputs_copy(
    const hylk_TransactionSpentOutputs* transaction_spent_outputs) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the number of previous transaction outputs contained in the
 * transaction spent outputs data.
 *
 * @param[in] transaction_spent_outputs Non-null
 * @return                              The number of spent transaction outputs for the transaction.
 */
HYLIUMKERNEL_API size_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_spent_outputs_count(
    const hylk_TransactionSpentOutputs* transaction_spent_outputs) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns a coin contained in the transaction spent outputs at a
 * certain index. The returned pointer is unowned and only valid for the
 * lifetime of transaction_spent_outputs.
 *
 * @param[in] transaction_spent_outputs Non-null.
 * @param[in] coin_index                The index of the to be retrieved coin within the
 *                                      transaction spent outputs.
 * @return                              A coin pointer.
 */
HYLIUMKERNEL_API const hylk_Coin* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_spent_outputs_get_coin_at(
    const hylk_TransactionSpentOutputs* transaction_spent_outputs,
    size_t coin_index) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction spent outputs.
 */
HYLIUMKERNEL_API void hylk_transaction_spent_outputs_destroy(hylk_TransactionSpentOutputs* transaction_spent_outputs);

///@}

/** @name Transaction Input
 * Functions for working with transaction inputs.
 */
///@{

/**
 * @brief Copy a transaction input.
 *
 * @param[in] transaction_input Non-null.
 * @return                      The copied transaction input.
 */
HYLIUMKERNEL_API hylk_TransactionInput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_input_copy(
    const hylk_TransactionInput* transaction_input) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the transaction out point. The returned transaction out point is
 * not owned and depends on the lifetime of the transaction.
 *
 * @param[in] transaction_input Non-null.
 * @return                      The transaction out point.
 */
HYLIUMKERNEL_API const hylk_TransactionOutPoint* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_input_get_out_point(
    const hylk_TransactionInput* transaction_input) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction input.
 */
HYLIUMKERNEL_API void hylk_transaction_input_destroy(hylk_TransactionInput* transaction_input);

///@}

/** @name Transaction Out Point
 * Functions for working with transaction out points.
 */
///@{

/**
 * @brief Copy a transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The copied transaction out point.
 */
HYLIUMKERNEL_API hylk_TransactionOutPoint* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_out_point_copy(
    const hylk_TransactionOutPoint* transaction_out_point) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the output position from the transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The output index.
 */
HYLIUMKERNEL_API uint32_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_out_point_get_index(
    const hylk_TransactionOutPoint* transaction_out_point) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Get the txid from the transaction out point. The returned txid is
 * not owned and depends on the lifetime of the transaction out point.
 *
 * @param[in] transaction_out_point Non-null.
 * @return                          The txid.
 */
HYLIUMKERNEL_API const hylk_Txid* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_transaction_out_point_get_txid(
    const hylk_TransactionOutPoint* transaction_out_point) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the transaction out point.
 */
HYLIUMKERNEL_API void hylk_transaction_out_point_destroy(hylk_TransactionOutPoint* transaction_out_point);

///@}

/** @name Txid
 * Functions for working with txids.
 */
///@{

/**
 * @brief Copy a txid.
 *
 * @param[in] txid Non-null.
 * @return         The copied txid.
 */
HYLIUMKERNEL_API hylk_Txid* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_txid_copy(
    const hylk_Txid* txid) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two txids are equal.
 *
 * @param[in] txid1 Non-null.
 * @param[in] txid2 Non-null.
 * @return          0 if the txid is not equal.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_txid_equals(
    const hylk_Txid* txid1, const hylk_Txid* txid2) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Serializes the txid to bytes.
 *
 * @param[in] txid    Non-null.
 * @param[out] output The serialized txid.
 */
HYLIUMKERNEL_API void hylk_txid_to_bytes(
    const hylk_Txid* txid, unsigned char output[32]) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the txid.
 */
HYLIUMKERNEL_API void hylk_txid_destroy(hylk_Txid* txid);

///@}

///@}

/** @name Coin
 * Functions for working with coins.
 */
///@{

/**
 * @brief Copy a coin.
 *
 * @param[in] coin Non-null.
 * @return         The copied coin.
 */
HYLIUMKERNEL_API hylk_Coin* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_coin_copy(
    const hylk_Coin* coin) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns the block height where the transaction that
 * created this coin was included in.
 *
 * @param[in] coin Non-null.
 * @return         The block height of the coin.
 */
HYLIUMKERNEL_API uint32_t HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_coin_confirmation_height(
    const hylk_Coin* coin) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Returns whether the containing transaction was a coinbase.
 *
 * @param[in] coin Non-null.
 * @return         1 if the coin is a coinbase coin, 0 otherwise.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_coin_is_coinbase(
    const hylk_Coin* coin) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Return the transaction output of a coin. The returned pointer is
 * unowned and only valid for the lifetime of the coin.
 *
 * @param[in] coin Non-null.
 * @return         A transaction output pointer.
 */
HYLIUMKERNEL_API const hylk_TransactionOutput* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_coin_get_output(
    const hylk_Coin* coin) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * Destroy the coin.
 */
HYLIUMKERNEL_API void hylk_coin_destroy(hylk_Coin* coin);

///@}

/** @name BlockHash
 * Functions for working with block hashes.
 */
///@{

/**
 * @brief Create a block hash from its raw data.
 */
HYLIUMKERNEL_API hylk_BlockHash* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_hash_create(
    const unsigned char block_hash[32]) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Check if two block hashes are equal.
 *
 * @param[in] hash1 Non-null.
 * @param[in] hash2 Non-null.
 * @return          0 if the block hashes are not equal.
 */
HYLIUMKERNEL_API int HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_hash_equals(
    const hylk_BlockHash* hash1, const hylk_BlockHash* hash2) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * @brief Copy a block hash.
 *
 * @param[in] block_hash Non-null.
 * @return               The copied block hash.
 */
HYLIUMKERNEL_API hylk_BlockHash* HYLIUMKERNEL_WARN_UNUSED_RESULT hylk_block_hash_copy(
    const hylk_BlockHash* block_hash) HYLIUMKERNEL_ARG_NONNULL(1);

/**
 * @brief Serializes the block hash to bytes.
 *
 * @param[in] block_hash     Non-null.
 * @param[in] output         The serialized block hash.
 */
HYLIUMKERNEL_API void hylk_block_hash_to_bytes(
    const hylk_BlockHash* block_hash, unsigned char output[32]) HYLIUMKERNEL_ARG_NONNULL(1, 2);

/**
 * Destroy the block hash.
 */
HYLIUMKERNEL_API void hylk_block_hash_destroy(hylk_BlockHash* block_hash);

///@}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // HYLIUM_KERNEL_HYLIUMKERNEL_H
