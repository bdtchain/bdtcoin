// Copyright (c) 2016-present The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/license/mit/.

#ifndef BDTCOIN_KERNEL_MEMPOOL_REMOVAL_REASON_H
#define BDTCOIN_KERNEL_MEMPOOL_REMOVAL_REASON_H

#include <string>

/** Reason why a transaction was removed from the mempool,
 * this is passed to the notification signal.
 */
enum class MemPoolRemovalReason {
    EXPIRY,      //!< Expired from mempool
    SIZELIMIT,   //!< Removed in size limiting
    REORG,       //!< Removed for reorganization
    BLOCK,       //!< Removed for block
    CONFLICT,    //!< Removed for conflict with in-block transaction
    REPLACED,    //!< Removed for replacement
};

std::string RemovalReasonToString(const MemPoolRemovalReason& r) noexcept;

#endif // BDTCOIN_KERNEL_MEMPOOL_REMOVAL_REASON_H
