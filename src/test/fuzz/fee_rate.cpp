// Copyright (c) 2020 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <amount.h>
#include <policy/feerate.h>
#include <test/fuzz/FuzzedDataProvider.h>
#include <test/fuzz/fuzz.h>
#include <test/fuzz/util.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

void test_one_input(const std::vector<uint8_t>& buffer)
{
    FuzzedDataProvider fuzzed_data_provider(buffer.data(), buffer.size());
    const CAmount juss_per_k = ConsumeMoney(fuzzed_data_provider);
    const CFeeRate fee_rate{juss_per_k};

    (void)fee_rate.GetFeePerK();
    const size_t bytes = fuzzed_data_provider.ConsumeIntegral<size_t>();
    if (!MultiplicationOverflow(static_cast<int64_t>(bytes), juss_per_k) && bytes <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        (void)fee_rate.GetFee(bytes);
    }
    (void)fee_rate.ToString();

    const CAmount another_juss_per_k = ConsumeMoney(fuzzed_data_provider);
    CFeeRate larger_fee_rate{another_juss_per_k};
    larger_fee_rate += fee_rate;
    if (juss_per_k != 0 && another_juss_per_k != 0) {
        assert(fee_rate < larger_fee_rate);
        assert(!(fee_rate > larger_fee_rate));
        assert(!(fee_rate == larger_fee_rate));
        assert(fee_rate <= larger_fee_rate);
        assert(!(fee_rate >= larger_fee_rate));
        assert(fee_rate != larger_fee_rate);
    }
}
