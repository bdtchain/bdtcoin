# Copyright (c) 2013-2016 The Bdtcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(WARN_CXXFLAGS) $(NOWARN_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
AM_LIBTOOLFLAGS = --preserve-dup-deps
PTHREAD_FLAGS = $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

BDTCOIN_INCLUDES=-I$(builddir) -I$(srcdir)/secp256k1/include $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS)

BDTCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

LIBBDTCOIN_SERVER=libbdtcoin_server.a
LIBBDTCOIN_COMMON=libbdtcoin_common.a
LIBBDTCOIN_CONSENSUS=libbdtcoin_consensus.a
LIBBDTCOIN_CLI=libbdtcoin_cli.a
LIBBDTCOIN_UTIL=libbdtcoin_util.a
LIBBDTCOIN_CRYPTO_BASE=crypto/libbdtcoin_crypto_base.a
LIBBDTCOINQT=qt/libbdtcoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBBDTCOIN_ZMQ=libbdtcoin_zmq.a
endif
if BUILD_BDTCOIN_LIBS
LIBBDTCOINCONSENSUS=libbdtcoinconsensus.la
endif
if ENABLE_WALLET
LIBBDTCOIN_WALLET=libbdtcoin_wallet.a
LIBBDTCOIN_WALLET_TOOL=libbdtcoin_wallet_tool.a
endif

LIBBDTCOIN_CRYPTO= $(LIBBDTCOIN_CRYPTO_BASE)
if ENABLE_SSE41
LIBBDTCOIN_CRYPTO_SSE41 = crypto/libbdtcoin_crypto_sse41.a
LIBBDTCOIN_CRYPTO += $(LIBBDTCOIN_CRYPTO_SSE41)
endif
if ENABLE_AVX2
LIBBDTCOIN_CRYPTO_AVX2 = crypto/libbdtcoin_crypto_avx2.a
LIBBDTCOIN_CRYPTO += $(LIBBDTCOIN_CRYPTO_AVX2)
endif
if ENABLE_SHANI
LIBBDTCOIN_CRYPTO_SHANI = crypto/libbdtcoin_crypto_shani.a
LIBBDTCOIN_CRYPTO += $(LIBBDTCOIN_CRYPTO_SHANI)
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBDTCOIN_CRYPTO) \
  $(LIBBDTCOIN_UTIL) \
  $(LIBBDTCOIN_COMMON) \
  $(LIBBDTCOIN_CONSENSUS) \
  $(LIBBDTCOIN_SERVER) \
  $(LIBBDTCOIN_CLI) \
  $(LIBBDTCOIN_WALLET) \
  $(LIBBDTCOIN_WALLET_TOOL) \
  $(LIBBDTCOIN_ZMQ)

