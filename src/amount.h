// Copyright (c) 2019-2020 Johir Uddin Sultan
// Copyright (c) 2020-2021 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_AMOUNT_H
#define BDTCOIN_AMOUNT_H

#include <stdint.h>

/** Amount in juss (Can be negative) */
typedef int64_t CAmount;

static const CAmount COIN = 100000000;

/** No amount larger than this (in jus) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Bdtcoin
 * currently happens to be less than 21,000,000 BDTC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static const CAmount MAX_MONEY = 71000000 * COIN;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#endif //  BDTCOIN_AMOUNT_H
