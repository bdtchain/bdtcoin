// Copyright (c) 2019-2020 Johir Uddin Sultan
// Copyright (c) 2020-2021 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_VALIDATION_H
#define BDTCOIN_VALIDATION_H

#if defined(HAVE_CONFIG_H)
#include <config/bdtcoin-config.h>
#endif

#include <amount.h>
#include <coins.h>
#include <crypto/common.h> // for ReadLE64
#include <fs.h>
#include <optional.h>
#include <policy/feerate.h>
#include <protocol.h> // For CMessageHeader::MessageStartChars
#include <script/script_error.h>
#include <sync.h>
#include <txmempool.h> // For CTxMemPool::cs
#include <txdb.h>
#include <versionbits.h>
#include <serialize.h>

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <key_io.h>
#include <bitset>

class CChainState;
class BlockValidationState;
class CBlockIndex;
class CBlockTreeDB;
class CBlockUndo;
class CChainParams;
class CInv;
class CConnman;
class CScriptCheck;
class CBlockPolicyEstimator;
class CTxMemPool;
class ChainstateManager;
class TxValidationState;
struct ChainTxData;

struct DisconnectedBlockTransactions;
struct PrecomputedTransactionData;
struct LockPoints;

/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 1000;
/** Default for -limitancestorcount, max number of in-mempool ancestors */
static const unsigned int DEFAULT_ANCESTOR_LIMIT = 25;
/** Default for -limitancestorsize, maximum kilobytes of tx + all in-mempool ancestors */
static const unsigned int DEFAULT_ANCESTOR_SIZE_LIMIT = 101;
/** Default for -limitdescendantcount, max number of in-mempool descendants */
static const unsigned int DEFAULT_DESCENDANT_LIMIT = 25;
/** Default for -limitdescendantsize, maximum kilobytes of in-mempool descendants */
static const unsigned int DEFAULT_DESCENDANT_SIZE_LIMIT = 101;
/** Default for -mempoolexpiry, expiration time for mempool transactions in hours */
static const unsigned int DEFAULT_MEMPOOL_EXPIRY = 336;
/** The maximum size of a blk?????.dat file (since 0.8) */
static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
/** Maximum number of dedicated script-checking threads allowed */
static const int MAX_SCRIPTCHECK_THREADS = 15;
/** -par default (number of script-checking threads, 0 = auto) */
static const int DEFAULT_SCRIPTCHECK_THREADS = 0;
static const int64_t DEFAULT_MAX_TIP_AGE = 24 * 60 * 60;
static const bool DEFAULT_CHECKPOINTS_ENABLED = true;
static const bool DEFAULT_TXINDEX = false;
static const char* const DEFAULT_BLOCKFILTERINDEX = "0";
/** Default for -persistmempool */
static const bool DEFAULT_PERSIST_MEMPOOL = true;
/** Default for using fee filter */
static const bool DEFAULT_FEEFILTER = true;
/** Default for -stopatheight */
static const int DEFAULT_STOPATHEIGHT = 0;
/** Block files containing a block-height within MIN_BLOCKS_TO_KEEP of ::ChainActive().Tip() will not be pruned. */
static const unsigned int MIN_BLOCKS_TO_KEEP = 288;
static const signed int DEFAULT_CHECKBLOCKS = 6;
static const unsigned int DEFAULT_CHECKLEVEL = 3;
// Require that user allocate at least 550 MiB for block & undo files (blk???.dat and rev???.dat)
// At 1MB per block, 288 blocks = 288MB.
// Add 15% for Undo data = 331MB
// Add 20% for Orphan block rate = 397MB
// We want the low water mark after pruning to be at least 397 MB and since we prune in
// full block file chunks, we need the high water mark which triggers the prune to be
// one 128MB block file + added 15% undo data = 147MB greater for a total of 545MB
// Setting the target to >= 550 MiB will make it likely we can respect the target.
static const uint64_t MIN_DISK_SPACE_FOR_BLOCK_FILES = 550 * 1024 * 1024;

struct BlockHasher
{
    // this used to call `GetCheapHash()` in uint256, which was later moved; the
    // cheap hash function simply calls ReadLE64() however, so the end result is
    // identical
    size_t operator()(const uint256& hash) const { return ReadLE64(hash.begin()); }
};

/** Current sync state passed to tip changed callbacks. */
enum class SynchronizationState {
    INIT_REINDEX,
    INIT_DOWNLOAD,
    POST_INIT
};

extern RecursiveMutex cs_main;
extern CBlockPolicyEstimator feeEstimator;
typedef std::unordered_map<uint256, CBlockIndex*, BlockHasher> BlockMap;
extern Mutex g_best_block_mutex;
extern std::condition_variable g_best_block_cv;
extern uint256 g_best_block;
extern std::atomic_bool fImporting;
extern std::atomic_bool fReindex;
/** Whether there are dedicated script-checking threads running.
 * False indicates all script checking is done on the main threadMessageHandler thread.
 */
extern bool g_parallel_script_checks;
extern bool fRequireStandard;
extern bool fCheckBlockIndex;
extern bool fCheckpointsEnabled;
/** A fee rate smaller than this is considered zero fee (for relaying, mining and transaction creation) */
extern CFeeRate minRelayTxFee;
/** If the tip is older than this (in seconds), the node is considered to be in initial block download. */
extern int64_t nMaxTipAge;

/** Block hash whose ancestors we will assume to have valid scripts without checking them. */
extern uint256 hashAssumeValid;

/** Minimum work we will assume exists on some valid chain. */
extern arith_uint256 nMinimumChainWork;

/** Best header we've seen so far (used for getheaders queries' starting points). */
extern CBlockIndex *pindexBestHeader;

/** Pruning-related variables and constants */
/** True if any block files have ever been pruned. */
extern bool fHavePruned;
/** True if we're running in -prune mode. */
extern bool fPruneMode;
/** Number of MiB of block files that we're trying to stay below. */
extern uint64_t nPruneTarget;
/** Documentation for argument 'checklevel'. */
extern const std::vector<std::string> CHECKLEVEL_DOC;

/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const FlatFilePos &pos, bool fReadOnly = false);
/** Translation to a filesystem path */
fs::path GetBlockPosFilename(const FlatFilePos &pos);
/** Import blocks from an external file */
void LoadExternalBlockFile(const CChainParams& chainparams, FILE* fileIn, FlatFilePos* dbp = nullptr);
/** Ensures we have a genesis block in the block tree, possibly writing one to disk. */
bool LoadGenesisBlock(const CChainParams& chainparams);
/** Unload database information */
void UnloadBlockIndex(CTxMemPool* mempool, ChainstateManager& chainman);
/** Run an instance of the script checking thread */
void ThreadScriptCheck(int worker_num);
/**
 * Return transaction from the block at block_index.
 * If block_index is not provided, fall back to mempool.
 * If mempool is not provided or the tx couldn't be found in mempool, fall back to g_txindex.
 *
 * @param[in]  block_index     The block to read from disk, or nullptr
 * @param[in]  mempool         If block_index is not provided, look in the mempool, if provided
 * @param[in]  hash            The txid
 * @param[in]  consensusParams The params
 * @param[out] hashBlock       The hash of block_index, if the tx was found via block_index
 * @returns                    The tx if found, otherwise nullptr
 */
CTransactionRef GetTransaction(const CBlockIndex* const block_index, const CTxMemPool* const mempool, const uint256& hash, const Consensus::Params& consensusParams, uint256& hashBlock);
/**
 * Find the best known block, and make it the tip of the block chain
 *
 * May not be called with cs_main held. May not be called in a
 * validationinterface callback.
 */
bool ActivateBestChain(BlockValidationState& state, const CChainParams& chainparams, std::shared_ptr<const CBlock> pblock = std::shared_ptr<const CBlock>());
CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams);

/** Guess verification progress (as a fraction between 0.0=genesis and 1.0=current tip). */
double GuessVerificationProgress(const ChainTxData& data, const CBlockIndex* pindex);

/** Calculate the amount of disk space the block & undo files currently use */
uint64_t CalculateCurrentUsage();

bool CheckProofOfProtocol(const CTransactionRef& ptx, const bool& fCheckPOP = true);

 struct WorkspaceData {
        WorkspaceData(const CTransactionRef& ptx) : m_ptx(ptx), m_hash(ptx->GetHash()) {}
        std::set<uint256> m_conflicts;
        CTxMemPool::setEntries m_all_conflicting;
        CTxMemPool::setEntries m_ancestors;
        std::unique_ptr<CTxMemPoolEntry> m_entry;

        bool m_replacement_transaction;
        CAmount m_modified_fees;
        CAmount m_conflicting_fees;
        size_t m_conflicting_size;

        const CTransactionRef& m_ptx;
        const uint256& m_hash;
    };
    
/**
 *  Actually unlink the specified files
 */
void UnlinkPrunedFiles(const std::set<int>& setFilesToPrune);

/** Prune block files up to a given height */
void PruneBlockFilesManual(int nManualPruneHeight);

/** (try to) add transaction to memory pool
 * plTxnReplaced will be appended to with all transactions replaced from mempool
 * @param[out] fee_out optional argument to return tx fee to the caller **/
bool AcceptToMemoryPool(CTxMemPool& pool, TxValidationState &state, const CTransactionRef &tx,
                        std::list<CTransactionRef>* plTxnReplaced,
                        bool bypass_limits, bool test_accept=false, CAmount* fee_out=nullptr) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/** Get the BIP9 state for a given deployment at the current tip. */
ThresholdState VersionBitsTipState(const Consensus::Params& params, Consensus::DeploymentPos pos);

/** Get the numerical statistics for the BIP9 state for a given deployment at the current tip. */
BIP9Stats VersionBitsTipStatistics(const Consensus::Params& params, Consensus::DeploymentPos pos);

/** Get the block height at which the BIP9 deployment switched into the state for the block building on the current tip. */
int VersionBitsTipStateSinceHeight(const Consensus::Params& params, Consensus::DeploymentPos pos);


/** Apply the effects of this transaction on the UTXO set represented by view */
void UpdateCoins(const CTransaction& tx, CCoinsViewCache& inputs, int nHeight);

/** Transaction validation functions */

/**
 * Check if transaction will be final in the next block to be created.
 *
 * Calls IsFinalTx() with current block height and appropriate block time.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckFinalTx(const CTransaction &tx, int flags = -1) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/**
 * Test whether the LockPoints height and time are still valid on the current chain
 */
bool TestLockPointValidity(const LockPoints* lp) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/**
 * Check if transaction will be BIP 68 final in the next block to be created.
 *
 * Simulates calling SequenceLocks() with data from the tip of the current active chain.
 * Optionally stores in LockPoints the resulting height and time calculated and the hash
 * of the block needed for calculation or skips the calculation and uses the LockPoints
 * passed in for evaluation.
 * The LockPoints should not be considered valid if CheckSequenceLocks returns false.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckSequenceLocks(const CTxMemPool& pool, const CTransaction& tx, int flags, LockPoints* lp = nullptr, bool useExistingLockPoints = false) EXCLUSIVE_LOCKS_REQUIRED(::cs_main, pool.cs);

/**
 * Closure representing one script verification
 * Note that this stores references to the spending transaction
 */
class CScriptCheck
{
private:
    CTxOut m_tx_out;
    const CTransaction *ptxTo;
    unsigned int nIn;
    unsigned int nFlags;
    bool cacheStore;
    ScriptError error;
    PrecomputedTransactionData *txdata;

public:
    CScriptCheck(): ptxTo(nullptr), nIn(0), nFlags(0), cacheStore(false), error(SCRIPT_ERR_UNKNOWN_ERROR) {}
    CScriptCheck(const CTxOut& outIn, const CTransaction& txToIn, unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn, PrecomputedTransactionData* txdataIn) :
        m_tx_out(outIn), ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn), cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR), txdata(txdataIn) { }

    bool operator()();

    void swap(CScriptCheck &check) {
        std::swap(ptxTo, check.ptxTo);
        std::swap(m_tx_out, check.m_tx_out);
        std::swap(nIn, check.nIn);
        std::swap(nFlags, check.nFlags);
        std::swap(cacheStore, check.cacheStore);
        std::swap(error, check.error);
        std::swap(txdata, check.txdata);
    }

    ScriptError GetScriptError() const { return error; }
};

/** Initializes the script-execution cache */
void InitScriptExecutionCache();


/** Functions for disk access for blocks */
bool ReadBlockFromDisk(CBlock& block, const FlatFilePos& pos, const Consensus::Params& consensusParams);
bool ReadBlockFromDisk(CBlock& block, const CBlockIndex* pindex, const Consensus::Params& consensusParams);
bool ReadRawBlockFromDisk(std::vector<uint8_t>& block, const FlatFilePos& pos, const CMessageHeader::MessageStartChars& message_start);
bool ReadRawBlockFromDisk(std::vector<uint8_t>& block, const CBlockIndex* pindex, const CMessageHeader::MessageStartChars& message_start);

bool UndoReadFromDisk(CBlockUndo& blockundo, const CBlockIndex* pindex);

/** Functions for validating blocks and updating the block tree */

/** Context-independent validity checks */
bool CheckBlock(const CBlock& block, BlockValidationState& state, const Consensus::Params& consensusParams, bool fCheckPOW = true, bool fCheckMerkleRoot = true);

/** Check a block is completely valid from start to finish (only works on top of our current best block) */
bool TestBlockValidity(BlockValidationState& state, const CChainParams& chainparams, const CBlock& block, CBlockIndex* pindexPrev, bool fCheckPOW = true, bool fCheckMerkleRoot = true) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/** Check whether witness commitments are required for a block, and whether to enforce NULLDUMMY (BIP 147) rules.
 *  Note that transaction witness validation rules are always enforced when P2SH is enforced. */
bool IsWitnessEnabled(const CBlockIndex* pindexPrev, const Consensus::Params& params);

/** Update uncommitted block structures (currently: only the witness reserved value). This is safe for submitted blocks. */
void UpdateUncommittedBlockStructures(CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& consensusParams);

