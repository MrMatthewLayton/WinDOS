#!/bin/bash
# Run Hatch Pattern demo in QEMU
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
IMAGE="$QEMU_DIR/hatch-disk.img"
FREEDOS_DIR="$QEMU_DIR/freedos"
BUILD_DIR="$PROJECT_DIR/build/bin"

echo "=== Running Hatch Pattern Demo in QEMU ==="

if [ ! -f "$BUILD_DIR/hatch.exe" ]; then
    echo "Error: hatch.exe not found. Run 'make hatch_demo' first."
    exit 1
fi

# Always recreate disk image
echo "Creating disk image..."
dd if=/dev/zero of="$IMAGE" bs=512 count=2880 2>/dev/null
mformat -i "$IMAGE" -f 1440 -v HATCHDEMO ::

# Copy FreeDOS kernel and command
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
mcopy -i "$IMAGE" /tmp/KERNEL.SYS ::
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
mcopy -o -i "$IMAGE" /tmp/COMMAND.COM ::

# Copy CWSDPMI and demo
mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CWSDPMI.EXE" ::
mcopy -i "$IMAGE" "$BUILD_DIR/hatch.exe" ::

# Create AUTOEXEC.BAT
cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
CWSDPMI.EXE
hatch.exe
EOF
sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || true
mcopy -i "$IMAGE" /tmp/autoexec.bat ::AUTOEXEC.BAT

# Install boot sector
dd if="$FREEDOS_DIR/144m/x86BOOT.img" of="$IMAGE" bs=512 count=1 conv=notrunc 2>/dev/null

echo "Starting QEMU..."
qemu-system-i386 \
    -drive file="$IMAGE",format=raw,if=floppy \
    -boot a \
    -m 64 \
    -cpu 486 \
    -vga std \
    -display default \
    -no-reboot \
    2>&1

echo "QEMU exited."
