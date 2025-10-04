#!/bin/bash
# GutPuncher Build Script (Bash)
# Builds using CMake with Ninja generator

echo "========================================"
echo "  GutPuncher CMake/Ninja Build Script"
echo "========================================"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "[ERROR] CMake not found in PATH!"
    echo "Please install CMake: https://cmake.org/download/"
    echo ""
    exit 1
fi

echo "[INFO] CMake found: $(which cmake)"
cmake --version | head -n 1
echo ""

# Check for Ninja
if command -v ninja &> /dev/null; then
    echo "[INFO] Ninja found: $(which ninja)"
    GENERATOR="-G Ninja"
else
    echo "[WARN] Ninja not found in PATH, will use default generator"
    GENERATOR=""
fi
echo ""

# Check for GCC
if ! command -v gcc &> /dev/null; then
    echo "[ERROR] GCC not found in PATH!"
    echo "Please install MinGW-w64 with UCRT64 and add to PATH"
    echo "Download from: https://www.mingw-w64.org/"
    echo ""
    exit 1
fi

echo "[INFO] GCC found: $(which gcc)"
gcc --version | head -n 1
echo ""

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "[INFO] Cleaning old build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
echo "[INFO] Created build directory: $BUILD_DIR"
echo ""

# Configure
echo "[CMAKE] Configuring project..."
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc \
    $GENERATOR

if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] CMake configuration failed!"
    cd ..
    exit 1
fi

echo ""

# Build
echo "[BUILD] Compiling gutpuncher..."
cmake --build . --config Release

if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] Build failed!"
    cd ..
    exit 1
fi

cd ..

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"

# Show file info
EXE_PATH="build/bin/gutpuncher.exe"
if [ -f "$EXE_PATH" ]; then
    SIZE=$(stat -c%s "$EXE_PATH" 2>/dev/null || stat -f%z "$EXE_PATH" 2>/dev/null)
    SIZE_KB=$((SIZE / 1024))
    echo "Executable: $EXE_PATH"
    echo "File Size: $SIZE_KB KB"
else
    echo "Executable: Not found in expected location"
fi

echo ""
echo "To run: ./build/bin/gutpuncher.exe (as Administrator)"
echo ""
