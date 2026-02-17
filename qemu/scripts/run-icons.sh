#!/bin/bash
# Run WinDOS Icon Demo in QEMU with graphical display
# Loads icons from sysicons.icl and displays them in a grid

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
IMAGE="$QEMU_DIR/icon-disk.img"
FREEDOS_DIR="$QEMU_DIR/freedos"
BUILD_DIR="$PROJECT_DIR/build/bin"
ASSETS_DIR="$PROJECT_DIR/assets"

echo "=== Running WinDOS Icon Demo in QEMU ==="

# Check if icons.exe exists
if [ ! -f "$BUILD_DIR/icons.exe" ]; then
    echo "Error: icons.exe not found. Run 'make icon_demo' first."
    exit 1
fi

# Check if icon library exists
if [ ! -f "$ASSETS_DIR/icons/sysicons.icl" ]; then
    echo "Error: sysicons.icl not found in assets/icons/"
    exit 1
fi

# Create icon demo disk image
echo "Creating icon demo disk image..."

# Create a 1.44MB floppy disk image
dd if=/dev/zero of="$IMAGE" bs=512 count=2880 2>/dev/null

# Format as FAT12
mformat -i "$IMAGE" -f 1440 -v ICONDEMO ::

# Extract and copy FreeDOS kernel
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
mcopy -i "$IMAGE" /tmp/KERNEL.SYS ::

# Extract and copy COMMAND.COM
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
mcopy -i "$IMAGE" /tmp/COMMAND.COM ::

# Copy CWSDPMI
mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CWSDPMI.EXE" ::

# Copy mouse driver (CuteMouse)
mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CTMOUSE.EXE" ::

# Copy icon demo
mcopy -i "$IMAGE" "$BUILD_DIR/icons.exe" ::

# Copy icon library
mcopy -i "$IMAGE" "$ASSETS_DIR/icons/sysicons.icl" ::

# Create CONFIG.SYS
cat > /tmp/config.sys << 'EOF'
DOS=HIGH,UMB
FILES=40
BUFFERS=20
SHELL=COMMAND.COM /E:1024 /P
EOF
mcopy -i "$IMAGE" /tmp/config.sys ::CONFIG.SYS

# Create AUTOEXEC.BAT
cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
ECHO Loading mouse driver...
CTMOUSE.EXE
ECHO Starting WinDOS Icon Demo...
CWSDPMI.EXE
icons.exe
EOF
sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || true
mcopy -i "$IMAGE" /tmp/autoexec.bat ::AUTOEXEC.BAT

# Install boot sector
dd if="$FREEDOS_DIR/144m/x86BOOT.img" of="$IMAGE" bs=512 count=1 conv=notrunc 2>/dev/null

echo "Icon demo disk image created."
mdir -i "$IMAGE" ::

echo ""
echo "Starting QEMU with VGA display..."
echo "Press ESC to exit the demo."
echo ""

# Run QEMU with graphical display
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
