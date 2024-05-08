// Copyright (c) 2019-2020 Johir Uddin Sultan
// Copyright (c) 2020-2021 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_SHUTDOWN_H
#define BDTCOIN_SHUTDOWN_H

void StartShutdown();
void AbortShutdown();
bool ShutdownRequested();

#endif
