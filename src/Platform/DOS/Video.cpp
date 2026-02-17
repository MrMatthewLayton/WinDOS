#include "Video.hpp"
#include <dos.h>
#include <go32.h>
#include <sys/farptr.h>
#include <dpmi.h>

namespace Platform { namespace DOS {

void Video::SetCursorPosition(int row, int col) {
    __dpmi_regs regs;
    regs.h.ah = 0x02;       // Set cursor position
    regs.h.bh = 0;          // Page number
    regs.h.dh = row;        // Row
    regs.h.dl = col;        // Column
    __dpmi_int(0x10, &regs);
}

void Video::GetCursorPosition(int& row, int& col) {
    __dpmi_regs regs;
    regs.h.ah = 0x03;       // Get cursor position
    regs.h.bh = 0;          // Page number
    __dpmi_int(0x10, &regs);
    row = regs.h.dh;
    col = regs.h.dl;
}

void Video::WriteChar(char c, unsigned char attr) {
    __dpmi_regs regs;
    regs.h.ah = 0x09;       // Write character and attribute
    regs.h.al = c;          // Character
    regs.h.bh = 0;          // Page number
    regs.h.bl = attr;       // Attribute
    regs.x.cx = 1;          // Count
    __dpmi_int(0x10, &regs);
}

void Video::WriteCharAtCursor(char c) {
    __dpmi_regs regs;
    regs.h.ah = 0x0E;       // Teletype output
    regs.h.al = c;          // Character
    regs.h.bh = 0;          // Page number
    __dpmi_int(0x10, &regs);
}

void Video::SetVideoMode(int mode) {
    __dpmi_regs regs;
    regs.h.ah = 0x00;       // Set video mode
    regs.h.al = mode;       // Mode number
    __dpmi_int(0x10, &regs);
}

void Video::ScrollUp(int lines, unsigned char attr, int top, int left, int bottom, int right) {
    __dpmi_regs regs;
    regs.h.ah = 0x06;       // Scroll up
    regs.h.al = lines;      // Lines to scroll (0 = clear)
    regs.h.bh = attr;       // Attribute for blank lines
    regs.h.ch = top;        // Top row
    regs.h.cl = left;       // Left column
    regs.h.dh = bottom;     // Bottom row
    regs.h.dl = right;      // Right column
    __dpmi_int(0x10, &regs);
}

void Video::GetScreenSize(int& rows, int& cols) {
    // Read from BIOS data area
    // 0x40:0x4A = number of columns
    // 0x40:0x84 = number of rows - 1
    cols = _farpeekb(_dos_ds, 0x44A);
    rows = _farpeekb(_dos_ds, 0x484) + 1;

    // Default to 80x25 if values seem wrong
    if (cols == 0) cols = 80;
    if (rows == 0) rows = 25;
}

void Video::ClearScreen(unsigned char attr) {
    int rows, cols;
    GetScreenSize(rows, cols);
    ScrollUp(0, attr, 0, 0, rows - 1, cols - 1);
    SetCursorPosition(0, 0);
}

}} // namespace Platform::DOS
