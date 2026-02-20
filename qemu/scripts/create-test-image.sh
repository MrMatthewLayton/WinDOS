#!/bin/bash
# Create a bootable FreeDOS test image with WinDOS test executables
# This creates a minimal bootable image for CI/CD testing

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
FREEDOS_DIR="$QEMU_DIR/freedos"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
BUILD_DIR="$PROJECT_DIR/build/bin"
IMAGE="$QEMU_DIR/test-disk.img"

echo "=== Creating WinDOS Test Disk Image ==="
echo "Project: $PROJECT_DIR"
echo "Build:   $BUILD_DIR"
echo "Image:   $IMAGE"
echo ""

# Check if test executables exist
if [ ! -f "$BUILD_DIR/test.exe" ]; then
    echo "Error: test.exe not found. Run 'make test' first."
    exit 1
fi

# Create a 1.44MB floppy disk image (standard format, well-supported)
echo "Creating 1.44MB floppy disk image..."
dd if=/dev/zero of="$IMAGE" bs=512 count=2880 2>/dev/null

# Format as FAT12 (standard 1.44MB floppy)
echo "Formatting as FAT12..."
mformat -i "$IMAGE" -f 1440 -v WINDOS ::

echo "Extracting FreeDOS kernel from boot floppy..."
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
echo "Copying kernel to test image..."
mcopy -i "$IMAGE" /tmp/KERNEL.SYS ::

echo "Extracting COMMAND.COM from boot floppy..."
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
echo "Copying COMMAND.COM to test image..."
mcopy -o -i "$IMAGE" /tmp/COMMAND.COM ::

echo "Copying CWSDPMI..."
mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CWSDPMI.EXE" ::

echo "Copying test executables..."
mcopy -i "$IMAGE" "$BUILD_DIR/test.exe" ::

# Copy any other needed files from build
for exe in "$BUILD_DIR"/*.exe; do
    if [ -f "$exe" ]; then
        name=$(basename "$exe")
        if [ "$name" != "test.exe" ]; then
            mcopy -i "$IMAGE" "$exe" :: 2>/dev/null || true
        fi
    fi
done

# Create CONFIG.SYS
echo "Creating CONFIG.SYS..."
cat > /tmp/config.sys << 'EOF'
DOS=HIGH,UMB
FILES=40
BUFFERS=20
SHELL=COMMAND.COM /E:1024 /P
EOF
mcopy -i "$IMAGE" /tmp/config.sys ::CONFIG.SYS

# Create AUTOEXEC.BAT that runs tests and outputs to COM1
echo "Creating AUTOEXEC.BAT..."
cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
ECHO ========================================
ECHO WinDOS Test Suite
ECHO ========================================
ECHO.
CWSDPMI.EXE
test.exe
ECHO.
ECHO ========================================
ECHO Tests Complete
ECHO ========================================
ECHO TESTS_DONE
EOF
# Convert to DOS line endings
sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || unix2dos /tmp/autoexec.bat 2>/dev/null || true
mcopy -i "$IMAGE" /tmp/autoexec.bat ::AUTOEXEC.BAT

# Create boot sector (we need the FreeDOS boot sector)
echo "Installing boot sector..."
# Copy boot sector from FreeDOS boot floppy
dd if="$FREEDOS_DIR/144m/x86BOOT.img" of="$IMAGE" bs=512 count=1 conv=notrunc 2>/dev/null

echo ""
echo "Disk contents:"
mdir -i "$IMAGE" ::

echo ""
echo "=== Test disk image created: $IMAGE ==="
echo ""
echo "To test manually:"
echo "  $SCRIPT_DIR/run-tests.sh"
