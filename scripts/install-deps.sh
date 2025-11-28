#!/bin/bash

if [ -f /etc/debian_version ]; then
    sudo apt-get update
    sudo apt-get install -y \
        qtbase5-dev \
        libqt5sql5-sqlite \
        cmake \
        build-essential \
        clang-format \
        clang-tidy \
        doxygen
elif [ -f /etc/redhat-release ]; then
    sudo dnf install -y \
        qt5-qtbase-devel \
        qt5-qtbase-sqlite \
        cmake \
        gcc-c++ \
        clang-tools-extra \
        doxygen
else
    echo "Unsupported distribution"
    exit 1
fi

echo "✓ Dependencies installed"
