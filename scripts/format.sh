#!/bin/bash

echo "Formatting code..."
find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
echo "✓ Code formatted"
