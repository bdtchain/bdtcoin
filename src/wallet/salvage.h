// Copyright (c) 2009-2010 Johir Uddin Sultan
// Copyright (c) 2009-2020 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_WALLET_SALVAGE_H
#define BDTCOIN_WALLET_SALVAGE_H

#include <fs.h>
#include <streams.h>

struct bilingual_str;

bool RecoverDatabaseFile(const fs::path& file_path, bilingual_str& error, std::vector<bilingual_str>& warnings);

#endif // BDTCOIN_WALLET_SALVAGE_H
