#!/usr/bin/env bash
set -euo pipefail

# Detect OpenSSL prefix via brew (openssl@3 fallback to openssl).
detect_openssl() {
  if brew --prefix openssl@3 >/dev/null 2>&1; then
    brew --prefix openssl@3
  else
    brew --prefix openssl
  fi
}

OPENSSL_PREFIX=$(detect_openssl)
INCLUDE_DIR="$OPENSSL_PREFIX/include"
LIB_DIR="$OPENSSL_PREFIX/lib"

echo "OpenSSL prefix: $OPENSSL_PREFIX"

echo "Building /tmp/genesis_hylium ..."
c++ -O3 -march=native -funroll-loops -std=c++17 \
  -I"$INCLUDE_DIR" -L"$LIB_DIR" \
  contrib/devtools/genesis_hylium.cpp -lcrypto -o /tmp/genesis_hylium

THREADS="${HYLIUM_GENESIS_THREADS:-$(sysctl -n hw.ncpu)}"
MAIN_START_OPT=""
if [[ -n "${HYLIUM_GENESIS_START:-}" ]]; then
  MAIN_START_OPT="HYLIUM_GENESIS_START=${HYLIUM_GENESIS_START}"
fi

run_net() {
  local net="$1"
  local extra_env="$2"
  local log="docs/genesis_out_${net}.txt"
  echo "Mining ${net} ... (log: ${log})"
  env HYLIUM_GENESIS_THREADS="${THREADS}" HYLIUM_GENESIS_NETS="${net}" ${extra_env} /tmp/genesis_hylium | tee "${log}"
  # Print compact summary (last Found genesis line if present)
  if grep -q "Found genesis" "${log}"; then
    local line
    line=$(grep "Found genesis" "${log}" | tail -1)
    echo "SUMMARY ${net}: ${line}"
  else
    echo "SUMMARY ${net}: not found (check ${log})"
  fi
}

# Order: testnet, test4, regtest, main (main can resume with HYLIUM_GENESIS_START)
run_net "testnet" ""
run_net "test4" ""
run_net "regtest" ""
run_net "main" "${MAIN_START_OPT}"

echo "Done. Check docs/genesis_out_<net>.txt for full logs."
