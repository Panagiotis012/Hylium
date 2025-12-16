# Copyright (c) 2023-present The Hylium Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

function(generate_setup_nsi)
  set(abs_top_srcdir ${PROJECT_SOURCE_DIR})
  set(abs_top_builddir ${PROJECT_BINARY_DIR})
  set(CLIENT_URL ${PROJECT_HOMEPAGE_URL})
  set(CLIENT_TARNAME "hylium")
  set(HYLIUM_WRAPPER_NAME "hylium")
  set(HYLIUM_GUI_NAME "hylium-qt")
  set(HYLIUM_DAEMON_NAME "hyliumd")
  set(HYLIUM_CLI_NAME "hylium-cli")
  set(HYLIUM_TX_NAME "hylium-tx")
  set(HYLIUM_WALLET_TOOL_NAME "hylium-wallet")
  set(HYLIUM_TEST_NAME "test_hylium")
  set(EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
  configure_file(${PROJECT_SOURCE_DIR}/share/setup.nsi.in ${PROJECT_BINARY_DIR}/hylium-win64-setup.nsi USE_SOURCE_PERMISSIONS @ONLY)
endfunction()