/** Produce the necessary coinbase commitment for a block (modifies the hash, don't call for mined blocks). */
std::vector<unsigned char> GenerateCoinbaseCommitment(CBlock& block, const CBlockIndex* pindexPrev, const Consensus::Params& consensusParams);

/** RAII wrapper for VerifyDB: Verify consistency of the block and coin databases */
class CVerifyDB {
public:
    CVerifyDB();
    ~CVerifyDB();
    bool VerifyDB(const CChainParams& chainparams, CCoinsView *coinsview, int nCheckLevel, int nCheckDepth);
};

CBlockIndex* LookupBlockIndex(const uint256& hash) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/** Find the last common block between the parameter chain and a locator. */
CBlockIndex* FindForkInGlobalIndex(const CChain& chain, const CBlockLocator& locator) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

enum DisconnectResult
{
    DISCONNECT_OK,      // All good.
    DISCONNECT_UNCLEAN, // Rolled back, but UTXO set was inconsistent with block.
    DISCONNECT_FAILED   // Something else went wrong.
};

class ConnectTrace;

/** @see CChainState::FlushStateToDisk */
enum class FlushStateMode {
    NONE,
    IF_NEEDED,
    PERIODIC,
    ALWAYS
};

struct CBlockIndexWorkComparator
{
    bool operator()(const CBlockIndex *pa, const CBlockIndex *pb) const;
};

/**
 * Maintains a tree of blocks (stored in `m_block_index`) which is consulted
 * to determine where the most-work tip is.
 *
 * This data is used mostly in `CChainState` - information about, e.g.,
 * candidate tips is not maintained here.
 */
class BlockManager
{
    friend CChainState;

private:
    /* Calculate the block/rev files to delete based on height specified by user with RPC command pruneblockchain */
    void FindFilesToPruneManual(std::set<int>& setFilesToPrune, int nManualPruneHeight, int chain_tip_height);

    /**
     * Prune block and undo files (blk???.dat and undo???.dat) so that the disk space used is less than a user-defined target.
     * The user sets the target (in MB) on the command line or in config file.  This will be run on startup and whenever new
     * space is allocated in a block or undo file, staying below the target. Changing back to unpruned requires a reindex
     * (which in this case means the blockchain must be re-downloaded.)
     *
     * Pruning functions are called from FlushStateToDisk when the global fCheckForPruning flag has been set.
     * Block and undo files are deleted in lock-step (when blk00003.dat is deleted, so is rev00003.dat.)
     * Pruning cannot take place until the longest chain is at least a certain length (100000 on mainnet, 1000 on testnet, 1000 on regtest).
     * Pruning will never delete a block within a defined distance (currently 288) from the active chain's tip.
     * The block index is updated by unsetting HAVE_DATA and HAVE_UNDO for any blocks that were stored in the deleted files.
     * A db flag records the fact that at least some block files have been pruned.
     *
     * @param[out]   setFilesToPrune   The set of file indices that can be unlinked will be returned
     */
    void FindFilesToPrune(std::set<int>& setFilesToPrune, uint64_t nPruneAfterHeight, int chain_tip_height, bool is_ibd);

public:
    BlockMap m_block_index GUARDED_BY(cs_main);

    /** In order to efficiently track invalidity of headers, we keep the set of
      * blocks which we tried to connect and found to be invalid here (ie which
      * were set to BLOCK_FAILED_VALID since the last restart). We can then
      * walk this set and check if a new header is a descendant of something in
      * this set, preventing us from having to walk m_block_index when we try
      * to connect a bad block and fail.
      *
      * While this is more complicated than marking everything which descends
      * from an invalid block as invalid at the time we discover it to be
      * invalid, doing so would require walking all of m_block_index to find all
      * descendants. Since this case should be very rare, keeping track of all
      * BLOCK_FAILED_VALID blocks in a set should be just fine and work just as
      * well.
      *
      * Because we already walk m_block_index in height-order at startup, we go
      * ahead and mark descendants of invalid blocks as FAILED_CHILD at that time,
      * instead of putting things in this set.
      */
    std::set<CBlockIndex*> m_failed_blocks;

    /**
     * All pairs A->B, where A (or one of its ancestors) misses transactions, but B has transactions.
     * Pruned nodes may have entries where B is missing data.
     */
    std::multimap<CBlockIndex*, CBlockIndex*> m_blocks_unlinked;

    /**
     * Load the blocktree off disk and into memory. Populate certain metadata
     * per index entry (nStatus, nChainWork, nTimeMax, etc.) as well as peripheral
     * collections like setDirtyBlockIndex.
     *
     * @param[out] block_index_candidates  Fill this set with any valid blocks for
     *                                     which we've downloaded all transactions.
     */
    bool LoadBlockIndex(
        const Consensus::Params& consensus_params,
        CBlockTreeDB& blocktree,
        std::set<CBlockIndex*, CBlockIndexWorkComparator>& block_index_candidates)
        EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    /** Clear all data members. */
    void Unload() EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    CBlockIndex* AddToBlockIndex(const CBlockHeader& block) EXCLUSIVE_LOCKS_REQUIRED(cs_main);
    /** Create a new block index entry for a given block hash */
    CBlockIndex* InsertBlockIndex(const uint256& hash) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    //! Mark one block file as pruned (modify associated database entries)
    void PruneOneBlockFile(const int fileNumber) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    /**
     * If a block header hasn't already been seen, call CheckBlockHeader on it, ensure
     * that it doesn't descend from an invalid block, and then add it to m_block_index.
     */
    bool AcceptBlockHeader(
        const CBlockHeader& block,
        BlockValidationState& state,
        const CChainParams& chainparams,
        CBlockIndex** ppindex) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    ~BlockManager() {
        Unload();
    }
};

/**
 * A convenience class for constructing the CCoinsView* hierarchy used
 * to facilitate access to the UTXO set.
 *
 * This class consists of an arrangement of layered CCoinsView objects,
 * preferring to store and retrieve coins in memory via `m_cacheview` but
 * ultimately falling back on cache misses to the canonical store of UTXOs on
 * disk, `m_dbview`.
 */
class CoinsViews {

public:
    //! The lowest level of the CoinsViews cache hierarchy sits in a leveldb database on disk.
    //! All unspent coins reside in this store.
    CCoinsViewDB m_dbview GUARDED_BY(cs_main);

    //! This view wraps access to the leveldb instance and handles read errors gracefully.
    CCoinsViewErrorCatcher m_catcherview GUARDED_BY(cs_main);

    //! This is the top layer of the cache hierarchy - it keeps as many coins in memory as
    //! can fit per the dbcache setting.
    std::unique_ptr<CCoinsViewCache> m_cacheview GUARDED_BY(cs_main);

    //! This constructor initializes CCoinsViewDB and CCoinsViewErrorCatcher instances, but it
    //! *does not* create a CCoinsViewCache instance by default. This is done separately because the
    //! presence of the cache has implications on whether or not we're allowed to flush the cache's
    //! state to disk, which should not be done until the health of the database is verified.
    //!
    //! All arguments forwarded onto CCoinsViewDB.
    CoinsViews(std::string ldb_name, size_t cache_size_bytes, bool in_memory, bool should_wipe);

    //! Initialize the CCoinsViewCache member.
    void InitCache() EXCLUSIVE_LOCKS_REQUIRED(::cs_main);
};

enum class CoinsCacheSizeState
{
    //! The coins cache is in immediate need of a flush.
    CRITICAL = 2,
    //! The cache is at >= 90% capacity.
    LARGE = 1,
    OK = 0
};

/**
 * CChainState stores and provides an API to update our local knowledge of the
 * current best chain.
 *
 * Eventually, the API here is targeted at being exposed externally as a
 * consumable libconsensus library, so any functions added must only call
 * other class member functions, pure functions in other parts of the consensus
 * library, callbacks via the validation interface, or read/write-to-disk
 * functions (eventually this will also be via callbacks).
 *
 * Anything that is contingent on the current tip of the chain is stored here,
 * whereas block information and metadata independent of the current tip is
 * kept in `BlockMetadataManager`.
 */
class CChainState
{
protected:
    /**
     * Every received block is assigned a unique and increasing identifier, so we
     * know which one to give priority in case of a fork.
     */
    RecursiveMutex cs_nBlockSequenceId;
    /** Blocks loaded from disk are assigned id 0, so start the counter at 1. */
    int32_t nBlockSequenceId = 1;
    /** Decreasing counter (used by subsequent preciousblock calls). */
    int32_t nBlockReverseSequenceId = -1;
    /** chainwork for the last block that preciousblock has been applied to. */
    arith_uint256 nLastPreciousChainwork = 0;

    /**
     * the ChainState CriticalSection
     * A lock that must be held when modifying this ChainState - held in ActivateBestChain()
     */
    RecursiveMutex m_cs_chainstate;

    /**
     * Whether this chainstate is undergoing initial block download.
     *
     * Mutable because we need to be able to mark IsInitialBlockDownload()
     * const, which latches this for caching purposes.
     */
    mutable std::atomic<bool> m_cached_finished_ibd{false};

    //! Reference to a BlockManager instance which itself is shared across all
    //! CChainState instances. Keeping a local reference allows us to test more
    //! easily as opposed to referencing a global.
    BlockManager& m_blockman;

    //! mempool that is kept in sync with the chain
    CTxMemPool& m_mempool;

    //! Manages the UTXO set, which is a reflection of the contents of `m_chain`.
    std::unique_ptr<CoinsViews> m_coins_views;

public:
    explicit CChainState(CTxMemPool& mempool, BlockManager& blockman, uint256 from_snapshot_blockhash = uint256());

    /**
     * Initialize the CoinsViews UTXO set database management data structures. The in-memory
     * cache is initialized separately.
     *
     * All parameters forwarded to CoinsViews.
     */
    void InitCoinsDB(
        size_t cache_size_bytes,
        bool in_memory,
        bool should_wipe,
        std::string leveldb_name = "chainstate");

    //! Initialize the in-memory coins cache (to be done after the health of the on-disk database
    //! is verified).
    void InitCoinsCache(size_t cache_size_bytes) EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    //! @returns whether or not the CoinsViews object has been fully initialized and we can
    //!          safely flush this object to disk.
    bool CanFlushToDisk() EXCLUSIVE_LOCKS_REQUIRED(cs_main) {
        return m_coins_views && m_coins_views->m_cacheview;
    }

    //! The current chain of blockheaders we consult and build on.
    //! @see CChain, CBlockIndex.
    CChain m_chain;

    /**
     * The blockhash which is the base of the snapshot this chainstate was created from.
     *
     * IsNull() if this chainstate was not created from a snapshot.
     */
    const uint256 m_from_snapshot_blockhash{};

    /**
     * The set of all CBlockIndex entries with BLOCK_VALID_TRANSACTIONS (for itself and all ancestors) and
     * as good as our current tip or better. Entries may be failed, though, and pruning nodes may be
     * missing the data for the block.
     */
    std::set<CBlockIndex*, CBlockIndexWorkComparator> setBlockIndexCandidates;

    //! @returns A reference to the in-memory cache of the UTXO set.
    CCoinsViewCache& CoinsTip() EXCLUSIVE_LOCKS_REQUIRED(cs_main)
    {
        assert(m_coins_views->m_cacheview);
        return *m_coins_views->m_cacheview.get();
    }

    //! @returns A reference to the on-disk UTXO set database.
    CCoinsViewDB& CoinsDB() EXCLUSIVE_LOCKS_REQUIRED(cs_main)
    {
        return m_coins_views->m_dbview;
    }

    //! @returns A reference to a wrapped view of the in-memory UTXO set that
    //!     handles disk read errors gracefully.
    CCoinsViewErrorCatcher& CoinsErrorCatcher() EXCLUSIVE_LOCKS_REQUIRED(cs_main)
    {
        return m_coins_views->m_catcherview;
    }

    //! Destructs all objects related to accessing the UTXO set.
    void ResetCoinsViews() { m_coins_views.reset(); }

    //! The cache size of the on-disk coins view.
    size_t m_coinsdb_cache_size_bytes{0};

    //! The cache size of the in-memory coins view.
    size_t m_coinstip_cache_size_bytes{0};

    //! Resize the CoinsViews caches dynamically and flush state to disk.
    //! @returns true unless an error occurred during the flush.
    bool ResizeCoinsCaches(size_t coinstip_size, size_t coinsdb_size)
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    /**
     * Update the on-disk chain state.
     * The caches and indexes are flushed depending on the mode we're called with
     * if they're too large, if it's been a while since the last write,
     * or always and in all cases if we're in prune mode and are deleting files.
     *
     * If FlushStateMode::NONE is used, then FlushStateToDisk(...) won't do anything
     * besides checking if we need to prune.
     *
     * @returns true unless a system error occurred
     */
    bool FlushStateToDisk(
        const CChainParams& chainparams,
        BlockValidationState &state,
        FlushStateMode mode,
        int nManualPruneHeight = 0);

    //! Unconditionally flush all changes to disk.
    void ForceFlushStateToDisk();

    //! Prune blockfiles from the disk if necessary and then flush chainstate changes
    //! if we pruned.
    void PruneAndFlush();

    /**
     * Make the best chain active, in multiple steps. The result is either failure
     * or an activated best chain. pblock is either nullptr or a pointer to a block
     * that is already loaded (to avoid loading it again from disk).
     *
     * ActivateBestChain is split into steps (see ActivateBestChainStep) so that
     * we avoid holding cs_main for an extended period of time; the length of this
     * call may be quite long during reindexing or a substantial reorg.
     *
     * May not be called with cs_main held. May not be called in a
     * validationinterface callback.
     *
     * @returns true unless a system error occurred
     */
    bool ActivateBestChain(
        BlockValidationState& state,
        const CChainParams& chainparams,
        std::shared_ptr<const CBlock> pblock) LOCKS_EXCLUDED(cs_main);

