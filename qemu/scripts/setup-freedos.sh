#!/bin/bash
# Setup FreeDOS hard disk image for CI/CD testing
# This script creates a bootable FreeDOS image with CWSDPMI

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
QEMU_DIR="$(dirname "$SCRIPT_DIR")"
FREEDOS_DIR="$QEMU_DIR/freedos"
PROJECT_DIR="$(dirname "$QEMU_DIR")"

echo "=== FreeDOS Setup for WinDOS CI/CD ==="
echo "QEMU_DIR: $QEMU_DIR"
echo "FREEDOS_DIR: $FREEDOS_DIR"

cd "$FREEDOS_DIR"

# Create a raw disk image (easier to mount and modify)
if [ ! -f freedos-hd.img ]; then
    echo "Creating 64MB hard disk image..."
    qemu-img create -f raw freedos-hd.img 64M
fi

# We need to install FreeDOS. For automation, we'll use the boot floppy
# and run the installer interactively first, then save the result.

echo ""
echo "=== FreeDOS Installation Required ==="
echo ""
echo "To complete setup, run the following command manually:"
echo ""
echo "  qemu-system-i386 \\"
echo "    -fda $FREEDOS_DIR/144m/x86BOOT.img \\"
echo "    -hda $FREEDOS_DIR/freedos-hd.img \\"
echo "    -boot a \\"
echo "    -m 64"
echo ""
echo "In the FreeDOS installer:"
echo "  1. Select 'English' language"
echo "  2. Select 'Yes - Continue with FreeDOS install'"
echo "  3. Select 'Yes - Partition drive C:'"
echo "  4. Reboot when prompted"
echo "  5. Re-run the installer and select 'Yes - Please format drive C:'"
echo "  6. Select 'Plain DOS system' for minimal install"
echo "  7. Complete installation and reboot"
echo ""
echo "After installation, the image will be ready for CI/CD use."
echo ""

# Check if installation is already done
if [ -f "$FREEDOS_DIR/.installed" ]; then
    echo "FreeDOS appears to be installed (found .installed marker)"
    echo "To reinstall, remove: $FREEDOS_DIR/.installed"
fi
