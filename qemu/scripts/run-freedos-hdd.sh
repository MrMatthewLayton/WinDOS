#!/bin/bash
# Minimal FreeDOS HDD boot test
# Creates a bootable hard disk with just FreeDOS kernel

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
IMAGE="$QEMU_DIR/freedos-hdd.img"
FREEDOS_DIR="$QEMU_DIR/freedos"

echo "=== Minimal FreeDOS HDD Boot Test ==="

# Always recreate the image
rm -f "$IMAGE"

echo "Creating minimal HDD image..."

# Create a 4MB hard disk image
dd if=/dev/zero of="$IMAGE" bs=1M count=4 2>/dev/null

# Format as FAT16 with boot sector
# Use -F flag for FAT16, -h for hidden sectors (for HDD)
mformat -i "$IMAGE" -F -v FREEDOS -h 16 -s 32 ::

# Extract FreeDOS kernel from boot floppy
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
mcopy -i "$IMAGE" /tmp/KERNEL.SYS ::

# Extract COMMAND.COM
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
mcopy -o -i "$IMAGE" /tmp/COMMAND.COM ::

# Create minimal CONFIG.SYS
cat > /tmp/config.sys << 'EOF'
DOS=HIGH
FILES=20
BUFFERS=10
SHELL=COMMAND.COM /P
EOF
mcopy -i "$IMAGE" /tmp/config.sys ::CONFIG.SYS

# Create minimal AUTOEXEC.BAT
cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
ECHO.
ECHO FreeDOS HDD boot successful!
ECHO.
DIR
EOF
sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || true
mcopy -i "$IMAGE" /tmp/autoexec.bat ::AUTOEXEC.BAT

echo "HDD image created:"
mdir -i "$IMAGE" ::

echo ""
echo "Starting QEMU..."
echo ""

# Run QEMU with HDD
if [[ "$OSTYPE" == "darwin"* ]]; then
    DISPLAY_OPT="-display cocoa"
else
    DISPLAY_OPT="-display gtk,window-close=on"
fi

qemu-system-i386 \
    -drive file="$IMAGE",format=raw,if=ide \
    -boot c \
    -m 16 \
    -cpu 486 \
    $DISPLAY_OPT \
    -no-reboot \
    2>&1

echo "QEMU exited."