    bool AcceptBlock(const std::shared_ptr<const CBlock>& pblock, BlockValidationState& state, const CChainParams& chainparams, CBlockIndex** ppindex, bool fRequested, const FlatFilePos* dbp, bool* fNewBlock) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    // Block (dis)connection on a given view:
    DisconnectResult DisconnectBlock(const CBlock& block, const CBlockIndex* pindex, CCoinsViewCache& view);
    bool ConnectBlock(const CBlock& block, BlockValidationState& state, CBlockIndex* pindex,
                      CCoinsViewCache& view, const CChainParams& chainparams, bool fJustCheck = false) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    // Apply the effects of a block disconnection on the UTXO set.
    bool DisconnectTip(BlockValidationState& state, const CChainParams& chainparams, DisconnectedBlockTransactions* disconnectpool) EXCLUSIVE_LOCKS_REQUIRED(cs_main, m_mempool.cs);

    // Manual block validity manipulation:
    bool PreciousBlock(BlockValidationState& state, const CChainParams& params, CBlockIndex* pindex) LOCKS_EXCLUDED(cs_main);
    bool InvalidateBlock(BlockValidationState& state, const CChainParams& chainparams, CBlockIndex* pindex) LOCKS_EXCLUDED(cs_main);
    void ResetBlockFailureFlags(CBlockIndex* pindex) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    /** Replay blocks that aren't fully applied to the database. */
    bool ReplayBlocks(const CChainParams& params);
    bool RewindBlockIndex(const CChainParams& params) LOCKS_EXCLUDED(cs_main);
    bool LoadGenesisBlock(const CChainParams& chainparams);

    void PruneBlockIndexCandidates();

    void UnloadBlockIndex();

    /** Check whether we are doing an initial block download (synchronizing from disk or network) */
    bool IsInitialBlockDownload() const;

    /**
     * Make various assertions about the state of the block index.
     *
     * By default this only executes fully when using the Regtest chain; see: fCheckBlockIndex.
     */
    void CheckBlockIndex(const Consensus::Params& consensusParams);

    /** Load the persisted mempool from disk */
    void LoadMempool(const ArgsManager& args);

    /** Update the chain tip based on database information, i.e. CoinsTip()'s best block. */
    bool LoadChainTip(const CChainParams& chainparams) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    //! Dictates whether we need to flush the cache to disk or not.
    //!
    //! @return the state of the size of the coins cache.
    CoinsCacheSizeState GetCoinsCacheSizeState(const CTxMemPool* tx_pool)
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    CoinsCacheSizeState GetCoinsCacheSizeState(
        const CTxMemPool* tx_pool,
        size_t max_coins_cache_size_bytes,
        size_t max_mempool_size_bytes) EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    std::string ToString() EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

private:
    bool ActivateBestChainStep(BlockValidationState& state, const CChainParams& chainparams, CBlockIndex* pindexMostWork, const std::shared_ptr<const CBlock>& pblock, bool& fInvalidFound, ConnectTrace& connectTrace) EXCLUSIVE_LOCKS_REQUIRED(cs_main, m_mempool.cs);
    bool ConnectTip(BlockValidationState& state, const CChainParams& chainparams, CBlockIndex* pindexNew, const std::shared_ptr<const CBlock>& pblock, ConnectTrace& connectTrace, DisconnectedBlockTransactions& disconnectpool) EXCLUSIVE_LOCKS_REQUIRED(cs_main, m_mempool.cs);

    void InvalidBlockFound(CBlockIndex *pindex, const BlockValidationState &state) EXCLUSIVE_LOCKS_REQUIRED(cs_main);
    CBlockIndex* FindMostWorkChain() EXCLUSIVE_LOCKS_REQUIRED(cs_main);
    void ReceivedBlockTransactions(const CBlock& block, CBlockIndex* pindexNew, const FlatFilePos& pos, const Consensus::Params& consensusParams) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    bool RollforwardBlock(const CBlockIndex* pindex, CCoinsViewCache& inputs, const CChainParams& params) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    //! Mark a block as not having block data
    void EraseBlockData(CBlockIndex* index) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    friend ChainstateManager;
};

/** Mark a block as precious and reorganize.
 *
 * May not be called in a
 * validationinterface callback.
 */
bool PreciousBlock(BlockValidationState& state, const CChainParams& params, CBlockIndex *pindex) LOCKS_EXCLUDED(cs_main);

/** Mark a block as invalid. */
bool InvalidateBlock(BlockValidationState& state, const CChainParams& chainparams, CBlockIndex* pindex) LOCKS_EXCLUDED(cs_main);

/** Remove invalidity status from a block and its descendants. */
void ResetBlockFailureFlags(CBlockIndex* pindex) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

/**
 * Provides an interface for creating and interacting with one or two
 * chainstates: an IBD chainstate generated by downloading blocks, and
 * an optional snapshot chainstate loaded from a UTXO snapshot. Managed
 * chainstates can be maintained at different heights simultaneously.
 *
 * This class provides abstractions that allow the retrieval of the current
 * most-work chainstate ("Active") as well as chainstates which may be in
 * background use to validate UTXO snapshots.
 *
 * Definitions:
 *
 * *IBD chainstate*: a chainstate whose current state has been "fully"
 *   validated by the initial block download process.
 *
 * *Snapshot chainstate*: a chainstate populated by loading in an
 *    assumeutxo UTXO snapshot.
 *
 * *Active chainstate*: the chainstate containing the current most-work
 *    chain. Consulted by most parts of the system (net_processing,
 *    wallet) as a reflection of the current chain and UTXO set.
 *    This may either be an IBD chainstate or a snapshot chainstate.
 *
 * *Background IBD chainstate*: an IBD chainstate for which the
 *    IBD process is happening in the background while use of the
 *    active (snapshot) chainstate allows the rest of the system to function.
 *
 * *Validated chainstate*: the most-work chainstate which has been validated
 *   locally via initial block download. This will be the snapshot chainstate
 *   if a snapshot was loaded and all blocks up to the snapshot starting point
 *   have been downloaded and validated (via background validation), otherwise
 *   it will be the IBD chainstate.
 */
class ChainstateManager
{
private:
    //! The chainstate used under normal operation (i.e. "regular" IBD) or, if
    //! a snapshot is in use, for background validation.
    //!
    //! Its contents (including on-disk data) will be deleted *upon shutdown*
    //! after background validation of the snapshot has completed. We do not
    //! free the chainstate contents immediately after it finishes validation
    //! to cautiously avoid a case where some other part of the system is still
    //! using this pointer (e.g. net_processing).
    //!
    //! Once this pointer is set to a corresponding chainstate, it will not
    //! be reset until init.cpp:Shutdown(). This means it is safe to acquire
    //! the contents of this pointer with ::cs_main held, release the lock,
    //! and then use the reference without concern of it being deconstructed.
    //!
    //! This is especially important when, e.g., calling ActivateBestChain()
    //! on all chainstates because we are not able to hold ::cs_main going into
    //! that call.
    std::unique_ptr<CChainState> m_ibd_chainstate;

    //! A chainstate initialized on the basis of a UTXO snapshot. If this is
    //! non-null, it is always our active chainstate.
    //!
    //! Once this pointer is set to a corresponding chainstate, it will not
    //! be reset until init.cpp:Shutdown(). This means it is safe to acquire
    //! the contents of this pointer with ::cs_main held, release the lock,
    //! and then use the reference without concern of it being deconstructed.
    //!
    //! This is especially important when, e.g., calling ActivateBestChain()
    //! on all chainstates because we are not able to hold ::cs_main going into
    //! that call.
    std::unique_ptr<CChainState> m_snapshot_chainstate;

    //! Points to either the ibd or snapshot chainstate; indicates our
    //! most-work chain.
    //!
    //! Once this pointer is set to a corresponding chainstate, it will not
    //! be reset until init.cpp:Shutdown(). This means it is safe to acquire
    //! the contents of this pointer with ::cs_main held, release the lock,
    //! and then use the reference without concern of it being deconstructed.
    //!
    //! This is especially important when, e.g., calling ActivateBestChain()
    //! on all chainstates because we are not able to hold ::cs_main going into
    //! that call.
    CChainState* m_active_chainstate{nullptr};

    //! If true, the assumed-valid chainstate has been fully validated
    //! by the background validation chainstate.
    bool m_snapshot_validated{false};

    // For access to m_active_chainstate.
    friend CChainState& ChainstateActive();
    friend CChain& ChainActive();

public:
    //! A single BlockManager instance is shared across each constructed
    //! chainstate to avoid duplicating block metadata.
    BlockManager m_blockman GUARDED_BY(::cs_main);

    //! The total number of bytes available for us to use across all in-memory
    //! coins caches. This will be split somehow across chainstates.
    int64_t m_total_coinstip_cache{0};
    //
    //! The total number of bytes available for us to use across all leveldb
    //! coins databases. This will be split somehow across chainstates.
    int64_t m_total_coinsdb_cache{0};

    //! Instantiate a new chainstate and assign it based upon whether it is
    //! from a snapshot.
    //!
    //! @param[in] mempool              The mempool to pass to the chainstate
    //                                  constructor
    //! @param[in] snapshot_blockhash   If given, signify that this chainstate
    //!                                 is based on a snapshot.
    CChainState& InitializeChainstate(CTxMemPool& mempool, const uint256& snapshot_blockhash = uint256())
        EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    //! Get all chainstates currently being used.
    std::vector<CChainState*> GetAll();

    //! The most-work chain.
    CChainState& ActiveChainstate() const;
    CChain& ActiveChain() const { return ActiveChainstate().m_chain; }
    int ActiveHeight() const { return ActiveChain().Height(); }
    CBlockIndex* ActiveTip() const { return ActiveChain().Tip(); }

    BlockMap& BlockIndex() EXCLUSIVE_LOCKS_REQUIRED(::cs_main)
    {
        return m_blockman.m_block_index;
    }

    bool IsSnapshotActive() const;

    Optional<uint256> SnapshotBlockhash() const;

    //! Is there a snapshot in use and has it been fully validated?
    bool IsSnapshotValidated() const { return m_snapshot_validated; }

    //! @returns true if this chainstate is being used to validate an active
    //!          snapshot in the background.
    bool IsBackgroundIBD(CChainState* chainstate) const;

    //! Return the most-work chainstate that has been fully validated.
    //!
    //! During background validation of a snapshot, this is the IBD chain. After
    //! background validation has completed, this is the snapshot chain.
    CChainState& ValidatedChainstate() const;

    CChain& ValidatedChain() const { return ValidatedChainstate().m_chain; }
    CBlockIndex* ValidatedTip() const { return ValidatedChain().Tip(); }

    /**
     * Process an incoming block. This only returns after the best known valid
     * block is made active. Note that it does not, however, guarantee that the
     * specific block passed to it has been checked for validity!
     *
     * If you want to *possibly* get feedback on whether pblock is valid, you must
     * install a CValidationInterface (see validationinterface.h) - this will have
     * its BlockChecked method called whenever *any* block completes validation.
     *
     * Note that we guarantee that either the proof-of-work is valid on pblock, or
     * (and possibly also) BlockChecked will have been called.
     *
     * May not be called in a
     * validationinterface callback.
     *
     * @param[in]   pblock  The block we want to process.
     * @param[in]   fForceProcessing Process this block even if unrequested; used for non-network block sources.
     * @param[out]  fNewBlock A boolean which is set to indicate if the block was first received via this call
     * @returns     If the block was processed, independently of block validity
     */
    bool ProcessNewBlock(const CChainParams& chainparams, const std::shared_ptr<const CBlock> pblock, bool fForceProcessing, bool* fNewBlock) LOCKS_EXCLUDED(cs_main);

    /**
     * Process incoming block headers.
     *
     * May not be called in a
     * validationinterface callback.
     *
     * @param[in]  block The block headers themselves
     * @param[out] state This may be set to an Error state if any error occurred processing them
     * @param[in]  chainparams The params for the chain we want to connect to
     * @param[out] ppindex If set, the pointer will be set to point to the last new block index object for the given headers
     */
    bool ProcessNewBlockHeaders(const std::vector<CBlockHeader>& block, BlockValidationState& state, const CChainParams& chainparams, const CBlockIndex** ppindex = nullptr) LOCKS_EXCLUDED(cs_main);

    //! Load the block tree and coins database from disk, initializing state if we're running with -reindex
    bool LoadBlockIndex(const CChainParams& chainparams) EXCLUSIVE_LOCKS_REQUIRED(cs_main);

    //! Unload block index and chain data before shutdown.
    void Unload() EXCLUSIVE_LOCKS_REQUIRED(::cs_main);

    //! Clear (deconstruct) chainstate data.
    void Reset();

    //! Check to see if caches are out of balance and if so, call
    //! ResizeCoinsCaches() as needed.
    void MaybeRebalanceCaches() EXCLUSIVE_LOCKS_REQUIRED(::cs_main);
};

/** DEPRECATED! Please use node.chainman instead. May only be used in validation.cpp internally */
extern ChainstateManager g_chainman GUARDED_BY(::cs_main);

/** Please prefer the identical ChainstateManager::ActiveChainstate */
CChainState& ChainstateActive();

/** Please prefer the identical ChainstateManager::ActiveChain */
CChain& ChainActive();

/** Global variable that points to the active block tree (protected by cs_main) */
extern std::unique_ptr<CBlockTreeDB> pblocktree;

/**
 * Return the spend height, which is one more than the inputs.GetBestBlock().
 * While checking, GetBestBlock() refers to the parent block. (protected by cs_main)
 * This is also true for mempool checks.
 */
int GetSpendHeight(const CCoinsViewCache& inputs);

extern VersionBitsCache versionbitscache;

/**
 * Determine what nVersion a new block should use.
 */
int32_t ComputeBlockVersion(const CBlockIndex* pindexPrev, const Consensus::Params& params);

/** Get block file info entry for one block file */
CBlockFileInfo* GetBlockFileInfo(size_t n);

/** Dump the mempool to disk. */
bool DumpMempool(const CTxMemPool& pool);

/** Load the mempool from disk. */
bool LoadMempool(CTxMemPool& pool);

