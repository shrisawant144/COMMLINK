#!/bin/bash
set -euo pipefail

REPO_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE=${1:-Release}
cmake -S "$REPO_DIR" -B "$REPO_DIR/build" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$REPO_DIR/build" --parallel "${COMMLINK_BUILD_JOBS:-4}"
echo "Build complete: $REPO_DIR/build/bin/commlink"
