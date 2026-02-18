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

    # Copy working FreeDOS boot disk and remove installer files
    cp "$FREEDOS_DIR/144m/x86BOOT.img" "$IMAGE"
    mdel -i "$IMAGE" ::FDAUTO.BAT ::SETUP.BAT ::FDCONFIG.SYS 2>/dev/null || true
    mdeltree -i "$IMAGE" ::FREEDOS 2>/dev/null || true

    # Extract and copy COMMAND.COM to root
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

    # Copy test bitmap (skip if we need space for TTF)
    # if [ -f "$PROJECT_DIR/assets/test.bmp" ]; then
    #     mcopy -i "$IMAGE" "$PROJECT_DIR/assets/test.bmp" :: 2>/dev/null || true
    # fi

    # Copy icon library for cursor
    if [ -f "$PROJECT_DIR/assets/icons/sysicons.icl" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/icons/sysicons.icl" :: 2>/dev/null || true
    fi

    # Copy font files (FON)
    if [ -d "$PROJECT_DIR/assets/fonts/fon" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/fonts/fon/MSSANS.fon" :: 2>/dev/null || true
    fi

    # Copy font files (TTF) - Tahoma Bold for window titles
    if [ -f "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ]; then
        mcopy -i "$IMAGE" "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ::TAHOMABD.TTF 2>/dev/null || true
    fi

    # Create FDCONFIG.SYS (FreeDOS configuration file)
    cat > /tmp/fdconfig.sys << 'EOF'
DOS=HIGH,UMB
FILES=40
BUFFERS=20
SHELL=A:\COMMAND.COM /E:1024 /P
EOF
    sed -i '' 's/$/\r/' /tmp/fdconfig.sys 2>/dev/null || true
    mcopy -i "$IMAGE" /tmp/fdconfig.sys ::FDCONFIG.SYS

    # Create FDAUTO.BAT for GUI (FreeDOS autoexec file)
    cat > /tmp/fdauto.bat << 'EOF'
@ECHO OFF
ECHO Loading mouse driver...
CTMOUSE
ECHO Starting WinDOS GUI Demo...
forms
EOF
    sed -i '' 's/$/\r/' /tmp/fdauto.bat 2>/dev/null || true
    mcopy -i "$IMAGE" /tmp/fdauto.bat ::FDAUTO.BAT

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