lib_LTLIBRARIES = $(LIBBDTCOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BDTCOIND
  bin_PROGRAMS += bdtcoind
endif

if BUILD_BDTCOIN_NODE
  bin_PROGRAMS += bdtcoin-node
endif

if BUILD_BDTCOIN_CLI
  bin_PROGRAMS += bdtcoin-cli
endif
if BUILD_BDTCOIN_TX
  bin_PROGRAMS += bdtcoin-tx
endif
if ENABLE_WALLET
if BUILD_BDTCOIN_WALLET
  bin_PROGRAMS += bdtcoin-wallet
endif
endif

.PHONY: FORCE check-symbols check-security
# bdtcoin core #
BDTCOIN_CORE_H = \
  addrdb.h \
  addrman.h \
  attributes.h \
  banman.h \
  base58.h \
  bech32.h \
  blockencodings.h \
  blockfilter.h \
  bloom.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/assumptions.h \
  compat/byteswap.h \
  compat/cpuid.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_check.h \
  consensus/tx_verify.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  dbwrapper.h \
  flatfile.h \
  fs.h \
  httprpc.h \
  httpserver.h \
  index/base.h \
  index/blockfilterindex.h \
  index/disktxpos.h \
  index/txindex.h \
  indirectmap.h \
  init.h \
  interfaces/chain.h \
  interfaces/handler.h \
  interfaces/node.h \
  interfaces/wallet.h \
  key.h \
  key_io.h \
  logging.h \
  logging/timer.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_permissions.h \
  net_processing.h \
  net_types.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  node/coin.h \
  node/coinstats.h \
  node/context.h \
  node/psbt.h \
  node/transaction.h \
  node/ui_interface.h \
  node/utxo_snapshot.h \
  noui.h \
  optional.h \
  outputtype.h \
  policy/feerate.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  policy/settings.h \
  pow.h \
  protocol.h \
  psbt.h \
  random.h \
  randomenv.h \
  reverse_iterator.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/mining.h \
  rpc/protocol.h \
  rpc/rawtransaction_util.h \
  rpc/register.h \
  rpc/request.h \
  rpc/server.h \
  rpc/util.h \
  scheduler.h \
  script/descriptor.h \
  script/keyorigin.h \
  script/sigcache.h \
  script/sign.h \
  script/signingprovider.h \
  script/standard.h \
  shutdown.h \
  signet.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadinterrupt.h \
  threadsafety.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txrequest.h \
  txmempool.h \
  undo.h \
  util/asmap.h \
  util/bip32.h \
  util/bytevectorhash.h \
  util/check.h \
  util/error.h \
  util/fees.h \
  util/golombrice.h \
  util/macros.h \
  util/memory.h \
  util/message.h \
  util/moneystr.h \
  util/rbf.h \
  util/ref.h \
  util/settings.h \
  util/spanparsing.h \
  util/string.h \
  util/system.h \
  util/threadnames.h \
  util/time.h \
  util/translation.h \
  util/ui_change_type.h \
  util/url.h \
  util/vector.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  versionbitsinfo.h \
  wallet/bdb.h \
  wallet/coincontrol.h \
  wallet/coinselection.h \
  wallet/context.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/feebumper.h \
  wallet/fees.h \
  wallet/ismine.h \
  wallet/load.h \
  wallet/rpcwallet.h \
  wallet/salvage.h \
  wallet/scriptpubkeyman.h \
  wallet/sqlite.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallettool.h \
  wallet/walletutil.h \
  walletinitinterface.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  zmq/zmqrpc.h \
  zmq/zmqutil.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh "$(abs_top_builddir)/src/obj/build.h" \
	  "$(abs_top_srcdir)"
libbdtcoin_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between bdtcoind and bdtcoin-qt
# Contains code accessing mempool and chain state that is meant to be separated
# from wallet and gui code (see node/README.md). Shared code should go in
# libbdtcoin_common or libbdtcoin_util libraries, instead.
libbdtcoin_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libbdtcoin_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_server_a_SOURCES = \
  addrdb.cpp \
  addrman.cpp \
  banman.cpp \
  blockencodings.cpp \
  blockfilter.cpp \
  chain.cpp \
  consensus/tx_verify.cpp \
  dbwrapper.cpp \
  flatfile.cpp \
  httprpc.cpp \
  httpserver.cpp \
  index/base.cpp \
  index/blockfilterindex.cpp \
  index/txindex.cpp \
  init.cpp \
  interfaces/chain.cpp \
  interfaces/node.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  node/coin.cpp \
  node/coinstats.cpp \
  node/context.cpp \
  node/psbt.cpp \
  node/transaction.cpp \
  node/ui_interface.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/rbf.cpp \
  policy/settings.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  shutdown.cpp \
  signet.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txrequest.cpp \
  txmempool.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(BDTCOIN_CORE_H)

if ENABLE_WALLET
libbdtcoin_server_a_SOURCES += wallet/init.cpp
endif
if !ENABLE_WALLET
libbdtcoin_server_a_SOURCES += dummywallet.cpp
endif

if ENABLE_ZMQ
libbdtcoin_zmq_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES) $(ZMQ_CFLAGS)
libbdtcoin_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp \
  zmq/zmqrpc.cpp \
  zmq/zmqutil.cpp
endif


# wallet: shared between bdtcoind and bdtcoin-qt, but only linked
# when wallet enabled
libbdtcoin_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES) $(SQLITE_CFLAGS)
libbdtcoin_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_wallet_a_SOURCES = \
  interfaces/wallet.cpp \
  wallet/bdb.cpp \
  wallet/coincontrol.cpp \
  wallet/context.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/feebumper.cpp \
  wallet/fees.cpp \
  wallet/load.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/salvage.cpp \
  wallet/scriptpubkeyman.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  wallet/walletutil.cpp \
  wallet/coinselection.cpp \
  $(BDTCOIN_CORE_H)

if USE_SQLITE
libbdtcoin_wallet_a_SOURCES += wallet/sqlite.cpp
endif

libbdtcoin_wallet_tool_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
libbdtcoin_wallet_tool_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_wallet_tool_a_SOURCES = \
  wallet/wallettool.cpp \
  $(BDTCOIN_CORE_H)

