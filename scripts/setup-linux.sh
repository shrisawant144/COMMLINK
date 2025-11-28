#!/bin/bash
set -e

echo "Setting up CommLink dependencies for Linux..."

if [ -f /etc/debian_version ]; then
    echo "Detected Debian/Ubuntu"
    sudo apt update
    sudo apt install -y build-essential cmake qt5-default libqt5sql5-sqlite
elif [ -f /etc/redhat-release ]; then
    echo "Detected RHEL/Fedora"
    sudo dnf install -y gcc-c++ cmake qt5-qtbase-devel qt5-qtbase-sqlite
elif [ -f /etc/arch-release ]; then
    echo "Detected Arch Linux"
    sudo pacman -S --needed base-devel cmake qt5-base
else
    echo "Unsupported distribution. Install manually:"
    echo "- CMake 3.16+"
    echo "- GCC 7+ or Clang 5+"
    echo "- Qt5 (Core, Widgets, Sql, Network)"
    exit 1
fi

echo "Dependencies installed successfully!"
