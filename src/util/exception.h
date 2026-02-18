// Copyright (c) 2018-2025 JUS
// Copyright (c) 2009-2023 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_UTIL_EXCEPTION_H
#define BDTCOIN_UTIL_EXCEPTION_H

#include <exception>
#include <string_view>

void PrintExceptionContinue(const std::exception* pex, std::string_view thread_name);

#endif // BDTCOIN_UTIL_EXCEPTION_H
