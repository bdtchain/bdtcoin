// Copyright (c) 2019-2020 Johir Uddin Sultan
// Copyright (c) 2021-2022 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <policy/settings.h>

#include <policy/feerate.h>
#include <policy/policy.h>

bool fIsBareMultisigStd = DEFAULT_PERMIT_BAREMULTISIG;
CFeeRate incrementalRelayFee = CFeeRate(DEFAULT_INCREMENTAL_RELAY_FEE);
CFeeRate dustRelayFee = CFeeRate(DUST_RELAY_TX_FEE);
unsigned int nBytesPerSigOp = DEFAULT_BYTES_PER_SIGOP;
