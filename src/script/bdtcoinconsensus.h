// Copyright (c) 2020-2021 JUS
// Copyright (c) 2020-2022The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_SCRIPT_BDTCOINCONSENSUS_H
#define BDTCOIN_SCRIPT_BDTCOINCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_BDTCOIN_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/bdtcoin-config.h>
  #if defined(_WIN32)
    #if defined(DLL_EXPORT)
      #if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
        #define EXPORT_SYMBOL __declspec(dllexport)
      #else
        #define EXPORT_SYMBOL
      #endif
    #endif
  #elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBBDTCOINCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BDTCOINCONSENSUS_API_VER 1

typedef enum bdtcoinconsensus_error_t
{
    bdtcoinconsensus_ERR_OK = 0,
    bdtcoinconsensus_ERR_TX_INDEX,
    bdtcoinconsensus_ERR_TX_SIZE_MISMATCH,
    bdtcoinconsensus_ERR_TX_DESERIALIZE,
    bdtcoinconsensus_ERR_AMOUNT_REQUIRED,
    bdtcoinconsensus_ERR_INVALID_FLAGS,
} bdtcoinconsensus_error;

/** Script verification flags */
enum
{
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY           = (1U << 4), // enforce NULLDUMMY (BIP147)
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS             = (1U << 11), // enable WITNESS (BIP141)
    bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH | bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
                                                               bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY | bdtcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int bdtcoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                 const unsigned char *txTo        , unsigned int txToLen,
                                                 unsigned int nIn, unsigned int flags, bdtcoinconsensus_error* err);

EXPORT_SYMBOL int bdtcoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, bdtcoinconsensus_error* err);

EXPORT_SYMBOL unsigned int bdtcoinconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // BDTCOIN_SCRIPT_BDTCOINCONSENSUS_H
