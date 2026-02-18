// Copyright (c) 2018-2025 JUS
// Copyright (c) 2009-2020 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_NETMESSAGEMAKER_H
#define BDTCOIN_NETMESSAGEMAKER_H

#include <net.h>
#include <serialize.h>

namespace NetMsg {
    template <typename... Args>
    CSerializedNetMsg Make(std::string msg_type, Args&&... args)
    {
        CSerializedNetMsg msg;
        msg.m_type = std::move(msg_type);
        VectorWriter{msg.data, 0, std::forward<Args>(args)...};
        return msg;
    }
} // namespace NetMsg

#endif // BDTCOIN_NETMESSAGEMAKER_H
