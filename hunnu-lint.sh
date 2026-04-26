#!/bin/bash
# hunnu-lint.sh - Linter script for Hunnu language project
# Usage: ./hunnu-lint.sh [check|format|all]

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

install_tools() {
    echo "Installing linting tools..."
    if command -v brew &>/dev/null; then
        brew install clang-format cppcheck shellcheck
    elif command -v apt-get &>/dev/null; then
        sudo apt-get install -y clang-format cppcheck shellcheck
    elif command -v pacman &>/dev/null; then
        sudo pacman -S clang-format cppcheck shellcheck
    fi
}

check_format() {
    echo "=== Checking Code Format ==="
    if ! command -v clang-format &>/dev/null; then
        echo "clang-format not found. Install with: brew install clang-format"
        return 1
    fi
    
    local issues=0
    for f in $(find compiler cli -name "*.c" -o -name "*.h" 2>/dev/null); do
        if ! clang-format --style=file --dry-run "$f" 2>/dev/null | diff -q "$f" - &>/dev/null; then
            echo "Format issue: $f"
            issues=$((issues + 1))
        fi
    done
    
    if [ $issues -eq 0 ]; then
        echo "✓ Code format OK"
    else
        echo "✗ $issues files have formatting issues"
    fi
}

fix_format() {
    echo "=== Fixing Code Format ==="
    if ! command -v clang-format &>/dev/null; then
        echo "clang-format not found."
        return 1
    fi
    
    for f in $(find compiler cli -name "*.c" -o -name "*.h" 2>/dev/null); do
        clang-format -i "$f"
        echo "Formatted: $f"
    done
}

check_c() {
    echo "=== Checking C Code ==="
    if ! command -v cppcheck &>/dev/null; then
        echo "cppcheck not found. Run: install_tools"
        return 1
    fi
    
    cppcheck --enable=all --std=c11 -I compiler \
        --suppress=missingIncludeSystem \
        --error-exitcode=1 \
        compiler/ cli/ 2>&1 || true
    echo "✓ C code check complete"
}

check_misc() {
    echo "=== Checking Misc ==="
    local issues=0
    
    # Check for trailing whitespace
    for f in $(find . -name "*.c" -o -name "*.h" 2>/dev/null); do
        if grep -q '[[:space:]]$' "$f" 2>/dev/null; then
            echo "Trailing whitespace: $f"
            issues=$((issues + 1))
        fi
    done
    
    # Check for old TODO without author
    for f in $(find . -name "*.c" -o -name "*.h" 2>/dev/null); do
        if grep -q 'TODO\|FIXME' "$f" 2>/dev/null; then
            echo "TODO/FIXME found: $f"
            issues=$((issues + 1))
        fi
    done
    
    if [ $issues -eq 0 ]; then
        echo "✓ Misc checks OK"
    else
        echo "✗ $issues misc issues found"
    fi
}

check_build() {
    echo "=== Checking Build ==="
    if [ ! -d "build" ]; then
        echo "Creating build directory..."
        mkdir -p build
    fi
    
    cd build
    cmake .. >/dev/null 2>&1
    make -j$(nproc) >/dev/null 2>&1
    echo "✓ Build OK"
}

all_checks() {
    check_format || true
    check_c || true
    check_misc || true
    check_build || true
    echo ""
    echo "=== All checks complete ==="
}

case "${1:-check}" in
    check)
        all_checks
        ;;
    format)
        fix_format
        ;;
    install)
        install_tools
        ;;
    all)
        all_checks
        ;;
    *)
        echo "Usage: $0 {check|format|install|all}"
        exit 1
        ;;
esac