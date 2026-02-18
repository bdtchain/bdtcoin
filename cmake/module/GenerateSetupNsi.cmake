# Copyright (c) 2023-present The Bdtcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

function(generate_setup_nsi)
  set(abs_top_srcdir ${PROJECT_SOURCE_DIR})
  set(abs_top_builddir ${PROJECT_BINARY_DIR})
  set(CLIENT_URL ${PROJECT_HOMEPAGE_URL})
  set(CLIENT_TARNAME "bdtcoin")
  set(BDTCOIN_GUI_NAME "bdtcoin-qt")
  set(BDTCOIN_DAEMON_NAME "bdtcoind")
  set(BDTCOIN_CLI_NAME "bdtcoin-cli")
  set(BDTCOIN_TX_NAME "bdtcoin-tx")
  set(BDTCOIN_WALLET_TOOL_NAME "bdtcoin-wallet")
  set(BDTCOIN_TEST_NAME "test_bdtcoin")
  set(EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
  configure_file(${PROJECT_SOURCE_DIR}/share/setup.nsi.in ${PROJECT_BINARY_DIR}/bdtcoin-win64-setup.nsi USE_SOURCE_PERMISSIONS @ONLY)
endfunction()
