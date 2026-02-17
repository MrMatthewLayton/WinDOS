#include "Keyboard.hpp"
#include <dos.h>
#include <dpmi.h>

namespace Platform { namespace DOS {

char Keyboard::ReadChar() {
    __dpmi_regs regs;
    regs.h.ah = 0x00;       // Read character
    __dpmi_int(0x16, &regs);
    return regs.h.al;       // ASCII code
}

bool Keyboard::IsKeyAvailable() {
    __dpmi_regs regs;
    regs.h.ah = 0x01;       // Check for key
    __dpmi_int(0x16, &regs);
    // Zero flag is set if no key available
    return !(regs.x.flags & 0x40);
}

unsigned short Keyboard::ReadScanCode() {
    __dpmi_regs regs;
    regs.h.ah = 0x00;       // Read character
    __dpmi_int(0x16, &regs);
    return regs.x.ax;       // AH = scan code, AL = ASCII
}

unsigned short Keyboard::PeekKey() {
    __dpmi_regs regs;
    regs.h.ah = 0x01;       // Check for key (peek)
    __dpmi_int(0x16, &regs);
    if (regs.x.flags & 0x40) {
        return 0;           // No key available
    }
    return regs.x.ax;       // AH = scan code, AL = ASCII
}

}} // namespace Platform::DOS
