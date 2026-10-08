#!/bin/bash
set -e

# Prompt for sudo if packages are missing
if ! command -v cmake &> /dev/null || ! dpkg -s qt6-base-dev &> /dev/null; then
    echo "Installing required dependencies..."
    sudo apt-get update
    sudo apt-get install -y cmake build-essential qt6-base-dev qt6-tools-dev qt6-tools-dev-tools qt6-multimedia-dev libgl1-mesa-dev
fi

echo "Building Mousy..."
mkdir -p build
cd build
cmake ..
cmake --build .

echo "Build successful! You can run the app with:"
echo "./build/Mousy"
