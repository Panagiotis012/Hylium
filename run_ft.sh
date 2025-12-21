#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

pkill -f hyliumd || true

TMPROOT="/Users/Apple/Desktop/hyl_ft"
mkdir -p "$TMPROOT" build/test/functional

if [[ $# -lt 1 ]]; then
  echo "Usage: ./run_ft.sh <test1.py> [test2.py ...] | all"
  exit 2
fi

if [[ "${1:-}" == "all" ]]; then
  shift || true
  set -- feature_hylium_devnet_smoke.py feature_hylium_p2p_propagation.py feature_hylium_reorg_basic.py
fi

for TEST in "$@"; do
  if [[ ! -f "test/functional/$TEST" ]]; then
    echo "No such test: test/functional/$TEST"
    exit 2
  fi

  ln -sf "$(pwd)/test/functional/$TEST" "$(pwd)/build/test/functional/$TEST"

  python3 test/functional/test_runner.py \
    --tmpdir="$TMPROOT" \
    "$TEST"
done
