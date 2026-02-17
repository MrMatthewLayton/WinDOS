#!/bin/bash
# Run WinDOS tests in QEMU with FreeDOS
# Captures serial output for CI/CD integration

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
IMAGE="$QEMU_DIR/test-disk.img"
FREEDOS_BOOT="$QEMU_DIR/freedos/144m/x86BOOT.img"
OUTPUT_FILE="$QEMU_DIR/test-output.txt"
TIMEOUT=${TIMEOUT:-60}

echo "=== Running WinDOS Tests in QEMU ==="
echo "Image: $IMAGE"
echo "Timeout: ${TIMEOUT}s"
echo ""

# Check if image exists
if [ ! -f "$IMAGE" ]; then
    echo "Error: Test disk image not found."
    echo "Run: $SCRIPT_DIR/create-test-image.sh"
    exit 1
fi

# Clean previous output
rm -f "$OUTPUT_FILE"

# Run QEMU with serial output
# -nographic: No graphical output
# -serial file: Redirect serial to file
# -monitor none: Disable monitor
# -no-reboot: Exit on reboot/shutdown
echo "Starting QEMU..."

# Use timeout command if available, otherwise use background process
if command -v gtimeout &> /dev/null; then
    TIMEOUT_CMD="gtimeout"
elif command -v timeout &> /dev/null; then
    TIMEOUT_CMD="timeout"
else
    TIMEOUT_CMD=""
fi

# Run QEMU
if [ -n "$TIMEOUT_CMD" ]; then
    $TIMEOUT_CMD "$TIMEOUT" qemu-system-i386 \
        -drive file="$IMAGE",format=raw,if=floppy \
        -boot a \
        -m 64 \
        -cpu 486 \
        -nographic \
        -serial file:"$OUTPUT_FILE" \
        -monitor none \
        -no-reboot \
        2>&1 || true
else
    # Background process with manual timeout
    qemu-system-i386 \
        -drive file="$IMAGE",format=raw,if=floppy \
        -boot a \
        -m 64 \
        -cpu 486 \
        -nographic \
        -serial file:"$OUTPUT_FILE" \
        -monitor none \
        -no-reboot \
        2>&1 &

    QEMU_PID=$!

    # Wait with timeout
    ELAPSED=0
    while kill -0 $QEMU_PID 2>/dev/null && [ $ELAPSED -lt $TIMEOUT ]; do
        sleep 1
        ELAPSED=$((ELAPSED + 1))

        # Check if tests are done
        if [ -f "$OUTPUT_FILE" ] && grep -q "TESTS_DONE" "$OUTPUT_FILE" 2>/dev/null; then
            echo "Tests completed, stopping QEMU..."
            kill $QEMU_PID 2>/dev/null || true
            break
        fi
    done

    # Kill if still running
    kill $QEMU_PID 2>/dev/null || true
    wait $QEMU_PID 2>/dev/null || true
fi

echo ""
echo "=== Test Output ==="
if [ -f "$OUTPUT_FILE" ]; then
    cat "$OUTPUT_FILE"
    echo ""

    # Parse results
    if grep -q "TESTS_DONE" "$OUTPUT_FILE"; then
        echo "=== Tests Completed Successfully ==="

        # Count pass/fail if present (look for [PASS] and [FAIL] markers)
        # Strip ANSI codes and count matches
        CLEAN_FILE=$(sed 's/\x1b\[[0-9;]*m//g' "$OUTPUT_FILE")
        PASSED=$(echo "$CLEAN_FILE" | grep -c "\[PASS\]" || true)
        FAILED=$(echo "$CLEAN_FILE" | grep -c "\[FAIL\]" || true)

        # Default to 0 if empty
        PASSED=${PASSED:-0}
        FAILED=${FAILED:-0}

        echo "Passed: $PASSED"
        echo "Failed: $FAILED"

        if [ "$FAILED" -gt 0 ]; then
            exit 1
        fi
        exit 0
    else
        echo "=== Tests May Have Timed Out ==="
        exit 1
    fi
else
    echo "No output captured"
    exit 1
fi
