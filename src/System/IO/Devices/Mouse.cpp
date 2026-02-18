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
/*    Mouse private BIOS methods                                              */
/******************************************************************************/

bool Mouse::BiosInitialize()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0000;  // Initialize mouse
    __dpmi_int(0x33, &regs);
    return regs.x.ax != 0;
}

void Mouse::BiosShowCursor()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0001;  // Show cursor
    __dpmi_int(0x33, &regs);
}

void Mouse::BiosHideCursor()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0002;  // Hide cursor
    __dpmi_int(0x33, &regs);
}

void Mouse::BiosGetState(int& x, int& y, int& buttons)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0003;  // Get position and button status
    __dpmi_int(0x33, &regs);

    x = regs.x.cx;
    y = regs.x.dx;
    buttons = regs.x.bx;
}

void Mouse::BiosSetPosition(int x, int y)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0004;  // Set position
    regs.x.cx = x;
    regs.x.dx = y;
    __dpmi_int(0x33, &regs);
}

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

void Mouse::BiosSetSensitivity(int horizontalMickeys, int verticalMickeys)
{
    __dpmi_regs regs;
    regs.x.ax = 0x000F;  // Set mickey/pixel ratio
    regs.x.cx = horizontalMickeys;  // Horizontal mickeys per 8 pixels
    regs.x.dx = verticalMickeys;    // Vertical mickeys per 8 pixels
    __dpmi_int(0x33, &regs);
}

/******************************************************************************/
/*    Mouse public methods                                                    */
/******************************************************************************/

Boolean Mouse::Initialize()
{
    _available = Boolean(BiosInitialize());
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
        BiosShowCursor();
    }
}

void Mouse::HideCursor()
{
    if (static_cast<bool>(_initialized))
    {
        BiosHideCursor();
    }
}

MouseStatus Mouse::GetStatus()
{
    if (!static_cast<bool>(_initialized))
    {
        return MouseStatus();
    }

    int x, y, buttons;
    BiosGetState(x, y, buttons);

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
        BiosSetPosition(static_cast<int>(x), static_cast<int>(y));
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
        BiosSetSensitivity(
            static_cast<int>(horizontalMickeys),
            static_cast<int>(verticalMickeys)
        );
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
