#!/bin/bash
# Integration test script for Hunnu C + Rust project
# Tests bytecode round-trip: C compiler -> bytecode -> Rust VM

set -e

HUNNU_BIN="/home/kyuna/Desktop/hunnu-lang/build/hunnu"
TEST_DIR="/tmp/hunnu_integration_tests"
PASSED=0
FAILED=0

mkdir -p "$TEST_DIR"

echo "=== Hunnu C + Rust Integration Tests ==="
echo ""

run_test() {
    local name="$1"
    local source="$2"
    local expected="$3"
    
    echo -n "Test: $name ... "
    
    # Write test file
    local test_file="$TEST_DIR/${name}.hn"
    echo "$source" > "$test_file"
    
    # Run with Rust VM
    local output
    output=$($HUNNU_BIN run "$test_file" --vm-rust 2>&1)
    
    if [ "$output" = "$expected" ]; then
        echo "PASSED"
        ((PASSED++))
    else
        echo "FAILED"
        echo "  Expected: '$expected'"
        echo "  Got:      '$output'"
        ((FAILED++))
    fi
}

# Test 1: Simple arithmetic
run_test "arithmetic" \
    "fn main() { print(2 + 3) }" \
    "5"

# Test 2: String concatenation
run_test "string_concat" \
    "fn main() { print(\"Hello \" + \"World\") }" \
    "Hello World"

# Test 3: Variables
run_test "variables" \
    "fn main() { let x = 10; print(x) }" \
    "10"

# Test 4: Comparison
run_test "comparison" \
    "fn main() { print(5 > 3) }" \
    "true"

# Test 5: If/else
run_test "if_else" \
    "fn main() { if (1 > 0) { print(\"yes\") } else { print(\"no\") } }" \
    "yes"

# Test 6: While loop
run_test "while_loop" \
    "fn main() { let i = 0; while (i < 3) { print(i); i = i + 1 } }" \
    "0"

# Test 7: Arrays
run_test "arrays" \
    "fn main() { let arr = [1, 2, 3]; print(arr[0]) }" \
    "1"

# Test 8: Functions
run_test "functions" \
    "fn add(a, b) { return a + b } fn main() { print(add(2, 3)) }" \
    "5"

echo ""
echo "=== Results ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo ""

if [ $FAILED -gt 0 ]; then
    exit 1
fi

rm -rf "$TEST_DIR"
exit 0
