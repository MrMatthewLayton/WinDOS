#!/bin/bash
# Run WinDOS GUI demo in QEMU with graphical display
# Opens a window showing the FreeDOS desktop with WinDOS forms

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
IMAGE="$QEMU_DIR/gui-disk.img"
FREEDOS_DIR="$QEMU_DIR/freedos"
BUILD_DIR="$PROJECT_DIR/build/bin"

echo "=== Running WinDOS GUI Demo in QEMU ==="

# Check if forms.exe exists
if [ ! -f "$BUILD_DIR/forms.exe" ]; then
    echo "Error: forms.exe not found. Run 'make forms_demo' first."
    exit 1
fi

# Create GUI disk image if needed
if [ ! -f "$IMAGE" ] || [ "$BUILD_DIR/forms.exe" -nt "$IMAGE" ]; then
    echo "Creating GUI disk image..."

    # Create a 1.44MB floppy disk image
    dd if=/dev/zero of="$IMAGE" bs=512 count=2880 2>/dev/null

    # Format as FAT12
    mformat -i "$IMAGE" -f 1440 -v WINDOSGUI ::

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

    # Copy forms demo
    mcopy -i "$IMAGE" "$BUILD_DIR/forms.exe" ::

    # Copy graphics demo too if it exists
    [ -f "$BUILD_DIR/gfxdemo.exe" ] && mcopy -i "$IMAGE" "$BUILD_DIR/gfxdemo.exe" :: || true

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

    # Install boot sector
    dd if="$FREEDOS_DIR/144m/x86BOOT.img" of="$IMAGE" bs=512 count=1 conv=notrunc 2>/dev/null

    echo "GUI disk image created."
    mdir -i "$IMAGE" ::
fi

echo ""
echo "Starting QEMU with VGA display..."
echo "Press ESC in the WinDOS window to exit, then close QEMU window."
echo ""

# Run QEMU with graphical display
# -display cocoa: Use Cocoa display on macOS with zoom-to-fit for scaling
# -vga std: Standard VGA adapter with VESA BIOS Extensions
# -m 64: 64MB RAM
# -device VGA: Use VGA device with VBE support

# Detect platform and set display options
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS: Use Cocoa at native resolution (800x600)
    DISPLAY_OPT="-display cocoa"

    # Launch QEMU
    qemu-system-i386 \
        -drive file="$IMAGE",format=raw,if=floppy \
        -boot a \
        -m 64 \
        -cpu 486 \
        -vga std \
        $DISPLAY_OPT \
        -no-reboot \
        2>&1
else
    # Linux: Use GTK at native resolution
    DISPLAY_OPT="-display gtk,window-close=on"

    qemu-system-i386 \
        -drive file="$IMAGE",format=raw,if=floppy \
        -boot a \
        -m 64 \
        -cpu 486 \
        -vga std \
        $DISPLAY_OPT \
        -no-reboot \
        2>&1
fi

echo "QEMU exited."
