#!/bin/bash


# Compile program
g++ -Wall -Wextra -O2 -std=c++23 mastermind.cpp -o mastermind || {
  echo "❌ Compilation failed"
  exit 1
}

echo "✅ Compilation successful"
echo

# Initialize counters
PASS=0
FAIL=0

# Function to run a test
run_test() {
  set +e   # let failed commands continue instead of killing the script
  local name=$1
  local input=$2
  local expected_output=$3
  local expected_exit=$4

  echo "--- Running test: $name ---"

  ./mastermind 5 4 0 1 2 3 < "$input" > "tmp.out" 2>&1

  exit_code=$?

  # Check exit code if provided
  if [ -n "$expected_exit" ]; then
    if [ "$exit_code" -ne "$expected_exit" ]; then
      echo "❌ Wrong exit code: got $exit_code, expected $expected_exit"
      ((FAIL++))
      return
    fi
  fi

  # Compare output if expected file exists
  if [ -f "$expected_output" ]; then
    if diff --color=auto -u "$expected_output" "tmp.out" > tmp.diff; then
      echo "✅ Output OK"
      ((PASS++))
    else
      echo "❌ Output mismatch:"
      cat tmp.diff
      ((FAIL++))
    fi

  else
    echo "⚠️ No expected output file, exit code only checked"
  fi

  echo
}

# -------------------------------
# Define your test cases below
# -------------------------------

# Valid test: codemaker mode, correct inputs
run_test "Codemaker valid input" "tests/test_1.in" "tests/test_1.out" 1

# Error test: incomplete input (EOF mid-query)
run_test "EOF mid query error" "tests/test_2.in" "tests/test_2.out" 0

# Error test: invalid arguments
echo "Testing invalid args..."
./mastermind 6 4 999 999 999 < /dev/null > tmp.out 2> tmp.err
if [ $? -ne 1 ]; then
  echo "❌ Expected exit code 1 on invalid args"
  ((FAIL++))
else
  echo "✅ Correctly handled invalid args"
  ((PASS++))
fi

# Summary
echo
echo "✅ PASSED: $PASS"
echo "❌ FAILED: $FAIL"

# Clean up
rm -f tmp.out tmp.err
