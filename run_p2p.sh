#!/usr/bin/env bash
set -Eeuo pipefail

cd "$(dirname "$0")"

# Ensure build/test/functional has the test symlink
mkdir -p build/test/functional
ln -sf "$(pwd)/test/functional/feature_hylium_p2p_propagation.py" \
      "$(pwd)/build/test/functional/feature_hylium_p2p_propagation.py"

# Ensure config.ini link exists
ln -sf "$(pwd)/build/test/config.ini" test/config.ini

# Kill stray nodes
pkill -f hyliumd || true

# Run
mkdir -p /Users/Apple/Desktop/hyl_ft
python3 test/functional/test_runner.py \
  --tmpdir=/Users/Apple/Desktop/hyl_ft \
  feature_hylium_p2p_propagation.py