# crypto primitives library
crypto_libbdtcoin_crypto_base_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libbdtcoin_crypto_base_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbdtcoin_crypto_base_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha_poly_aead.h \
  crypto/chacha_poly_aead.cpp \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hkdf_sha256_32.cpp \
  crypto/hkdf_sha256_32.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/poly1305.h \
  crypto/poly1305.cpp \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha3.cpp \
  crypto/sha3.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/siphash.cpp \
  crypto/siphash.h

if USE_ASM
crypto_libbdtcoin_crypto_base_a_SOURCES += crypto/sha256_sse4.cpp
endif

crypto_libbdtcoin_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbdtcoin_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libbdtcoin_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libbdtcoin_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
crypto_libbdtcoin_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libbdtcoin_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbdtcoin_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libbdtcoin_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libbdtcoin_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
crypto_libbdtcoin_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

crypto_libbdtcoin_crypto_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbdtcoin_crypto_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libbdtcoin_crypto_shani_a_CXXFLAGS += $(SHANI_CXXFLAGS)
crypto_libbdtcoin_crypto_shani_a_CPPFLAGS += -DENABLE_SHANI
crypto_libbdtcoin_crypto_shani_a_SOURCES = crypto/sha256_shani.cpp

# consensus: shared between all executables that validate any consensus rules.
libbdtcoin_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
libbdtcoin_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/tx_check.cpp \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/bdtcoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  span.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h

# common: shared between bdtcoind, and bdtcoin-qt and non-server tools
libbdtcoin_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
libbdtcoin_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  bloom.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  key_io.cpp \
  merkleblock.cpp \
  netaddress.cpp \
  netbase.cpp \
  net_permissions.cpp \
  outputtype.cpp \
  policy/feerate.cpp \
  policy/policy.cpp \
  protocol.cpp \
  psbt.cpp \
  rpc/rawtransaction_util.cpp \
  rpc/util.cpp \
  scheduler.cpp \
  script/descriptor.cpp \
  script/sign.cpp \
  script/signingprovider.cpp \
  script/standard.cpp \
  versionbitsinfo.cpp \
  warnings.cpp \
  $(BDTCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libbdtcoin_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
libbdtcoin_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  interfaces/handler.cpp \
  logging.cpp \
  random.cpp \
  randomenv.cpp \
  rpc/request.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util/asmap.cpp \
  util/bip32.cpp \
  util/bytevectorhash.cpp \
  util/error.cpp \
  util/fees.cpp \
  util/system.cpp \
  util/message.cpp \
  util/moneystr.cpp \
  util/rbf.cpp \
  util/settings.cpp \
  util/threadnames.cpp \
  util/spanparsing.cpp \
  util/strencodings.cpp \
  util/string.cpp \
  util/time.cpp \
  $(BDTCOIN_CORE_H)

if USE_LIBEVENT
libbdtcoin_util_a_SOURCES += util/url.cpp
endif

if GLIBC_BACK_COMPAT
libbdtcoin_util_a_SOURCES += compat/glibc_compat.cpp
AM_LDFLAGS += $(COMPAT_LDFLAGS)
endif

# cli: shared between bdtcoin-cli and bdtcoin-qt
libbdtcoin_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
libbdtcoin_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbdtcoin_cli_a_SOURCES = \
  compat/stdin.h \
  compat/stdin.cpp \
  rpc/client.cpp \
  $(BDTCOIN_CORE_H)

nodist_libbdtcoin_util_a_SOURCES = $(srcdir)/obj/build.h
#

# bdtcoind & bdtcoin-node binaries #
bdtcoin_daemon_sources = bdtcoind.cpp
bdtcoin_bin_cppflags = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
bdtcoin_bin_cxxflags = $(AM_CXXFLAGS) $(PIE_FLAGS)
bdtcoin_bin_ldflags = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
bdtcoin_daemon_sources += bdtcoind-res.rc
endif

bdtcoin_bin_ldadd = \
  $(LIBBDTCOIN_WALLET) \
  $(LIBBDTCOIN_COMMON) \
  $(LIBBDTCOIN_UTIL) \
  $(LIBUNIVALUE) \
  $(LIBBDTCOIN_ZMQ) \
  $(LIBBDTCOIN_CONSENSUS) \
  $(LIBBDTCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

bdtcoin_bin_ldadd += $(BOOST_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(SQLITE_LIBS)

bdtcoind_SOURCES = $(bdtcoin_daemon_sources)
bdtcoind_CPPFLAGS = $(bdtcoin_bin_cppflags)
bdtcoind_CXXFLAGS = $(bdtcoin_bin_cxxflags)
bdtcoind_LDFLAGS = $(bdtcoin_bin_ldflags)
bdtcoind_LDADD = $(LIBBDTCOIN_SERVER) $(bdtcoin_bin_ldadd)

bdtcoin_node_SOURCES = $(bdtcoin_daemon_sources)
bdtcoin_node_CPPFLAGS = $(bdtcoin_bin_cppflags)
bdtcoin_node_CXXFLAGS = $(bdtcoin_bin_cxxflags)
bdtcoin_node_LDFLAGS = $(bdtcoin_bin_ldflags)
bdtcoin_node_LDADD = $(LIBBDTCOIN_SERVER) $(bdtcoin_bin_ldadd)

# bdtcoin-cli binary #
bdtcoin_cli_SOURCES = bdtcoin-cli.cpp
bdtcoin_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES) $(EVENT_CFLAGS)
bdtcoin_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
bdtcoin_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
bdtcoin_cli_SOURCES += bdtcoin-cli-res.rc
endif

bdtcoin_cli_LDADD = \
  $(LIBBDTCOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBDTCOIN_UTIL) \
  $(LIBBDTCOIN_CRYPTO)

bdtcoin_cli_LDADD += $(BOOST_LIBS) $(EVENT_LIBS)
#

# bdtcoin-tx binary #
bdtcoin_tx_SOURCES = bdtcoin-tx.cpp
bdtcoin_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BDTCOIN_INCLUDES)
bdtcoin_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
bdtcoin_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
bdtcoin_tx_SOURCES += bdtcoin-tx-res.rc
endif

bdtcoin_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBDTCOIN_COMMON) \
  $(LIBBDTCOIN_UTIL) \
  $(LIBBDTCOIN_CONSENSUS) \
  $(LIBBDTCOIN_CRYPTO) \
  $(LIBSECP256K1)

