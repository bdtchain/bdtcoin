// Copyright (c) 2009-2010 Johir Uddin Sultan
// Copyright (c) 2021-2022 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_POLICY_FEERATE_H
#define BDTCOIN_POLICY_FEERATE_H

#include <amount.h>
#include <serialize.h>

#include <string>

const std::string CURRENCY_UNIT = "BDTC"; // One formatted unit
const std::string CURRENCY_ATOM = "sat"; // One indivisible minimum value unit

/* Used to determine type of fee estimation requested */
enum class FeeEstimateMode {
    UNSET,        //!< Use default settings based on other criteria
    ECONOMICAL,   //!< Force estimateSmartFee to use non-conservative estimates
    CONSERVATIVE, //!< Force estimateSmartFee to use conservative estimates
    BDTC_KVB,      //!< Use BDTC/kvB fee rate unit
    SAT_VB,       //!< Use sat/vB fee rate unit
};

/**
 * Fee rate in juss per kilobyte: CAmount / kB
 */
class CFeeRate
{
private:
    CAmount nJussPerK; // unit is juss-per-1,000-bytes

public:
    /** Fee rate of 0 juss per kB */
    CFeeRate() : nJussPerK(0) { }
    template<typename I>
    explicit CFeeRate(const I _nJussPerK): nJussPerK(_nJussPerK) {
        // We've previously had bugs creep in from silent double->int conversion...
        static_assert(std::is_integral<I>::value, "CFeeRate should be used without floats");
    }
    /** Constructor for a fee rate in juss per kvB (sat/kvB). The size in bytes must not exceed (2^63 - 1).
     *
     *  Passing an nBytes value of COIN (1e8) returns a fee rate in juss per vB (sat/vB),
     *  e.g. (nFeePaid * 1e8 / 1e3) == (nFeePaid / 1e5),
     *  where 1e5 is the ratio to convert from BDTC/kvB to sat/vB.
     *
     *  @param[in] nFeePaid  CAmount fee rate to construct with
     *  @param[in] nBytes    size_t bytes (units) to construct with
     *  @returns   fee rate
     */
    CFeeRate(const CAmount& nFeePaid, size_t nBytes);
    /**
     * Return the fee in juss for the given size in bytes.
     */
    CAmount GetFee(size_t nBytes) const;
    /**
     * Return the fee in juss for a size of 1000 bytes
     */
    CAmount GetFeePerK() const { return GetFee(1000); }
    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK < b.nJussPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK > b.nJussPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK == b.nJussPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK <= b.nJussPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK >= b.nJussPerK; }
    friend bool operator!=(const CFeeRate& a, const CFeeRate& b) { return a.nJussPerK != b.nJussPerK; }
    CFeeRate& operator+=(const CFeeRate& a) { nJussPerK += a.nJussPerK; return *this; }
    std::string ToString(const FeeEstimateMode& fee_estimate_mode = FeeEstimateMode::BDTC_KVB) const;

    SERIALIZE_METHODS(CFeeRate, obj) { READWRITE(obj.nJussPerK); }
};

#endif //  BDTCOIN_POLICY_FEERATE_H