//! Check whether the block associated with this index entry is pruned or not.
inline bool IsBlockPruned(const CBlockIndex* pblockindex)
{
    return (fHavePruned && !(pblockindex->nStatus & BLOCK_HAVE_DATA) && pblockindex->nTx > 0);
}

std::string StrToBin(std::string words);

const std::vector<std::string> blockCheckPoint = {
"01100010010101010111101001101110011000010110000101001110011101000111010000110001011001010111010101110001010011100111100001100011001100110110110101110000011101110111100001110111011001010101100000110100001101010101001101101010010000110111001101000110011110010101011101011000",
"01100010010010100101100001101001010100000110111100110011001110000110110101000111001101100111000100110100011011010110111101100001011100110110010001000010010011000101010101011001010011100111001101111000011010000101011100110011010000010111001101101101011101100100010101001110",
"01100010010101100111100101001000011000010101100101010110010000100110001101000101011010110110101001011010010101100110000101100010010011010110010001100011001101100011100001110101010100010110111000110011011011100110101101010010011100110100101001010001011110010110111000110011",
"01100010010001010111100101110100011001110011100101000100010011010111001100110011011100000100001001101011010010110111100001000001010000100111011001101101010011100111010001001101011010100100110101110111010011000100011001110110001110010111000001110010010010000110010101001101",
"01100010011000010110111101100010011110100011010001100101010101110100011101001010011010100110101001000010001100110100101001111001011101000101010101110000001110010100011100110001010011100100100001001011011010010011001101010101011001100100110100110001010110010101010101010001",
"01100010010010110101011001101010011000010110011001010101011110100100001000110001010011000110111001101110010001110101010100110101001110010111101000110111011001010111100101110010010000010100101101011010011000110100111001001101011101010111001001010001010001100111100101101101",
"01100010010001110111001101111010011010010101101001100100010001110011000101101000010100110111011001000110010101110011000101110100001101110101001000110101011110000100010101101011011100100111001001110101011010000111001101100101010001100111000101001110011011100110111001111001",
"01100010010011010110101101000101001110010100001001010010010001000110000101111010010010100111000001000011011110010111000101101101011100000110010001101110011011110110001001101111011001100100010101111001011011100111001101000110011011010110001101010101010000010101001000111000",
"01100010010110000110100101010100011000110110011101001101010110000110000101010100011011100111000001101101011001110011001001010000010000010111010101110111001110010101000001010111001110000110111101100001010000010101100000110011011010100011010000110111010101110111000001010000",
"01100010011000100101011101000100011101110101010101111000001100110011010101010111010100010101001001110100001101100111000001000001010101100111001101100010011101100110100101011001011100110011011001001110010010110100011001011000001101010101011001100011001100100111011001100010",
"01100010010001110110100101001100001100100110001001100110010010000111101001001000001100010111000101000100001101000101000101011010010100100100101101110011010110100100011101101010001101110110011101000010011101010110001001000110011010010100010101111000011100110100110001010011",
"01100010010100100011000101101111010101000101101001011001011100100111010001010000001101010111001101110111001101100110100101001011011101110011001101111000010110000111010001110011001110000101010001011010011011100011001101001011010110010110000101000110011010010111011001001010",
"01100010011000010111010101110010011000010101011101110111011011010101000101000110010011000110010100110101011000100011010101110111001101110100100001000100001101000101001000110010010101100101000000111000011011100111101000110100011101100111010001101000011010100111010001010010",
"01100010010100010011010001101010001101000110011101001101010010000101011100110110001110010011010001001101010011000101000101110101010100100111101001111010011011110110111101010111010001000111010101001110001110010100000101010111011000110111010001100011010100110110010001100101",
"01100010010100110100010001101000011110010100111001000111010001100101001101000101010100100110010001001000010100010111100001011000010110100100010001011001010101100011001100111000010011100011010101001101010101100101000001011010001101110100011000110100011000110110001100111000",
"01100010010101110111000100110110001101000101010001000111011101010101100101100010011010100111010100110010001101100111010101010010011010110101001101110110011010000111101001010000010101000111001101110010011011100100010101110111010101110100010101110000010011010110110101010100",
"01100010010101010111100001001010010110000110101000110100010000100111100001010001010110010011100101100101010100010100101001000101011110010100011001101011010110010100010001110111010001010110011001001010001100110110111101110010011001100111010001001010011011100100011101000001",
"01100010010010100110111101000111001110000011011001110100010011010101001101000110011101000011011101110101010011000100110001100001010010100011000101101101011100000100001001101011010100110110011001010101011100110011001001010000011101100111001101000010010011000110010001100100",
"01100010010100100101011001111001010011000111100001110100011001010100110101010111011001100110011101110010011010010101100101110110011010010111100101110100001101110110101001110100011010100111001001101110010100010011010001000001001110010101010001110000010011100101101000111001",
"01100010010100110100000101000010011001000101100101111001010000010100110000110101011110100011100001000011010011010100100001110001011011100101100001011001001101110101100101000100010100100011011000110010010011010101001001100010011010000111001000110010010011010111100000110100",
"01100010011000110011001101101010010001010111101001110000010010110101000000110010011011100110010100110111011000010100110101001011011100000110101101110001011001100110011101000011011010010110001101001011011010110011011100111000010001110111010001000001001101010011010100111000",
"01100010010101110101010101111000010100010110110101100110011000010100110001001101001110010110100101110111011000110100111001101000010101000101000101110011001110010110010101100111010101000100111001111001010000110100001001010100011101100111000101010011010110000111001001110110",
"01100010010100100101101001100011011000110110101101110100011110100100011001101010001100110011011100110101010010100011100100110111011010010110011101001011010101000100010000110110011100110110001001110110011100010011000101010010010000110011010001010111010001010111101001100001",
"01100010010100000111001000110011010101110101010100110100010011000011011101101000011001100110110101010101010101000011011001101011001101110110010100110110010100010100110001100111011100010011100001110100001101110110111101000100011010000111101001100110010110010100100001110100",
"01100010010110100101010101010100011011110111001001101001010101010111000001000110001110010100110001000011010011100100011001100100010000100101010000110111001101000111001001100111011100110100011100110110011101000111100001101000010011100111101001010000010010100100010000110011",
"01100010010001000100011001100101001101000011011001100010011100110011001001000110010010110100001101100101010101010101011101001100010011100100101001000001010000100111101001011000011010010011100001100100010110010011000101101001010011010111001001011001010110000110010000110110",
"01100010010110000111100001110011001100010111001001010101011110100101011000110100011001000101100001111000010001000100001100110110010001000110111100110100011011100100000101110001011011010111100101100011001101010110010101110010010110100110011101001011010100010110011100110110",
"01100010010010100110100101100101010101000111011100110001010010000011100001001000010110010111001001001101010000110100100001100100001110010110101001010010010100100110011101101000011001110101101001101010001101010110100000110001010101110100010000110011011101110110011101010111",
"01100010010101110111010001001000010100100110111001100101010101110111100001010101010100110110100101010001011100100100011001110111011001110011001101110111011010100110100101010100011101010101100001001101010001100100010001101011010110100110100001000101010001110100110001000001",
"01100010010101000110001101100011001110000011100101001000010010000101010101101110010100010100000101110100011010110100111001110011010110000100011001111001010101100011001001010111010001110101001001101011011000110111010001110001010001100100011001010000011000010100001001010010",
"01100010010010000101010001110001010001010100101101000101010011100111001001110111001100010100010101001110010110100110101101100111010110100011001001111000011010010110100001011010011110000110010000110110011101000101000001100101010011000111101001010100010100100111011001001101",
"01100010010011100011010101111010010011000011100101110101010011000011000101000111010001000110100001000001010010110111010101011000011010110111000100111000011010010011010101010001001100010011011001010001001110000101100001011010010001100111000101101111011010100111010001110110",
"01100010010010000011010001100010010100110101010101111010011011100011100101000110010000110011010001101101010001100101011101100011011110000110011101101101011011110110001101110001011001110101001001011010010110010110101101010100011001100101011101100011010100010111011101100001",
"01100010010101000101001001000011011110000111011101110101001110000110100101111000010101100111010001110110010000010101000101111000010010110011011100110100011011010100101000111000010100100110011101011000001100100110010001100100010101110111010001110111011011100101010001000010",
"01100010010010000100101101000011010011000100011001011000010010110101100001100101010001100011010101010100011001000111100000110111001101100110001000110001011110000100011001000100001100010111001101101010010001010111011001010000010101010100110101000100011011100101011100110001",
"01100010010010100011000101101001010100000111001101100110011101100101001101101001011001010011001001010110010000110101100101010000001100110100010000110100011100110101001001101010001101010101001001010110010011010101010100110100011001010011011001100010010100100111100101010000",
"01100010010011100100010001110111001100110011010001110100001110000110001001000001010100110011100101101001011110000011011001100100010011100011100101000010011011100110111101010000011001110110101000110011010010000110001100110100001101000100110000110110011100010110110101101101",
"01100010010001000011011101100101011101000111001101010100010010110111000001110010011101100111101000110001011000100111100101100001010011100101001001110001010101100110101101101001010010100110100001000111010110100100011101001010011101010110001001110111011010010100000101011010",
"01100010010100010110011101100110010101100111010101011001001101100101011001100010001101110110010001001010010101010101001001010100010100110100110101001101010001110110001101010111011001100110111100110010011110000101001101111001011101010110010101110001010000010011100101001101",
"01100010010100110111000001010110010101000101000001110110011000110100111001100010010001110101011001010010010011100110010000111001010011010100011001001011011101110100010100110010010000110110100000110100010100100011011101010100001101110011001001001000010100100110100001110001",
"01100010010100100110011101101111011100110011010000110101011011100100000101010010010010100110010100110011011100010101010001000100001101110100011001101000011110000110000101000100010001110110001101001110010010110110101001001110011100000100101101010010010011000011000101110111",
"01100010011000100110001001110101010101100100010101011001001101110100001101101001010101100111000101111000010010110100100001100010001100100101000001010001011100100011010001000010011000100100111001010000001101000101100000110111010101110100010101001000011010000110110101110101",
"01100010010100000100111000111001010001110101001101101111010101010100000101010010011001000111011001000111011100010101011101001101011100110111011001000101010001000011010000110110011011010011011101000100011011110111000001100111010100110100100000110010010010110110000100110010",
"01100010010110010111001001101000011011100101100101011000010100110111100101110000010100100111100001010001011011010100110101110101011101000101010101110001010101010011000101011000010001110111001101110001011101010101011001001100011011100011001101101010001101110110001101110100",
"01100010010101100111100001111000010000110011011101000110011011110101001101000001011010000100010101000111011000010100110001010010010010110101010001100001010000100100000101000110010001010011010001000011011011010100011001110010011001110110000101110001011110000111011001100011",
"01100010010110010111010001010101011100100111011101000101010001000100101000110010011001000100010101101101010001100011011001000101001101110110010101110111001100100110001101100010011010000011100101111001011001000100101001001000010101100011100001001100001100010110010001010001",
"01100010010010100101010001001010011010000110101001111001010100000110001101010000011100000110011000110100010000100101000001000010010001010101011001000101010100000110011001000111010010000011011101110010011000110100101101001011010010000100000101110110011000010101010101111000",
"01100010010101110011001100110101010001110101100101000001001100110011010001100001010101010111100001100100010010100101011001111000010001000110100001000100010001110110010101001011011101100101010001111000010011000110010001000110010110010111010001100101001101100011011101100111",
"01100010010010000110010101101010001101110100010101010111010100000110100001010101011010010101010101010111011101100011100000111000011001000111101001110000010010100111001101010101011011110100110101010010010010000100001001000011010011010101100001101001011011100011100101010010",
"01100010010101000101011101000100010100110100001001101111001101000100111001110101011011110110010101101011001110000101000000111000011101110111100100111000010100110100010001100010010110010011000101111001010110100111001101010010010110000110100001001010001101110110101101010011",
"01100010011000010111001001010110010110010111011101101011011001110110111001011001001100010011100001110001011101010100110001000010001101000111101000110001011110000110100001010110010010100100000101010100011011110101010001110011001101010100011101010000011001110111010101011010",
"01100010010100010100101101011000001101010110101101010100010101000111010101100011010101100101011001000110011101100111000101110100001100100101100101101110010001110011001000110001011000010111100001111001010110010110010000110010011000110100010101110000001101110100110001100100",
"01100010010011000110011000110110010011100110100001101010010100100011010001110010010101110100010101000010010100110100101001010101001101000110010101110011011011010101000000110111001101110110111000110111011100000110010001110011011100000100101001110111011110000111001101011000",
"01100010010001010100010101100001001101110111001000111001011100000111100001110100010011100110100101010011011001100100010001010011001101000110011001101010010010000011100101100011010011010111001001010100011011010100011101100011011011110110100001101000010011100110100001100101",
"01100010010010100110111001010010010100010100110101011010011000110111001101100110010110100100001001100010001100010110111001010010001101010111000101000110011101000111100001101000011000110110110101001011010001110100010001100110010011100110010001010001010001010100011001111001",
"01100010010101010011001101100001011000010011011001101010010100010111011001110000010101100100111001101001010110000110101001101000010010000011100101001110010011100101001101010010010010110100011101101011001101100101000100111001010000110111010000110111011110100011011001001000",
"01100010010000110111100001101110011110010111011001101011011100100110100001111000011010010110000100110011010010110111101001000001011011100100001101100101010001110100100001101001010011000111100001100111011100100100010101101110001100010110001001101001010100010100110101110011",
"01100010011000010110110101001101011100110111100001111000010100000101001101011000010000100100010001011010011101010011100001000101010001010011000101001100001110000101001001011001011101010011001101010110011000100110011100110100010011000100101100110101010110100110100101001010",
"01100010010100010110001001110111011011010111000001100010010001000110100000111000011011110100000101110100011100100110011101101101010110010101010001100111010101100111100100110101010010000110110101010010010100010111010001100011010100010111010100110110011001110110000101000001",
"01100010010011010111000001001000001101010101011101101011011010110100000100111000011001000100010001010111011011010101010001101111011000010100011001001010011000110100101001011010010001000110101001010010011010110011001101111001001101000111101001001110011000110100100001110010",
"01100010010011010110101001001010011100100110110100110001011001100101000000110110001100010011011101111010010011010100001001110110011100110111011000111000011100010100110101001100010010110110001101001000010100000111100001001011010101100111011101110001011100100101100001101101",
"01100010010100010100011101110000010010100101010101101110010101000110010001111000010001110110100001100001010110010110011001110111011010110101101001110010010000100100101001000100010100000110100101010100011001100100100001001011010100000100010101101010010000100111011101010110",
"01100010010101010101100101101101010000010111010001000110010001100101011001011000011100100011001101110100001101000111000001010111011010010011011001110011010110100111010001100001011110010111000101001110010110010100100001101010011010110101000001010001011100100110011101000101",
"01100010010001110011000101001101011010000101000001111010010011100101011001101101010001000101001001010100001110010100110001010011010100010101100101101001001110000100010001010001011110100110100100111000011101110100000101101001001101110011100001010011001110010111001001111010",
"01100010010010100111001001010100010101000110100001001011010011010111010001000001010101010101100101110110010101000100110001101001010011100100101101001010011010110110010101001000011000110100001001010001010100000011100001010101010010110011100101001101011010100011011101101011",
"01100010010011010111011101110010011110000011010001110010001110010111001000111001010011010111011101110000010100110110000100111000011001010011011001110101010011010101011101101011011001100110001101010100011011110110001001110100010000110111010101101101011000010110100101010000",
"01100010011000010101010100110011010110000101001101010100001101110100010101001000010110000011100001101001010101000100100001100001011001110011011101010111010110010011011101010111010100000111010100110100011100000011011101100001011001010111010101110010011101110100101101100011",
"01100010010010110011000101011000010100110110001001010011010110010111011100110001010110100101001101101101010001000110101001001010010000100111100101100111010001000011001001100011010010000101011101111010010100110111101001011010011101000011010001101010010010100101100001110011",
"01100010010101110100010001011010010001010011000101010110010101100111010101101011010011100100111000110111010010100111001001001010011010010111100001110000010011100100011000110101010101000111001100111000010110100110000101001110011010100011011101011010010110100101001001110100",
"01100010010001100110110101001011011010100110000101000010001101110101011100111000011100000011001001110011011100100100010100110001001100010101101001010011010100110111101001000001011010100101000100110101011101100100111001101010011010000111000001100010010011010111100101101111",
"01100010010001010110011101110110010101100100101101011010011011010110101101100100010101010101000001101101001100010110101101000110001110000100110101010110010011100110010101101101001100010100110101010101010100110111000000111001011100010011011001000110011100010111011100110111",
"01100010010100010011100001110110011101000100010001110000011010010011010001010011001110010111001001001100011001010101001001000010010000110110111101000011010000100110101100111001011001000011100000110011011001110111010001001110011000010110000100110101010010110110010001101110",
"01100010010001010111000101001101010100110100110001010101010001100100111001110011001101110100001001000111010011100100001101001110011011110111010101110111011010000011001001000111010100010100011101110010011000110110001100111000010101110101001101110001011010010011010101001011",
"01100010010001010011001101110011010010110110011101100110010001010100111001011001011101000101100001110111010001010011011001001100001110010110001001011000011001100101011101000011011101110100000101010001011101000110001000111001011000110011010101110100011010110100011101010100",
"01100010010001100110010100111000010000100100001001110001010001010101100101100100011110100111100001101010010000110011011000110001001100100011010001000011010000100110011101000101011101100110111001010010011001010101010001100101011000010101001001100111010011000110000101001011",
"01100010010100100100010101110101010000010011010101110011011010000011011001011001010011010110101000110100011010100100011001111010011101010011011001111001011010110111000001100011010001000100110001010110010001110011001101001100011010010111001000110110001100100101101000110011",
"01100010010101000111011001001100010110100100010000110101010110100101011001001100011001110111011001100011011110010100110101000111011011010110101001000010011010000111100101110101001101000110100101100111011010000111000101001011010011000101001000110111011100000101000101010100",
"01100010010011100100101101100101011101000110011100111000011110000100011101011010011010000111010000111000001110000011100001001110011101110110100101000001010100100011001001100011010010100100001001010010011000010100110101110001001101010111000001001110011101000101010101110010",
"01100010010100110110010101110111010100100100010001110000010010000100011001000001011110100101001101110011010011010100001101110110011101000110111101100110010110000101100001001110011001100111011001010101011100110111010001010110011110100110011101000111011010000011001101110010",
"01100010010011010111000101000111010000100100010001010100011011110111011101010001001100100110110101101010010011010101011001000101011100100100000101010000010101100110111101010011001110010111001101101011001101010101011101101011010100010111000101100001011010000011100100110010",
"01100010010100100111001101100101010101110110110100111000011101000110111000110111001101010111000001100011010001010101010101110001011110000011100001001100010001110011100001101001010100000100011001001110010101110100010001010101010011100101011000110111011000110101011100111000",
"01100010010110010110100101010100010010000111011001110011010011000011001101001010011001110101000000110011011010000111011101101111010110100100000101100010010010000110101000110101011101000011000101101001010100110011010101111000011110000111011001011010001110010100010101101011",
"01100010010110000110100001100101010000010110100101010011010110000111010001100011010101010100011101000111010110100101010001000101010100100011010101110001001100110101011001110100010101100110000101100111010000010100110001000100010101010100101001010000001101110100010101100011",
"01100010010100000111001001110010010110100100110101000110011101100101001101010000001100100101101001100010001101110101011000110001001101100011001001111001011010100111010101001000011110010100011101010010010100100111001000111001011011110100010101001010010001100110111001000100",
"01100010010100110110000101111010011001110110000101010110010100100111001001110111011000010111000101010100011101110110010101010010001100110101000101000001011011110100010000111001010000100011000101011000001101100101101001010110001101110111010101000010011110000011100101010101",
"01100010010010000101100000110011010000100101010001100001010100000101010001110101011100000101000101001010011001010110010001000100010100100100101001101000010101100101100001111000010100100101010001100111010101110111000001010011010001000101011101111001001110010100111001010100",
"01100010010001010100110001001010010000010101101001100010011010000011010001001000010101010110100001110001011000010011011001110001011011100100011001011010011101010110110100110110010010000011011001010100001101110110011001011010011110100111101001010001010100100100011001010000",
"01100010011000010110011101011000011100000110111001000110010000110100011101000110011101110111001001101101001110010110010101001101011110010110010001010111010100000100001001000100001110000110001000111000010101110110001101000100001101010011010101010101010110000111100100110011",
"01100010010011000110001001010011001100010111001101000111010011010100000101110101010011100011001001000110011001100111010101010011001100010110101101100010010100000110001101110101010100100100110101110111010100100101001101000011010011100011010101110100010000110111001001101101",
"01100010011000100100011001011000011101100100110101010010010001010110001101010010010100100011100001010101011001000011100001011000011011010110001001101011010110010110100101000001010110100011010101010011001101100100101001101001010101110100010001010010011000100011001001001010",
"01100010011000100110011101001011010110000011000100110111010010100101001001101000010101010110110101101111011100000111101000111001001101010111011001110010011001000110011101001011011101010100100001010100010010000110100100111001011100000011100101101000011001110110111101100111",
"01100010010101000111011101001010011101110011010001011000001101110110010001010000011001110101101001100011010001010111001101111010011100010110011001010101010100100011011001110110001101100011010001001101011001010101101001110000011010010100011001110100001101000101010101100110",
"01100010010100100100110101011001001101010101010001011000001101000110001001101001010001010100100001000001011101000111100001100100010001000111101001110011010101010100110101010000010100110101010101111000001100010100010001111000010001000110001001100111011001010011001101000001",
"01100010010100010110011101110111010101010111001001001011011000110110010001110001011101110101000000111000011001010101010001010001001110010110101101101001010000110110110101111010011110000101011000111000011011110110100101100110010001110110000101110100011100110100110001100001",
"01100010010100100111000001110100010101100111011101000110010100110110011101010000011010000101100000110110010101110100000101110111011010010111001101001010011010000110011001100011010100010110111001001100001110000100100000110001010000100110011100110101011000010111011100111000",
"01100010010110000100101001010100010110100101100001111010011100000011010101100110010101110101100001011000010101010011100100111001010110010101011101100101011001100110011100111000010001110111010101001101001101010101001101001010001110010110000101111001011100110011010001011001",
"01100010011000100100111001000100010110000100001101000010001101000110000101101001011011010100010101000110001100010101001101000111001100010101000101110001011101010100101000110110011110100110111101001000010110100111010001001011011011010110001101101001011000010100011101101111",
"01100010010011010110101001100100001100010101010101010110010001110011000101100100010000010100011000110100001101000110111001001110011101010110111001010101010010100011001001001011010100110110101100110001011010110100110001010111011010000110010101100010011001100101000000111000",
"01100010010011000011100100110101010000010011010101000101011110100011001101110000011101100100100001101001010001110110011000110110011011100011100101001100011010000101010101101010010011100110011101000001010100000011010101100001011011110110100001000100010100100011010001111001",
"01100010010110000111001101101111001100110100011001101000001110000011010001100110011010010011000101101000001100010110101000110101001101010111011001010011001100100111000101000001010000010100001000110010001101110100000101000100010010110111001101010011011011010101010001100101",
"01100010010101110101000001000011010000010100101101000111011001110110100001011000010101100111000101100001011100100111011001101000011001000011001101001100001110010111001101010101010011000101100101101111010001010101101000110101010101100101010001111001001100010101001001110000",
"01100010010001010111010001001101010010100111000100111000010101100100110101111000010001000011001100110011001100100111001101000100011100010100001101110000010000100111100000111000001100010100011001010100010011010111001101010110011010000101100001010001011000010100100001001101",
"01100010010100000110011001101101001100010111100101001000010101110101011101101011011011010100011101010100011110100111010101010000011010010110100001100010010010000011010101001101010001000100101001010100011001010100001001110001010001110101001100110100011001100101100101011000",
"01100010010110000110100101100001010101010110010101111010010100110110100001110101011001110100100001010111010100110100011001000101010010110110011101101110010011000100110001110110011100000101010001100010010001000101100101000100010010000110110101100100011001110100011101011000",
"01100010010100100101001101010001011010110011000101101111010010000011010001101010011100000110111000110110011011110100111001001100011000100111010101001000011110010110010001100100011010000100011001110001011000110111100001000111001100100111100101010100010101000111001101001110",
"01100010010110010100010101011001010010100100111001100100010001010011001001110110011000100111001100111000011011110110110101110011011000100011010001110000010101010111011101111000011100110100011001101101011011010100000101000111011011010111010000110101011010110101011001000101",
"01100010010101000101100001000101011100000011010000110011001101000101100101010001010001000101001101100001010110100101000101110011001101000100001101010100010000100110011001000010011110010101011101010001011101100100000101111010011000100011100101010001011011110011010001101010",
"01100010010110000100100000110101011100110110001001010010010101110101010001101001001101000100010101100001001101110110101101001110010101110101100001010001011001010101001001101101001100110100100001101111010100100110101001000010010100100101001101011010011010010100101001001110",
"01100010010101100100000101010111011010100111001101000100001101110011100101000100010011100011100101011001010110100100011001010011010100000110010101110111011001100110111101101001010100000111001101101010001100100110010100110010010010100101010001010101001101100011010001000100",
"01100010010011010111010001110000011010100101000001100011011011010100010101101110010011100111011101101101010110100111100001010100011011100100011001001101001100100110011001100101010001000100000101011001001100110011011001111010011110000101000101100001001101000100110101000010",
"01100010010110100101101001000100011100100110011001010010010011000011001001010011010001000011010101000101011101000100110001001101011000100011010101100111010100100110000101000110011010000100111001001010010101000101011001001011001101110111001101000101011001100111000001011001",
"01100010010011000101000101010111011011100111000001100011010011000101010001110011011011010111100101001101011001100100011001110011001101100011100001000100011100110100101000110101010001110100001001010110011011100011010001110100010001000110000101001000010101000100100001001101",
"01100010010011000110110101011001010110010011011001100001010011010101100101011010010000010011100001101101010001100110100001101101010001100111011001000101011011010011000101100010011011100101011101101000011110000011000101010101011110100110000101010101011100100100101001111010",
"01100010010110100011000101001011011100000011100001001010011010110100101001111000011010000110111101100011011101000100010001000100011110000011001101001011010110010100001101001100010001010100011100111000010101110101001001110101010001010110011001100110010100010101011000111001",
"01100010010110010100010000110111011100010011001101001101010000100100000101101000011110000101101001010100010100000011010001010001001101000100101101100110011110000110010001010101010010100110101001101110010010110100010001100101010101100110111001011000001110010110010001100001",
"01100010010101110110001101010000011110010110010101010010010101000110100101111010011011110100101001110011001110010111000101101011001101010100101001000101011011010011010101110001001100100110101001010111001100110100010100110010010001100100011001000101010100110011001101000111",
"01100010010011010101100101000100010100110110010001000100001100010101010101110011011100000110001101001011001101000101010001100110010101100011001000111001001100110111000100110011010010110111010101111010010000110111011001101110011001010011000100111000011001010111101001000110",
"01100010010010110101100001110110011011110111010101110101010001010011011101111010001101010100001001100010011011010011001001100101011101110100011001001000010000100011001101001000010101000110010101101010011100110100100001010001011001110011001101100101010101110100001001111001",
"01100010010101110100011100110100011100000111000001000001010101010100110101101001010001000111101001110001010100110110000101001100001100110100001001001011010001010110011001001011010100000111010101010111010011000100001001001110010110010110101001101110010000010111001100110010",
"01100010010101110101011001000010010010100111100001001000010001100101001001000111011100100110101000111000011000010100111001000001001100100100110001000100011011010111001101010100010011100110001001010100011000010100000100110100010100000101010001010101011010110011011101001011",
"01100010010001110100010001001011010101110110100100110010010001110110001001001010010100000101100001000110001100010101010001001000011010110100011001001000011001110011100101000100011010110111100101000100010010100110101101100111011001100110101001011001010101000011100001010011",
"01100010010101100111100000111000010000110101010001010000001101000110010001101011010001010011000101000111011100000100011101010101001100110110111100110010011101000100010000110110010101010101101001010001010101010111010101100100001110000100110001001010011100000100101101111010",
"01100010010001100101101001000111010100010101100101010100011010100101100001000110011010110110001101000110010001100110010100110001001101100101000001110110011010100110111001110001010101100111100001011001011010100101100101110011011011100101011101001010010110000100001101100011",
"01100010010110010110111101010010011110010111101001101111010110000011100001110000011100110110000101110101011010010111101000111001010011100101001101001110010011010100000100110001010010100101100101100111011100000110010001011001001101010101101001111010010010110011010001101111",
"01100010010100000110111000110101011001000110111001000011011011100101000001101110011000010110111001000110010011010111101001111001001100110100111001010110001100100111000000110101011001110111101000110010010011010101011001111001011101010101001000111000001110010101010101100010",
"01100010010011000101000101010011010011000011001001000011011001010111100001001010010101000011011101100010011011100111010101000111010000110100001001100111011101110100101001100110011101010100110101001010011000010101101000110111011100010111001001110111010110010100011000110001",
"01100010010001000101001101010011011100100100100000110100001101010110011001001000010001100101001001110010010000110100111001111000011011010110100001001010010101000100011001100101010011010111001101000001001100010100110001000110001101110101100001101000010010110100001001000001",
"01100010010010000100001100110010010110010101100101100111011010100011010101100100010110100100001101100111010100010101000101110011010010000111001101000111011100100111001000110011010011000100110001100010010010000101001001000100011011010101000001011000011011010011001101010111",
"01100010010101100100000100110101011100110110111001001110011100010111001001100110011001010110101100110010001101100100000100110011011101110111011100110001011000010100111001110000010100010111011000110100010110000100011101110101011010110111001101110101001110000111010001000010",
"01100010010011100100110101100111010001000110011101101000001100110100000101010010010001000111010101000101010101010101000101000101010001010111000000110111011001110100010001101111010110100101001001000110010010000110010101011010001101000100001101001010010100000110110101010110",
"01100010010100110111001001110100011000010011001001001010001101110111000001101010001101000110111001000011011001010101000001111001010101110011100101100010001100110111001001010100011100110100110001000100011101100110110100110100011010010100110101101101011100100100000101100011",
"01100010010010100100001101010001010010100110001001000010011001000011011101000100010100110110001101111000001101010101011101110000010010110110000101010000011101100011000101001110011110010101100101001000011011110110011101010111010101110100001101001101010000110101010000110101",
"01100010011000100110010100110111001101110101100101100010011001110110111001100100010100110100110101011010010011100100011001010001010001010111100001111000001100100110011001010101011101100100101001000010010101010111100101000111001100100101100101000100010010110100111000110110",
"01100010010001010100100001001100001101110111010001010110001101100111100101000010011100000110000101010100010100000011010101000111011001110111010001110001010001110110000101110100010001010011001101101010010110010101001100110001010110010111011101000011010000110011100101101001",
"01100010010101000110100001111001011100000110100001100110010001010100111001100101011110000110111001101011011010000111100101010010010001010101011101000001010000110110111001001011010100100101101001101001010110100110000101001101001100010110010001100001010001010110110101110010",
"01100010010001100011100101110011011101000011100001111000010100100101000001100110010000100101001101000110011011100110101001001100010101110101010100110101011001000101100001001010011010110101001101011001001100110100110001100101011100000110101001100101011101110110101001100001",
"01100010010101010100101101100100011001000110010001000111011101110110110101000011011000010110111001110101001101110101000101101011011101100100010101101011011001000111000101011010011100100110101101010010011100010110010001010001011110000101100101000100001100100011011100110001",
"01100010010001110101101001011000011000110011010101010010011000010100101101110011011110000111100101000001010100100111011001100100010000110101001001110100001110000101011101100011011100110011000101010011010101100100100001111010011010000011100101100001011001110111000100110100",
"01100010010100000110011001100110001100100100100000110110001101010111100101000011001101110100011001110101011100100110000101110100011010000100010101001000001101100110110101110100010000110101010101101000011011010111000100110111011100000011100001100001001101110100101001101110",
"01100010010001000100110001101000010110000110101000110111010011000111101001010011010110100110111101101001010010100011100101000101001100010100101001010000011110000110100001001010011100010110100000110100001100110101100001110100010101100100001000110111011000110111000101010100",
"01100010010101110011000101111001010011100100011100110110010110010100101001000101011000110011001001010110011011110100010001101011011001100100101100110010010100110110001001111000010000110011011101010000010110100100011101000001011010010111000001010110001100110111010100110010",
"01100010010101000100010101010001010110100111000101011010011100010111011101000001010011100110011001000001001110010101010101010101001110010111000001010100010100000100100001001100010010100111000100110101010011100110010001001000010101100110010101101111010101000100110001110111",
"01100010010010100110010001110111010101100100001101011000011001100100111001110010011010000011001101000001010101110011010101110101001101110101100000110111010110100111001101001100001101010101010001011010011001000011010100111001010011100101011101010011010100100011000101010111",
"01100010010011010101011001000111011011010011011001010110001100100100111001110000011010100100010001100010010110010111011101010101011000110110010101110000011110100110111101011000011001000111101001001011001100010011001101010100010010000110100001010011010101100011100101000010",
"01100010010011000101001001110010001100100100110001110101010000100100010001111001010011000111011101111001011000010101010001100110011010100101000101101001010110100101000001001101001110000110000101010110010001000100011001110011011100000101101001000111011010000101011100110001",
"01100010010011010110101001101001011000010101010001110100011001000011010001101011001110010111001001000011011010100111000101110001011000100011000101110010011001010011011101010100011011110100000100111001011101110101000001100110011010100111100000111000011010010111000101010010",
"01100010010011000110100101001100011001100100100001111010001110000011011001100101010100000011001001001011011001000111001001010011010110100100010101010110011011110101011001000110011100100011000101011000001101010100100000110011011000010110101000110101010011000100011101101011",
"01100010010100100110100001100010011110000111010101110000011100100101100100111001001100110110010101000010010110000101100001010100011101100111100101001100011011010100010101001010011101110100011101100111010001010011100001101001001101100011100101010100010011000111010101011000",
"01100010010001000101011101001011011010010110010001101010011001110101011001010110010010000011010001010101011000100101100001000110011000110100000100110111010000110111100001000100011000110110011101111000001100010110011101011000001101100110111001100001011000110101010101111010",
"01100010010000110111010001011010011100100101010101000101001100110100000101010100010010110011010101110111010010100100100001110010001100110101001001010101010110010101100100110101001101000011010000111000001101000101001101100111011100010110101101011001010100010110101001110101",
"01100010010011010110010101001011001101000101010001100111011100110100111001010100010011010111011100110111011010000110111101101010011000100100110001100011011101010100100001010110010001010111001001110100001110000111100001000110010110100100010001100010011110000011010101100100",
"01100010010110000110111001110010011000100011010001111000001101000110010101000111011110000110001101001000010110000100101001110011011000110100111001101010011001110101000001010001011110000110111001110011010101010101001001111000011001010100111000110111011001100111100101110001",
"01100010010100110100001101110101010011100100000100110101011000100011001100110101010110000111001001010010010011000101000101100100001100010111011001110010011100000101000001000001010100110111001100111000011001100111000101000001010101100101001101001101010100000110100001000110",
"01100010010010110110011001001100011101100011010101101111010011010011010101001010001101110100110101110000011001100110011001001000011000110110001100110010010010110110011101011010010100110101001101011001001110010011010100110110011000110110010001010011010101110111010001111001",
"01100010010001100111101001101000001101110110001001101111001100100101001000110100001101010101011001110001010000100100001100111001010110100101100101010000010100010110011101100111001101100011001001110110001110010100001101010001011001110111100101001010010010100011001101010101",
"01100010010110100100100001010010010010100100101001100010010110000111101001001000001101000101010101100100010110100100101101101111011000100100101001100111010011100101100000111000001100010100000101101000011001100110100101010001010000110111100001100101010110100111010000110010",
"01100010010100110100101101001011010001110100110100110001010100110101001101010101011011110111010000110001011011010011011101110111010001000111101001110001010101000100000101000101011110000100101101010101011101010100110101000100001101010100100001011001010000100110010101010010",
"01100010010101010100001101011010011001110011010001011001011010010111100101010100010100110110001000111001010100010101000001100011010101010111100100110011011101110111011001010010001101100110111001010000010100000111000101010111010101010101001101101010011100110101010001000011",
"01100010010110100100001101100010001101110100001001100110010110000111000101100110010110100111010001001010010001110101100000111001010101100100111000111000010110100110011001000110010011010101011101110010010001100110011101110011011000100100110001010101010101100011011101111010",
"01100010010010000110010101011010011100000111100001111010010101000111001101010001010010100101010101110100010000100100011101100001001101110110100000110111011010000101000100110010010001000111100001111000011000010100001101101010001101010110000101000111011001000110111101000011",
"01100010010011010100011001101011011001100101001000110100001100010111011101010000001100110110001101010110011101000100110001011000011011010110010101101101011010000100001001110110011100110110100001011001011011010100001101001110011011010101101001100110010100110111000001101101",
"01100010010011100110011101101001010100010100110101000111010000010110100100110110010001010101010101001101011000100100011001000111011001000101010100110100011001010110010101110111010010000110010101110011001100010111000000110110011010100101010101010000011011100101011001110000",
"01100010010001010110111001101101010100110101011001010010011010000111011101101101011100000100100001100110001110010111100101000111011010010110111101110010010100000100001000110110011101010110010001110100010110010100011001010111001110000011001000110100010010110111100101100011",
"01100010010010000011010101011000001100100110111101010011001110000101010100111000010110100100111001010010011101110111100001100001011100110110010101000111011101100100100001010011011011110101010101101000010110100101001001100011010011100110111101010010011110100101000101100010",
"01100010010001110110111001111001011010000100010001110000010101110110001100110101011110100111000101011010010010110101011001010100011011100111001101111001010100010011100001000001011010000011011000110011010001000101010000110001011101010110001001101000011110010111000001110100",
"01100010010100100110001101000010010010100100110101001011010001000110000101100011011100010011011101110110010010110101011001001100010100100110000100110110010101000110010101110110011011100011001001001110011011110011100001110011011101010110110101110100010001000111010001111001",
"01100010010001100101000001110001011000110110100001000010010101010100010100110100001100100111011101101000010001110110101001000010001101110100110101000010001101000100000101100101010001110111010000111001010000010110001101110011010110000110001101100101010101100101100101000010",
"01100010010100100101101000111000010101000111010101011001001101010100100001000110010100000111100101111010011010010110010101110011001101100100110100110111010100100110000101010001011101000110100001010111010110010101000101110111001101110100101000110111001101000110011101110100",
"01100010010100010110100101100010010100100101100101010011011010100100111001100011011010100101000101110110011101000111100001001100001101000100010101110111001100100100111001110011010100110101001001110010010100110111011001101110011101110011011101101111010100010111001001010100",
"01100010010010110100110001101010011010000101011001010100011001010011011001000100011010000110010000110001011101010100001100110111011010000100101101000010011100110101000101000010011100010100100001110000001110000110111101111010010100010111001001001011010011010111000101010111",
"01100010010011010011100001010010010110010111100000110011001100110110010001000100010000100101001101110010010001110101100001101010010110100101000000110111010010110110110101001011011100100011001001010010001110000100101000110010011000100101011001111000010101100110001001010010",
"01100010010100000100101001010110010101100101101001001000001101100011000101010110001100110100001000111000010001110111100101010111011010000101100000110101001101000110111001110100011010010011100100110010010001100111010101101011010000110100110001100101011001000011100101010100",
"01100010010001100101101000110101011011010100001000111000011010010011010101000110010001110111000000111000010010000101011001110111011110010011011101110000010110100110000101000001010101100110011101110001011001010110110101010011010010100100110001101111011100110110101100111001",
"01100010010001000100100001110001011110010011100100110100011010000111011001110101010101010110010101100011011101100110010101001011010001000100000100111001011001010101001100110100010100000101001001101000011101110110010101101101010101000110111101010111010100010011010001010010",
"01100010010001100111000001011000010000100111010101100100011001110100100001010100011100100101011001110000011110010111001001000100011110000110100001000111001110000111011101101010010011010110100101011000011010100100000101000100001100110110011100110001011010110110100001101111",
"01100010010010100111001101101011010110010101001100110100011100110111100001111000010100010100010001110000010100110101101001101101010000100100111000110101011000110111001001110101011010110111100001100100010001000101010000110110011000110101100001100111011001110110101101100111",
"01100010010100000100001101000010011010010111010101010010011001010110100001101001011001000111100101111001011101110101010000111000010010100110011101011010011110000011100101001101011100100110011101110001010110000101000100110111011010000111010001001000011100000011100100110111",
"01100010010100000100010101110011001110000100100000110111010110010100011101100111011001100110111001111000011100110101001001000001011011110100001101110110011100000111001101110000010000010111001001101111011011010100100000111001011110010101100101101111011001010100110001111010",
"01100010010011100011011001011010010100100101000101111000001110010110011101010010011100000100011101000110011100110101000001011010011101110110001001010110010110100101011100111000011011010100010001110110001100110101101001111010011010000110011000111001010001110100010001100001",
"01100010010100100110100101101111010001010100001101010100001110010110011101110011001100100100010101000110011010110100010001101101011101110111000001110011010110000011001101001010010110100100011101100110001101010100001001100011001100100011001101110000011010010100100001010110",
"01100010010001110100011001010111010100110111000101001010010011000110001101101110011011010111000101000010011101010011001001100011010100000111000101111001011101000011011001110010011011100100111001001011011010110100001000110001010101010110010001101010010010000101001101001000",
"01100010010110010100101001011001011000110110011001100111010101010110010001001010010011010111010101110101011110100111011001000101011100100101100001000100011010000111010100111001011101110100000101010100010001000110111101001101011110000110100101100110011001010011001100110111",
"01100010010001010101100000110110011000110100001001101110011101110110010101110110011110010110100101000101011100010110010001100001011101010110111101010001010010000100011101100110011010110100110001001010010101010011100001001010001101110111000001101110001110010100011001010000",
"01100010010001010101001001001010011010010111001100110001011011010110011101010101010000110101000101000010010100110111011001111000010110010111100101001101010100000111011101000110010001100110010101011001001101010111000001000100001101100100100001010111011010000100000100110110",
"01100010010100100100110001010110010011000111000001000110011011110110011101110011010101100101001101101110011001110100010101100011011110000110110101000010001101000110001101001000011011100101100101101000011010110101000101001101010110100101010101101010010101000011011000110001",
"01100010010011000100110000111001001110000011100101101001001100100011011001001101011000110110011001000111001101010110001101111001010001010111011101000010010110010101101001111001011110000011000101111000001100100110001101100101001101110111000101011001011000100110110101010000",
"01100010010010110111011001110001010011000100010001010100011100000100001101001011011100010100111000110111011010000100101101010001010010000100110101101001010010110100111001100110011101010111100101110010011110000110101101110011010001110110011101100001010011100011010001101001",
"01100010010101110110110101100110010100110111001000110010001101100011100001100110010001000101011101111010001100010110010001101010010101110111001101110011010011000101011001001101011101010011010100110010001100100110000101101001011001010101010001010101010001010110011101000101",
"01100010011000010110011001010010011011100100001000110110010110100110110101101111011110000101011001101110011010010101000001101011011010000100110101000010010010000011000101110011011001110110001101010110011000110011100001101000001101010101011001101011011110000011100001000011",
"01100010010001010110100001101101010110000111010001110001011000010111010101010101011011010100111000110100001110000011001001001011011011110100000101000100001110000101000101011001010001110110010001001110011101000110110101000001011000110110010001101101011001110101000101110010",
"01100010010010000101011001000011011110100101001001001101001101110111001001000001001101010100110001010011010001110101010101100001010101010101000001011001011101100100110101010010011101100110001101000101010001000101100101011010001110000111100001100110010100010101101001110101",
"01100010010011100100011101100100011011010100000101111001010101110101001001010110010010000110010101100111010100100100000101101110010100110110100000110001011110100110101001001110011010100100110001100111010000010100110001110110010110010100011000111001011100110100110001010110",
"01100010010101000101000001010010001100010100011100110011010010110101011001001110001101100101001001001011010000100100010101100011010100000111011001010000011000110101011100110100001101100111010000110110011000110011100001010110011011100101010101000010011000110100010101010001",
"01100010010101100110000101111001010100000110111101101001011001100100010001001100010100000111010101111010011110010111011000110010001110010111000101000101010001010100100001111010010101110100011101101001011100100011100101000110011100000111001101001010010001010100101101101011",
"01100010010110100100000101000100011100000011000101001110010010100101011001010111011101010101100100110010011110010101000001101001011110010100011000111000011100100111011001101001010100010110011101101111011000100100001001110101010010000100101101101010011101010011011101011001",
"01100010010100110101000101000110001110010110011001101110011000010101101001110001001100110100100001010100010110010110111001101110001110000110101101000100010011100100001101100110001100010111010001101000001101010100000101010101011001100011100100110101011010100011011001000111",
"01100010010100000110101101110100011000010100100001010101011010000110101000110111011000110100111001000001010110000110101001000011010000110110000101101001001110000011000101110110010101100101101000110010010110000110100101111010010010110100001001111000010101100111011101110110",
"01100010010101000111001001110111001100100101001001000011010000100011001001101101010000100111001101000010010100010101001101100101011010100100110001101111011000010111001101000001010010110101010101110111011001110110110100111000011011110101000001100011011100010011000101110001",
"01100010010101000100011101100001011000010111011101111000011011010011010101001011011101100111011001000010001100100101100100111001010101000110100101010101010011010111000101000101011010110110011001100100001100010100010101001100010011000100101101001011011010100100010101110101",
"01100010010010100111011001010001010000100100101101101101011101010111010101001110010101000100110101010011011001010100110101011001011100010100010101010001010010100011001001111000001110010011011100110011001110010111010101000100010110000101011101100100010001100100010001010011",
"01100010010100100011011000110010010101000110110101000010010100000111000000110010010110010111011001010001011110100100001000111000010011010011011101010110010101010111100001011001011011010111100001000100010101010101010001001011011000100101001001000001010110100110010001111001",
"01100010010001100101100101010000011010110111001001010011001101010100000101110110011100100100001101011001011101000111100101010000011011010110111001010111001101110011001101110010010010100101001101100001001100100011001101001101011001000101000101100001010010110101000001000001",
"01100010010010100100001101110100011100000101001101110100011100000101001101000100010001100101011101010110010101010101000001010010011101000101000101000100001100110110011001101111011010100100011001000011011101010111011101010000010001010110001001101010011101100100101101110000",
"01100010010001000011000101011010010001100101100001110111001110010110000101110001010010110100001101001110010011100111101000111001010101000100111001101110011100100101000100110111001101110100011101000101011010010110111001100001011110100100011101001010011000100110010101110010",
"01100010010100010100100001110000011101010100110101110111010101000100011001111000011100000011100101111000011110010110111001111010011010010011000101110011011101100101011001101010011001100110100001111000011000100100000101000110011010000111010101010000011101110110010001010111",
"01100010010110000101010101010011011000110111100001000110011110000111010101101101010010000011011101110101010110010011010001010001010010100101011000111000001100100111100101000110010101000110000101010001011001110110111101000100011100000111011001010010011011110111100001010101",
"01100010010010100101001001000110001100010100001101110011011010010100010001000111011110010101001001011010011100100111010001010010001110000100010001001010010110100110110100110101010101010101010101100101011110010011100001001010011011010101100101110011011011010101011101010111",
"01100010010100000110010001101001010001010110001101010100001100110111001000110111010010000110111101000101010101000110001101001100011110000111010001110110011100110011011100110101011110010110111100110010010001110101010001101011001110000011010001111001011010100100001001110000",
"01100010011000100011100001000001001101100111000101101111011001010110000101010011010100110111011101000110010011000111001000110111010010000100100001000100010011010100101001111001011101010111010001000101011010000101010101011000011100110111010001010000010001010100001101101011",
"01100010010101110110010001100111011011010011001101101111011101100100101000110101010110010100110101000111010001100100110001000111010100000101000001011001010001000101011101111000010010110100101101111001010011010110100001010011011101010111010101001100010110010111011001101011",
"01100010010101010101011001010011001101110100010001011010010011010101100101100011011100100011001001101010011100000110010001000110011011110100110101000010001100010110001101011000011110010100111001111000011101100101001001110111010000010011010101100110001100100100011000110110",
"01100010011000100111011100110010011101010111101001100010011001000110111100110100010101100111100001100101011001100101010001010101010011010101000101000001010100000011100001001010001100100100101001010001011100110110001101001100010010000110101001110011010101110101000001001011",
"01100010010101100110101001010110010010000101100101100011010110000111000101001101011100000110000101010011010010110110010101101110011010100100001101110010010000010101010001100010010000110011100001110111011101110011100001110111011010110111100101001010010000100111010101101000",
"01100010010001110101000001010100010000100110101100111001011011100110001101110011010100000100000101010010010010000100110101100011010011010101001100110110011011010111000001000100001100100101001101000010011000110110111001000110010100000110101001000111011100000100011001110011",
"01100010010001110011100001001101011101100110011101000011010010110100011001111000010001110101000001010010011100100111100101000010001101110110100101010000010101010110101101101111010100100100110000110011010101000011011101110100011100010100010001011010010101100110011001110001",
"01100010010100010101011000110011011001110110010001000100011000110111011100110100010000100101001001011000010000110100010101010010011001100100101001111001010010110011010101010110010101000110010101101101010000100110100101110001010110100111000001000110010110100011010001010101",
"01100010010101000111100101000111010000010101101000110010010100110111000101000110001110000101011100110101011000100110011101101101011100100111010101111000001100100111010101110001010100110111001001100110011011010100111001010111010101100011010101001010001101000011100101101110",
"01100010010101010110010001010101001100110011001001001100011100000110000101101011011010110011010001000011010100000110011100110101010101000011001101101110010001010111010101110001010101010110001001110001010101000110100001101011001110010101001001110001011110000100101101000001",
"01100010010010000011010001101011010011000011001000110001011001000011000101110011011100000101000001100111011101110011100001001010011100110111101001011010011100100110101001000010011101010101010001101111010011100100010101110001011001010100011101001000011101100011001101000111",
"01100010010100110111011001100010011100000111100001001000011010010011000101000111001100110100100001100101011010100111100001110111011001010111001001110101011001110110001001000111010100100101001001000011010011010110110100110011011001010110111101111000011000110110001101001100",
"01100010011000010110010101111010011010100011100101101001010011010101001000110101010011100011001001010000010001010111000101010110010010100100101101101111010101110101000001101001011001110111001101001100011110010110000101100111010100010110100101001100010011000011001101111001",
"01100010010100110101001000110001010001010100110001101010011100110110111100110010010101100101001101011000011100000111101000110011011100100110100101100010010110010111000001101001011001100111100101110001011010010111001001100101010001010011100101000010011001110011001001011000",
"01100010010010000011010101000010010101100110010101011000011101100111000001110010010101010011100000110110011100000111011101111000011110100110010100110101011100100100001101110100001101010101001000111000001100110110001001001110011010010110010100110001011000110101100101101101",
"01100010010100100110011001000111010001100100010001010101010110100110000101001011001100110100110001101111011101010100111001010110010100000110011001000101011010000101010001101010011010100101010001000111010000100100111001000011001100100111000101100101011100110111010001101000",
"01100010010011010111001101001101011110100100010100110001010001110011100000110011011001000111101001101010011101000011100101100010010101110110000100110110011000100100110001100111010010110111100001100010011001000011001001110111011110100101100001100011010110100110111101000001",
"01100010010011100110010001101000010100000101010101001110010010110111000101000111010000110111100001000111010101000110101000110111011101010110101001010010010000110110111101011010011000110100001101100011010100110101100000110010010101010101010001000111010001110110011100110011",
"01100010010001010110011001100110011000010101010101110101001101010110000101000011010000110110111001110100011001000100110001111010010011010101101001110101010101100101010101111000010011000100111001011001010101000110010101110110010100110011000101101000010000100111000101000111",
"01100010010010000110010101000011011100100111001100110111011110010111000001100100010101110111100101011010011010010011000101001010010001000110011001010000010100100111011001010010010000010110100000110110011110100100000101001110011010000011100101100101011001010110011101100010",
"01100010010110010110001101110110010101000111010101010000011100110101000101010101011001000110000101010110001100100011001101111010011001010111011001001000011101110101011001010100010000010101001001100101011100100011011101001101011010010100111000110110010101100101001001010010",
"01100010010001000111100001001000011100100101100101010111010101110110011101010011010001010111011000110111011110100100110001111010011101100011011101100111001100110111011001110011010001100110001100110101011010110111011101000011011110100110101001010011010001100110110101101001",
"01100010010010110101010001101111010101000111011101111001011001010011010100110010010011000100001101110011001101110100011001010011010100010100011001110111011100000100001101110011010100110011100101100011011110000111001101100001010100110101100101110001011100010111001100111001",
"01100010010010100100100001001110011101010100111001100110011010010110100001001011011011010100101101101001011010010111010101010111011101010111100101100101010101000100001001000101010100010101100101101110010110100110100101110000010000010011100101001110011001110011100001110110",
"01100010010010000110000101000111010101010101100001110110011101110100110101000111010100000111100101101011011011010101100001110110011000010011010001101000010001000110101100111000010000110111000101010110010101110100100001101010010101010110010101010110011101100101100101100101",
"01100010010001110100110101101011011100110100010001010110011001110110100001000010010010100110110101100110010001100011001101000100001100100100011101100100010001100100010101101111001100100110010001101011010100100011100001100010010010000100110001010000010010100100101001000001",
"01100010010100110011011000110011010010000101000101010011010000010110110101000111011001010100110101011000010101010111010001000101010110000111100101010000001101000100011001100110010001100111001001100010011110000100010101100001011010010101101001011010010001000100000101110110",
"01100010010011000110101101010000011011100100100001011000011011100111000101100110010110000110011100110011011101100111100001011010011110010110101101000010011000100100010001010001010100000110010100110011011001000111001000110101010100100110010101010001011011100111011101010111",
"01100010010010100011001101110011001101000101010101010101011000010111100101100101001101000100010001000110010101000110001001001000010101110100001001010010011110010011100101101010001101100011011100110010010101010110001001100011011010100011001101000011010100000110100101010110",
"01100010010110100100001001101101011010110011001000110011011101100110011001100111010000110111001101110001011100110101001000110111010000110110101001011001010101110011000101000011011101100111010001001110011001110100011001010010001101100111000001011001011101100110111101001011",
"01100010010100000101000101110101011101000110101001100101011010000100111001101111011110010100101101010000010011010110110101101110011000010100001001111000011101010101100001000111011011010110110101010001001100100110001101110100011101010110000100110011011101010101011101110111",
"01100010010010000011001000111000001101010110101001101001001101110110010001010000011110100111011001000111011100000110100001001110011000010111100101110011011001010100011000110001001110000110100101101111011000110101001001101000011011110101011001010101001100010110100001100011",
"01100010010001010101000101011010010100110101011001010110010010000110100101101011011101000110100001010110010010100100111001100011011000110110010001011010001100010011001001101010001101110100110001100111010010100110111000110111010110010111010101110110010000010110010101010011",
"01100010010110010101011101100001010101110110011001000110010000100100010101001101010110100100010001010011010101000110001100110010011110000111001001100010011001100101000000110011011101010101010101001110011010110100000101001101011001000101011101101001011110000110001101010010",
"01100010010110000110010100110100011101010011001101011010001101100101001101100101011001100110011101000110010010100111001101000011001100100101001001110101011001000110011101101000011101110111000101001000010110010101000001110101011001000100001100110001011000100100101001000100",
"01100010010100100011100000111001011000100110010100111000011010100101010001110100010011100100011001100111010100010110110100110101011101010110001101110110011011100111001001100011011110000100001101100101001100100111010101001011011000010110001101101011010001010110100101010111",
"01100010011000010111101001101001011000110011010001000110001101000110001101101101010110100011001001010011011101110101010101000010011001100101011101100100010101000110110101100001010010100110011001100010010011100100101101100001011101010111101001101010010010100110101001101011",
"01100010010101000111100001000111011010010100000101111010011000110111010101101011011100000110000100110111010100010101001001101000001101010110111001111000010011100111010001000010001101110011100000110001010100100101000101000010001101010101001001000001001100010100001001001101",
"01100010010010100011000101100011001101100101100101010101010001100111011000110010011011100101010101101101011101010110101001100010010101010101000101110110001101010101001101101110011010000110011101000001010000110110001101110000011011010110110101010100011101000100110001101000",
"01100010010110100111000101011000011001000111000001101000011101000110011101100101010100110100101001110010010000100100101101101000010001000110101101100100011010110011001101100101011000100101010101110110011010110110101000111000011000110011011101101101010010100110010000110110",
"01100010010100010100000101100010010000010101100101000110001110000100110101000101010110010011001101101011001100010110011101110110010011100110001001101001010110010110011001001110010011100110100100111001010011010101010100110010001101010101100100110100011011100110111001010001",
"01100010010101100101011001001010011101000111011001010011011010000110001101010000011110000011100101001101011110100100010001110010011100100100000101100100011010110111000101000110011110100111100000110011010011010100000101000101011110100011000101011010011110010111100101101010",
"01100010010101100100000101000011001101000110011000110100011100100101010001010110011011010101011001101011011100100100110101011001011100010101001001100010001100010111100101010101010000110100010001110101010101000011011101100010011010010110110101110111010001010110100101010001",
"01100010010010000110000100110010011000110111010101001010010000010111000101101010010110000101100001011001010110010111000101010101011110100100101101101011011011010110101101110110010000010110010000110100010001000100001001110001010110100111100000110100011101010111001101110001",
"01100010010010000100000101011001001101000101101001100110011011100100010100110101011001100110100001101111010011100100001001111000010110100110111101100110010011010011000101010001011101000101001100111001011001110111101001110011011010010111100001100001011011100111011001101000",
"01100010010110100100110001101001010110100100001100110011001100010111101001101011011101010110011101100101011100010101011101010110001101010111000101101000001101110011011001110101001101010100100001101010011101000100111001101101011110100100111000111001011010110110011101110100",
"01100010010101010100011101110111011110000101011101011010010110100100010101100110010110000100001101100001010101110110100001100100010000110110111101001101011001110110101001100010010001100100100001101011010110100111100001100101010000100100110101101010001100110111011001100110",
"01100010010010100110110101110001010101100011100001000110011110010011000101000111011001110110011001001011011000010101100100110110010110100101001101100110010110010111011001100101011101010011100101010000001101110100110000110001010001110110011001010111001101010100111000110011",
"01100010010011100100001101100110011010100110101101000001010100010101011001011010011011010111001000110001010101010011100101100001011110000110111001100100011101000110110101100010001100010111011100111001010001010111010001110011001101000011001001111001011011010111100000110111",
"01100010010010000110101101001100010101000110001101100001010010100100100001101001010010100101101001010100011010010011001001110010010100100101010101000110011011110101001001111001011010110011001101001011010100100101000101110100001110000111001101100010011001010110100001110111",
"01100010010001110111100001110101010101000110111101101101010100000101101000110011011000110111001001001101010100100011100101011010010011000011000101101001011010000111010001011010011010110111000001101110011001110111001001001011010100010011001001101010010100110100010001010010",
"01100010010010000111011101101110010110010100011101001101001101100011001001000111001101000100111001100001010101100111010001010011010001100101000101001110001101100101001101101110010110100011011100110110011110100110100101011000010010000011010100110011001101000011010100110101",
"01100010010011000011011001001110001110000110010001010111001101000101000001010101001101010111001101110011011100100110100101010100010001000100101001111000010011000111010001000101010001010011001101001100010000010100001101110001001100100110101000111001001100010100111001110000",
"01100010010110010111011101010110010101110011100001100101001110010110100001101101010011000100000101100001011000110110000101111001011110100101001100110110011010110100000101011010011000100110101101100100011001100110111100110001011001110100101101110101001100110111101001011000",
"01100010010011010101011101110010011010010111010101100110010000010100111001101111001110010100100001010001011001100111000101101011010100010100011101010100011000100100000101010110011000110110010100111001001101010011100001110101011100110011000101110001011011110101010101110101",
"01100010010001000011000101101000010010100110010101100101010110010101001101100100010011000011010101000011001110010101100101110101011101100100000101111010010011100111001101111000011010000110001001001100010001010100100001111001010100110101001001110110010011100111000001000101",
"01100010010100000100011101100111010101010110101001000110001101010011011001101011011101010111010001101011011010000111100001101001010101010110001000110001011001000100011001101001010100010110010101011010011101110100101101010111011101110100101001000011011010010111001101001110",
"01100010010110000101010100110101010011010110010101000001001101100110111001110100010001010100111001100101011110100110001101010011010010100110111001011001010001110011100100110111010101110101011101010011011000100110100001000100010101000101010101011010010100110110101001100100",
"01100010010001110111010101001011011100010110011001101001011110010110101101101010010000100011100101110011011010010101101001101000010101110101100001100001010011000110110100111000001101000100001101001101011000100110100001110000011100000111101000110101010010110100011101100111",
"01100010010010000011011001110000011101010101011001010010011001100100011100111001011101110011100101010000010100000110110100110001011100010111010101100101010010110111000101001110010101000100001101011001011001100110001101100010011011010111000001100100011001110100010101010100",
"01100010010001100110100100110110011110100100100001000100001110010111011001010000010101000110000101001110010110000100011100110101011001010110111101000010011011010111011101010000010010100110001101000100010101000101001001100101011010110100110001001101011001000110011001101001",
"01100010010100110110100001101111010001100100001001000101001101000100011001110011010001110111001101110110010110100100111001110100010110100100001101000100011000100111100101101010010010110100010101011001001110010110101101110110011011110011011100110101001110000110100000111000",
"01100010010110000100110001000101010000100101010101110010010101110011011100110101011110000111010101000110010000110111000001011010011011100100110001011001010101010011010101010000010101010100110101011000011001000100010001010111011011010100101001110101011000110011001001101000",
"01100010011000010111101001111010010100110100011001100111001101110100010100110100010110100111101001101010010000100111001101011010010010110101100101100110001100110100110101110000010001000100010001110110010100100111001001100100011101110100010100110111010110010111000001100010",
"01100010010101000110111101000011001101000101100100111001011001110100011100110110010100100101001001011001011010010111101001100010011100010110011001001101011100110100001001010001011011010011010100110001011011100110010001010001010010110111101001111010011001100110001001110010",
"01100010010010100111100001001000010011010101000101000100010101100110010001100011001110010011010001100110010101100110101101001100010101100101000000110101010000110101100101000111010100110111101001000100010100010111010101101111010001000110001101100110011011010101100001101011",
"01100010010110100101001001000011001100100100110001001010010110010101001101010000010011000100011001101000010000010110100001001110010100100101001001011001011000110100110001111000001100110100110101000011010101010011001001110000011101000110100001100101010110010110101000110101",
"01100010010100000011001101110010010100100011100000110010011001100101101001000111011001110101000001101010001100010110110101000110001100110111000101100001010101000110100001011001010110010110100001001011010110100100110001010001011001010101000101000110011011110101000101100100",
"01100010010101100111010101010111011011010101010101110110010011100111001101110110011101010011010001101101010100100100011101100111001100010101010001100011010100110110101001001110010010100110011101111010011001110100011001010111011110010111000101110100010101110110000101010001",
"01100010010110010100101100111000011010100011100100110100010010000110110101010100011010110110100001000011011100110101010101001100011101110101011001100111010010110110111100110010010110100111000001010010011101000011010001101110010000110111011001100010001110000011011001001100",
"01100010010001100011100101010100011001000111100101101000011110000011010001110001010001100111011001110001010001010100000101010011011101000101010101110000010110010111100001000001010000010110001101000110010011000011010101000111010101100101001101110001001101010111101001000101",
"01100010010011010101100001000111011100100111100000110100010101110011010000110001010110000011001001100100011010100110101000111000011110000111001101101101001110000101000100111001010010000011000101011010001100110101001001100010001100010110100101010100010110100111011001110000",
"01100010010101110111001001000100001100010110111100110100011000010110110101000001010101100100111001000111011101010100111001101001010010000100101001000101001101000101010101001110010001000011001001010010010000010100110101000001010000100110000101100100010001000011011101000111",
"01100010010011010111001001010110001100110111101000110100001100010101100101010011010010100100101000110100011100110111000001101111011011110101011001100101010011100100110001101000010010100110010001101101011001000110111001000001010000010101000101110001001110010110101101000111",
"01100010010100010100011101010110011000100110000101011010010101110100010100110100001110000111001101101101010001100011100000111001010100010011010101110101001101010101101001110101011100110111001100110110010001110111001000111001010000010101011001001000010010000101011100110111",
"01100010010010110110101101010011011010110110111101111010011000110111010101001100011100000111001101010101011011010110001001111000001100100011011001100010010110010110000100110100011110000100101101010101010000110110001101010011011110000011001001000100010011000111001001000100",
"01100010010101110111010001101001010101100100001101001010010010000110010001100110011011110110010101110010011001000011001101111001011010100110111101001101010001000100111001100011001101110100010001100001001100110111011000110110001101000110111000110101001110010110010001110010",
"01100010010010100011010101110001001100100111001100110100011001100011100001000011011001110011010000110111011101010100000101101111010001000100101000110011011101000110001101110101011001000110010101111001001101110111010100110010011011010110011000110011011101000100101000111001",
"01100010010110000111100001010110010000010011010001010011010101110011100101101000011100110111011001000100011110000110001001010111001100110110111001101101011011110111011001101111011101000011011001010000011010100100011101010111011011100011010100110110010001100101010001000100",
"01100010010110100011001101000011011001000011100101000010010010000111100000110010001100110100011001000100011010100110111001011001011101110110100100110010011001000011010001011000011000010111010101010010011011010110011001011000010101010110001101100100011011100011011000110100",
"01100010010101010101011001010010011001110111011001001010010010000111100001110000010001000101100001100001010101100011010001011000010001100100101001111000010010000110001001000001011100010100101001100101011100110111001001010000010101010110011001001010001101100110010101100101",
"01100010010001110101011101000001011000010110110101111001010010100101101001100010011101000110111101100010010100110011001101000101011011100111011001100010010000110101010100110010010100000100100001001101011011100111100101000110001101100111011101110000010010100110100001100010",
"01100010010100000100110001110111011100100110101001110100010101110011100001101110010100100011100001110110010001110110001100110010010000010100111001010010011000110100001101111010001101010100110000110011010000110111010100110010011001000100011101110000010011010100101000110111",
"01100010010100000111001101111000010001010101000001110111011100100111001101110011011010000111010101000010011000100101011000110110010001010101001001011001010010100011001101100010010010100110100001111010011010010111100001101010001110000110110100110111001100010100101101100101",
"01100010011000100100011101100010010101000101001000110111010101110101100001100101011010110111100001011000010010100110101001011000001101000111100100110001001100110110101100110001010010000110011001101000001110010110100001001011010000010100110001100011010100110110011101000010",
"01100010010101110101010101110010010100010011011001010011001110010100101100110100010011100100110001101010011100100110001001111001011010110011100001011000011110000111010000110101010011010100000100110110011101100111000001101110011100110110010101000010011100010111010101010010",
"01100010010101110110111001000011011110010110010100110001011101010111001001111000001100100100001000110010010000010100101101100110010100100111011001011010010011100100000100111001001110000101001100110111011101100100100001001000011000010100000100110110011100110111000101011001",
"01100010010110100110100101111010010010100011011001111010010000100111000101111000010101000111001100110100010000110110000101110001011100010011011101101110010011100100111001110000010011000101011101110100011101000101000001001000010010110111010001100100011100100111101001100001",
"01100010010001100101001001100100010101100111001100110010011001100100101101101111001100010111100001000110011110000100011001010001010000100100110101011000011001100011100001110100011011110100011101001000011010000111100001000011010001110110111001100001011010000100010101000110",
"01100010010110000101011001101000011001010110111101010101011100110110010101000111001110000110110101100010011011110100110001010110011011110111100101000100010001010101101001010010010000110101010001010000010110010100011001100111011010000111001101010100011110100110101001110101",
"01100010010001110011001001001100010101110110010001001101011110000110100001000101011000100111100001011000011101000100001100110010011001010100010101101011011110000111001001001110010101110101100001101010010001100111100101010101011100100110101101010001010101110100100000110111",
"01100010010001110011011100110011010000100101010000110101010100010011010001000110010010100110101101101111011001100100110101101001010011100100010101110100001101100011100001001011011010000111010101000001011011110101000001101101011100000100011101010000010110010101001001111010",
"01100010010010100110001101101001011000100110110101100101010110010111101001000100010100010101010001011000011011010110001101101001010100000100100001010010011000100011001001110110011010000101010001011000001101000110100101111001010010110111100000110101010100010011011101100001",
"01100010010011010011100100110100010010100011011001001010011101110101011001100111001101110101000101011010010101010100001001100100011010100110011001001011010000100011011101101110010100100011001100110011010010110110001001111000010011010111001101011001010011010110100001001000",

};
#endif // BDTCOIN_VALIDATION_H