bdtcoin_tx_LDADD += $(BOOST_LIBS)
#

# bdtcoin-wallet binary #
bdtcoin_wallet_SOURCES = bdtcoin-wallet.cpp
bdtcoin_wallet_CPPFLAGS = $(bdtcoin_bin_cppflags)
bdtcoin_wallet_CXXFLAGS = $(bdtcoin_bin_cxxflags)
bdtcoin_wallet_LDFLAGS = $(bdtcoin_bin_ldflags)
bdtcoin_wallet_LDADD = $(LIBBDTCOIN_WALLET_TOOL) $(bdtcoin_bin_ldadd)

if TARGET_WINDOWS
bdtcoin_wallet_SOURCES += bdtcoin-wallet-res.rc
endif
#

# bdtcoinconsensus library #
if BUILD_BDTCOIN_LIBS
include_HEADERS = script/bdtcoinconsensus.h
libbdtcoinconsensus_la_SOURCES = support/cleanse.cpp $(crypto_libbdtcoin_crypto_base_a_SOURCES) $(libbdtcoin_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libbdtcoinconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libbdtcoinconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libbdtcoinconsensus_la_LIBADD = $(LIBSECP256K1)
libbdtcoinconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BDTCOIN_INTERNAL
libbdtcoinconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crc32c/src/*.gcda crc32c/src/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += index/*.gcda index/*.gcno
CLEANFILES += interfaces/*.gcda interfaces/*.gcno
CLEANFILES += node/*.gcda node/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += rpc/*.gcda rpc/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += util/*.gcda util/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += obj/build.h

EXTRA_DIST = $(CTAES_DIST)


config/bdtcoin-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/bdtcoin-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/bdtcoin-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/bdtcoin-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS)
if TARGET_DARWIN
	@echo "Checking macOS dynamic libraries..."
	$(AM_V_at) OTOOL=$(OTOOL) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if TARGET_WINDOWS
	@echo "Checking Windows dynamic libraries..."
	$(AM_V_at) OBJDUMP=$(OBJDUMP) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) OTOOL=$(OTOOL) $(PYTHON) $(top_srcdir)/contrib/devtools/security-check.py $(bin_PROGRAMS)
endif

if EMBEDDED_LEVELDB
include Makefile.crc32c.include
include Makefile.leveldb.include
endif

include Makefile.test_util.include
include Makefile.test_fuzz.include

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
