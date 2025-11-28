#!/bin/bash
set -e

BUILD_TYPE=${1:-Release}
BUILD_DIR="build"

echo "Building CommLink (${BUILD_TYPE})..."

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
cmake --build .

echo "✓ Build complete: ${BUILD_DIR}/bin/commlink"
