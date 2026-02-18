#include "Mouse.hpp"
#include <dos.h>
#include <dpmi.h>

namespace Platform::DOS
{

bool Mouse::Initialize()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0000;  // Initialize mouse
    __dpmi_int(0x33, &regs);
    return regs.x.ax != 0;
}

void Mouse::ShowCursor()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0001;  // Show cursor
    __dpmi_int(0x33, &regs);
}

void Mouse::HideCursor()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0002;  // Hide cursor
    __dpmi_int(0x33, &regs);
}

MouseState Mouse::GetState()
{
    __dpmi_regs regs;
    regs.x.ax = 0x0003;  // Get position and button status
    __dpmi_int(0x33, &regs);

    MouseState state;
    state.x = regs.x.cx;
    state.y = regs.x.dx;
    state.leftButton = (regs.x.bx & 0x01) != 0;
    state.rightButton = (regs.x.bx & 0x02) != 0;
    state.middleButton = (regs.x.bx & 0x04) != 0;
    return state;
}

void Mouse::SetPosition(int x, int y)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0004;  // Set position
    regs.x.cx = x;
    regs.x.dx = y;
    __dpmi_int(0x33, &regs);
}

void Mouse::SetHorizontalBounds(int min, int max)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0007;  // Set horizontal bounds
    regs.x.cx = min;
    regs.x.dx = max;
    __dpmi_int(0x33, &regs);
}

void Mouse::SetVerticalBounds(int min, int max)
{
    __dpmi_regs regs;
    regs.x.ax = 0x0008;  // Set vertical bounds
    regs.x.cx = min;
    regs.x.dx = max;
    __dpmi_int(0x33, &regs);
}

void Mouse::SetSensitivity(int horizontalMickeys, int verticalMickeys)
{
    __dpmi_regs regs;
    regs.x.ax = 0x000F;  // Set mickey/pixel ratio
    regs.x.cx = horizontalMickeys;  // Horizontal mickeys per 8 pixels
    regs.x.dx = verticalMickeys;    // Vertical mickeys per 8 pixels
    __dpmi_int(0x33, &regs);
}

} // namespace Platform::DOS
