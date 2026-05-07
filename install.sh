#!/bin/sh
# Hunnu Language Installer - Linux / macOS
# Usage: ./install.sh [PREFIX]
#   PREFIX defaults to /usr/local, or $HUNNU_PREFIX if set
#
# Environment variables:
#   HUNNU_PREFIX  - Installation prefix (overridden by first argument)

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
PREFIX="${1:-${HUNNU_PREFIX:-/usr/local}}"
HUNNU_LIBDIR="$PREFIX/lib/hunnu"
HUNNU_BINDIR="$PREFIX/bin"
BUILD_DIR="$PROJECT_DIR/build"

# Print banner
cat << 'EOF'
 __ __ __  __  __  __  __  __  __
|\  |\  |\  \|\  \|\  \|\  \|\  \|\  \
\ \ \ \ \ \  \ \  \ \  \ \  \ \  \ \  \
 \ \ \ \ \ \  \ \  \ \  \ \  \ \  \ \  \
  \ \_\ \_\ \__\ \__\ \__\ \__\ \__\ \__\
   \|_\|_\|__\|\__\|\__\|\__\|\__\|\__|
        Hunnu Language Installer
EOF
echo ""

# Detect platform
UNAME_S="$(uname -s)"
case "$UNAME_S" in
    Linux*)   PLATFORM="linux" ;;
    Darwin*)  PLATFORM="macos" ;;
    *)        PLATFORM="unix" ;;
esac

# Detect available make tool
if command -v gmake >/dev/null 2>&1; then
    MAKE="gmake"
elif command -v make >/dev/null 2>&1; then
    MAKE="make"
else
    echo "Error: Neither 'make' nor 'gmake' found."
    exit 1
fi

# Detect parallel jobs
if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
elif command -v sysctl >/dev/null 2>&1 && [ "$PLATFORM" = "macos" ]; then
    JOBS="$(sysctl -n hw.ncpu)"
else
    JOBS="4"
fi

echo "Platform:  $PLATFORM"
echo "Prefix:    $PREFIX"
echo "Build dir: $BUILD_DIR"
echo "Jobs:      $JOBS"
echo ""

# Step 1: Build
echo "[1/3] Configuring and building hunnu..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX"
$MAKE -j"$JOBS"
echo ""

# Step 2: Install binary
echo "[2/3] Installing binary to $HUNNU_BINDIR..."
mkdir -p "$HUNNU_BINDIR"
cp "$BUILD_DIR/hunnu" "$HUNNU_BINDIR/hunnu"
chmod +x "$HUNNU_BINDIR/hunnu"
echo "  -> $HUNNU_BINDIR/hunnu"
echo ""

# Step 3: Install standard library
echo "[3/3] Installing standard library to $HUNNU_LIBDIR..."
mkdir -p "$HUNNU_LIBDIR/stdlib"
cp -r "$PROJECT_DIR/stdlib/"* "$HUNNU_LIBDIR/stdlib/"
echo "  -> $HUNNU_LIBDIR/stdlib/"
echo ""

# Verify installation
echo "Verifying installation..."
if "$HUNNU_BINDIR/hunnu" --version >/dev/null 2>&1; then
    echo "  Version: $("$HUNNU_BINDIR/hunnu" --version)"
else
    echo "  Warning: Binary installed but version check failed."
fi

# Print setup instructions
cat << EOF

Installation complete!

  Binary: $HUNNU_BINDIR/hunnu
  Stdlib: $HUNNU_LIBDIR/stdlib/

To use hunnu from anywhere, add the following to your shell profile
(~/.bashrc, ~/.zshrc, ~/.profile, or equivalent):

  export HUNNU_STDLIB_PATH="$HUNNU_LIBDIR"
  export PATH="\$PATH:$HUNNU_BINDIR"

Quick test:
  hunnu --version
  hunnu run examples/main.hn

EOF
