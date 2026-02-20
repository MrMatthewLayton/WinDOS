#!/bin/bash
# Run WinDOS GUI demo in QEMU with hard disk image (more space for TTF fonts)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
IMAGE="$QEMU_DIR/gui-hdd.img"
FREEDOS_DIR="$QEMU_DIR/freedos"
BUILD_DIR="$PROJECT_DIR/build/bin"

echo "=== Running WinDOS GUI Demo (HDD) in QEMU ==="

# Check if forms.exe exists
if [ ! -f "$BUILD_DIR/forms.exe" ]; then
    echo "Error: forms.exe not found. Run 'make forms_demo' first."
    exit 1
fi

# Create HDD image if needed
if [ ! -f "$IMAGE" ] || [ "$BUILD_DIR/forms.exe" -nt "$IMAGE" ]; then
    echo "Creating HDD image..."

    # Create a 4MB hard disk image
    dd if=/dev/zero of="$IMAGE" bs=1M count=4 2>/dev/null

    # Format as FAT16
    mformat -i "$IMAGE" -F -v WINDOSHDD ::

    # Extract and copy FreeDOS kernel
    mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
    mcopy -i "$IMAGE" /tmp/KERNEL.SYS ::

    # Extract and copy COMMAND.COM
    mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
    mcopy -o -i "$IMAGE" /tmp/COMMAND.COM ::

    # Copy CWSDPMI
    mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CWSDPMI.EXE" ::

    # Copy mouse driver (CuteMouse)
    mcopy -i "$IMAGE" "$FREEDOS_DIR/bin/CTMOUSE.EXE" ::

    # Copy forms demo
    mcopy -i "$IMAGE" "$BUILD_DIR/forms.exe" ::

    # Copy assets if they exist
    if [ -d "$PROJECT_DIR/assets" ]; then
        for asset in "$PROJECT_DIR/assets"/*; do
            if [ -f "$asset" ]; then
                mcopy -i "$IMAGE" "$asset" :: 2>/dev/null || true
            fi
        done
    fi

    # Copy icon library for cursor
    if [ -f "$PROJECT_DIR/assets/icons/sysicons.icl" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/icons/sysicons.icl" :: 2>/dev/null || true
    fi

    # Copy font files (FON)
    if [ -d "$PROJECT_DIR/assets/fonts/fon" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/fonts/fon/MSSANS.fon" :: 2>/dev/null || true
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/fonts/fon/FIXEDSYS.fon" :: 2>/dev/null || true
    fi

    # Copy font files (TTF) - HDD has enough space
    if [ -f "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ::TAHOMABD.TTF 2>/dev/null || true
    fi

    # Create CONFIG.SYS
    cat > /tmp/config.sys << 'EOF'
DOS=HIGH,UMB
FILES=40
BUFFERS=20
SHELL=COMMAND.COM /E:1024 /P
EOF
    mcopy -i "$IMAGE" /tmp/config.sys ::CONFIG.SYS

    # Create AUTOEXEC.BAT for GUI
    cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
ECHO Loading mouse driver...
CTMOUSE.EXE
ECHO Starting WinDOS GUI Demo...
CWSDPMI.EXE
forms.exe
EOF
    sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || true
    mcopy -i "$IMAGE" /tmp/autoexec.bat ::AUTOEXEC.BAT

    echo "HDD image created."
    mdir -i "$IMAGE" ::
fi

echo ""
echo "Starting QEMU with HDD..."
echo "Press ESC in the WinDOS window to exit, then close QEMU window."
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
    -m 64 \
    -cpu 486 \
    -vga std \
    $DISPLAY_OPT \
    -no-reboot \
    2>&1

echo "QEMU exited."
