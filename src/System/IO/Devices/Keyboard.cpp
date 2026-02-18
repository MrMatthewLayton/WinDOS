#include "Keyboard.hpp"
#include <dos.h>
#include <dpmi.h>
#include <conio.h>

namespace System::IO::Devices
{

/******************************************************************************/
/*    Keyboard private BIOS methods - kept for PeekKey() use                  */
/******************************************************************************/

unsigned short Keyboard::BiosPeekKey()
{
    __dpmi_regs regs;
    regs.h.ah = 0x01;       // Check for key (peek)
    __dpmi_int(0x16, &regs);
    if (regs.x.flags & 0x40)
    {
        return 0;           // No key available
    }
    return regs.x.ax;       // AH = scan code, AL = ASCII
}

/******************************************************************************/
/*    Keyboard public methods                                                 */
/******************************************************************************/

Boolean Keyboard::IsKeyPressed()
{
    return Boolean(kbhit() != 0);
}

Char Keyboard::ReadKey()
{
    return Char(getch());
}

void Keyboard::ReadKey(UInt8& scanCode, Char& character)
{
    // INT 16h, AH=00h - Read character
    __dpmi_regs regs;
    regs.h.ah = 0x00;       // Read character
    __dpmi_int(0x16, &regs);
    unsigned short key = regs.x.ax;  // AH = scan code, AL = ASCII

    scanCode = UInt8((key >> 8) & 0xFF);   // High byte: scan code
    character = Char(key & 0xFF);          // Low byte: ASCII
}

Char Keyboard::PeekKey()
{
    if (!kbhit()) return Char('\0');

    unsigned short key = BiosPeekKey();
    return Char(key & 0xFF);  // Low byte: ASCII
}

Boolean Keyboard::PeekKey(UInt8& scanCode, Char& character)
{
    unsigned short key = BiosPeekKey();
    if (key == 0)
    {
        return Boolean(false);
    }

    scanCode = UInt8((key >> 8) & 0xFF);   // High byte: scan code
    character = Char(key & 0xFF);          // Low byte: ASCII
    return Boolean(true);
}

KeyboardStatus Keyboard::GetStatus()
{
    // INT 16h, AH=02h - Get shift flags
    __dpmi_regs regs;
    regs.h.ah = 0x02;  // Get shift flags
    __dpmi_int(0x16, &regs);
    unsigned char flags = regs.h.al;

    KeyboardStatus status;
    status.shiftPressed = Boolean((flags & 0x03) != 0);  // Left or right shift
    status.ctrlPressed = Boolean((flags & 0x04) != 0);   // Ctrl
    status.altPressed = Boolean((flags & 0x08) != 0);    // Alt
    status.scrollLock = Boolean((flags & 0x10) != 0);    // Scroll Lock
    status.numLock = Boolean((flags & 0x20) != 0);       // Num Lock
    status.capsLock = Boolean((flags & 0x40) != 0);      // Caps Lock

    return status;
}

} // namespace System::IO::Devices
