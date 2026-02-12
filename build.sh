#!/bin/bash

# IRC Client Build Script

set -e  # Exit on error

echo "==============================================="
echo "  IRC Client Build Script"
echo "==============================================="
echo ""

# Check if Qt is installed
if ! command -v qmake &> /dev/null && ! command -v qmake6 &> /dev/null; then
    echo "❌ Qt not found! Please install Qt development packages:"
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt install qtbase5-dev"
    echo ""
    echo "Fedora:"
    echo "  sudo dnf install qt5-qtbase-devel"
    echo ""
    echo "macOS:"
    echo "  brew install qt@5"
    echo ""
    exit 1
fi

# Create build directory
echo "📁 Creating build directory..."
mkdir -p build
cd build

# Run CMake
echo "🔧 Running CMake..."
cmake ..

# Build
echo "🔨 Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# Check if build succeeded
if [ -f "IRCClient" ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "To run the application:"
    echo "  cd build"
    echo "  ./IRCClient"
    echo ""
    echo "Or simply run:"
    echo "  ./build/IRCClient"
    echo ""
else
    echo ""
    echo "❌ Build failed!"
    exit 1
fi
