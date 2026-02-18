#include "Mouse.hpp"
#include <dos.h>
#include <dpmi.h>

namespace System::IO::Devices
{

/******************************************************************************/
/*    Mouse static members                                                   */
/******************************************************************************/

Boolean Mouse::_initialized = Boolean(false);
Boolean Mouse::_available = Boolean(false);

/******************************************************************************/
/*    Mouse private BIOS methods - kept for SetBounds() use                   */
/******************************************************************************/

void Mouse::BiosSetHorizontalBounds(int min, int max)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0007;  // Set horizontal bounds
    regs.x.cx = min;
    regs.x.dx = max;
    __dpmi_int(0x33, &regs);
}

void Mouse::BiosSetVerticalBounds(int min, int max)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0008;  // Set vertical bounds
    regs.x.cx = min;
    regs.x.dx = max;
    __dpmi_int(0x33, &regs);
}

/******************************************************************************/
/*    Mouse public methods                                                    */
/******************************************************************************/

Boolean Mouse::Initialize()
{
    // INT 33h, AX=0000h - Initialize mouse
    __dpmi_regs regs;
    regs.x.ax = 0x0000;  // Initialize mouse
    __dpmi_int(0x33, &regs);
    bool result = (regs.x.ax != 0);

    _available = Boolean(result);
    _initialized = _available;
    return _initialized;
}

Boolean Mouse::IsAvailable()
{
    return _initialized;
}

void Mouse::ShowCursor()
{
    if (static_cast<bool>(_initialized))
    {
        // INT 33h, AX=0001h - Show cursor
        __dpmi_regs regs;
        regs.x.ax = 0x0001;  // Show cursor
        __dpmi_int(0x33, &regs);
    }
}

void Mouse::HideCursor()
{
    if (static_cast<bool>(_initialized))
    {
        // INT 33h, AX=0002h - Hide cursor
        __dpmi_regs regs;
        regs.x.ax = 0x0002;  // Hide cursor
        __dpmi_int(0x33, &regs);
    }
}

MouseStatus Mouse::GetStatus()
{
    if (!static_cast<bool>(_initialized))
    {
        return MouseStatus();
    }

    // INT 33h, AX=0003h - Get position and button status
    __dpmi_regs regs;
    regs.x.ax = 0x0003;  // Get position and button status
    __dpmi_int(0x33, &regs);

    int x = regs.x.cx;
    int y = regs.x.dx;
    int buttons = regs.x.bx;

    return MouseStatus(
        Int32(x),
        Int32(y),
        Boolean((buttons & 0x01) != 0),  // Left button
        Boolean((buttons & 0x02) != 0),  // Right button
        Boolean((buttons & 0x04) != 0)   // Middle button
    );
}

Int32 Mouse::GetX()
{
    return GetStatus().x;
}

Int32 Mouse::GetY()
{
    return GetStatus().y;
}

void Mouse::SetPosition(Int32 x, Int32 y)
{
    if (static_cast<bool>(_initialized))
    {
        // INT 33h, AX=0004h - Set position
        __dpmi_regs regs;
        regs.x.ax = 0x0004;  // Set position
        regs.x.cx = static_cast<int>(x);
        regs.x.dx = static_cast<int>(y);
        __dpmi_int(0x33, &regs);
    }
}

void Mouse::SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY)
{
    if (static_cast<bool>(_initialized))
    {
        BiosSetHorizontalBounds(static_cast<int>(minX), static_cast<int>(maxX));
        BiosSetVerticalBounds(static_cast<int>(minY), static_cast<int>(maxY));
    }
}

void Mouse::SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys)
{
    if (static_cast<bool>(_initialized))
    {
        // INT 33h, AX=000Fh - Set mickey/pixel ratio
        __dpmi_regs regs;
        regs.x.ax = 0x000F;  // Set mickey/pixel ratio
        regs.x.cx = static_cast<int>(horizontalMickeys);  // Horizontal mickeys per 8 pixels
        regs.x.dx = static_cast<int>(verticalMickeys);    // Vertical mickeys per 8 pixels
        __dpmi_int(0x33, &regs);
    }
}

Boolean Mouse::IsLeftButtonPressed()
{
    return GetStatus().leftButton;
}

Boolean Mouse::IsRightButtonPressed()
{
    return GetStatus().rightButton;
}

} // namespace System::IO::Devices
