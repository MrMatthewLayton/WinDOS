#!/bin/bash
# Boot from floppy, use HDD for data (larger files like TTF fonts)
# Floppy = boot + kernel + CWSDPMI
# HDD = demo exe + fonts + assets

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$QEMU_DIR")"
FLOPPY="$QEMU_DIR/boot-floppy.img"
HDD="$QEMU_DIR/data-hdd.img"
FREEDOS_DIR="$QEMU_DIR/freedos"
BUILD_DIR="$PROJECT_DIR/build/bin"

echo "=== WinDOS GUI Demo (Floppy Boot + HDD Data) ==="

# Check if forms.exe exists
if [ ! -f "$BUILD_DIR/forms.exe" ]; then
    echo "Error: forms.exe not found. Run 'make forms_demo' first."
    exit 1
fi

# Create boot floppy (small - just boot essentials)
echo "Creating boot floppy..."
rm -f "$FLOPPY"
dd if=/dev/zero of="$FLOPPY" bs=512 count=2880 2>/dev/null
mformat -i "$FLOPPY" -f 1440 -v BOOTDISK ::

# Copy FreeDOS kernel
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::KERNEL.SYS /tmp/
mcopy -i "$FLOPPY" /tmp/KERNEL.SYS ::

# Copy COMMAND.COM
mcopy -i "$FREEDOS_DIR/144m/x86BOOT.img" ::FREEDOS/BIN/COMMAND.COM /tmp/
mcopy -i "$FLOPPY" /tmp/COMMAND.COM ::

# Copy CWSDPMI and mouse driver
mcopy -i "$FLOPPY" "$FREEDOS_DIR/bin/CWSDPMI.EXE" ::
mcopy -i "$FLOPPY" "$FREEDOS_DIR/bin/CTMOUSE.EXE" ::

# Copy FON font as fallback
if [ -f "$PROJECT_DIR/assets/fonts/fon/MSSANS.fon" ]; then
    mcopy -i "$FLOPPY" "$PROJECT_DIR/assets/fonts/fon/MSSANS.fon" :: 2>/dev/null || true
fi

# CONFIG.SYS
cat > /tmp/config.sys << 'EOF'
DOS=HIGH,UMB
FILES=40
BUFFERS=20
LASTDRIVE=Z
SHELL=COMMAND.COM /E:1024 /P
EOF
mcopy -i "$FLOPPY" /tmp/config.sys ::CONFIG.SYS

# AUTOEXEC.BAT - run from C: drive
cat > /tmp/autoexec.bat << 'EOF'
@ECHO OFF
ECHO Loading mouse driver...
CTMOUSE.EXE
ECHO Starting WinDOS GUI Demo from C: drive...
CWSDPMI.EXE
C:\FORMS.EXE
EOF
sed -i '' 's/$/\r/' /tmp/autoexec.bat 2>/dev/null || true
mcopy -i "$FLOPPY" /tmp/autoexec.bat ::AUTOEXEC.BAT

# Install boot sector
dd if="$FREEDOS_DIR/144m/x86BOOT.img" of="$FLOPPY" bs=512 count=1 conv=notrunc 2>/dev/null

echo "Boot floppy created:"
mdir -i "$FLOPPY" ::

# Create data HDD (plenty of space)
echo ""
echo "Creating data HDD..."
rm -f "$HDD"
dd if=/dev/zero of="$HDD" bs=1M count=8 2>/dev/null
mformat -i "$HDD" -F -v WINDOSDAT ::

# Copy forms demo
mcopy -i "$HDD" "$BUILD_DIR/forms.exe" ::FORMS.EXE

# Copy icon library
if [ -f "$PROJECT_DIR/assets/icons/sysicons.icl" ]; then
    mcopy -i "$HDD" "$PROJECT_DIR/assets/icons/sysicons.icl" ::SYSICONS.ICL
fi

# Copy Tahoma Bold TTF
if [ -f "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ]; then
    mcopy -i "$HDD" "$PROJECT_DIR/assets/fonts/ttf/tahomabd.ttf" ::TAHOMABD.TTF
fi

echo "Data HDD created:"
mdir -i "$HDD" ::

echo ""
echo "Starting QEMU (boot from floppy, data on HDD)..."
echo "Press ESC in the WinDOS window to exit."
echo ""

# Run QEMU with both floppy and HDD
if [[ "$OSTYPE" == "darwin"* ]]; then
    DISPLAY_OPT="-display cocoa"
else
    DISPLAY_OPT="-display gtk,window-close=on"
fi

qemu-system-i386 \
    -drive file="$FLOPPY",format=raw,if=floppy \
    -drive file="$HDD",format=raw,if=ide \
    -boot a \
    -m 64 \
    -cpu 486 \
    -vga std \
    $DISPLAY_OPT \
    -no-reboot \
    2>&1

echo "QEMU exited."
