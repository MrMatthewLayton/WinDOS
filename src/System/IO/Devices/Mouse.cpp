#include "Mouse.hpp"
#include <dpmi.h>

namespace System::IO::Devices
{
    Boolean Mouse::isInitialized = false;
    Boolean Mouse::isAvailable = false;

    Boolean Mouse::Initialize()
    {
        // INT 33h, AX=0000h - Initialize mouse
        __dpmi_regs regs;
        regs.x.ax = 0x0000;
        __dpmi_int(0x33, &regs);
        const bool result = regs.x.ax != 0;

        return isInitialized = isAvailable = result;
    }

    Boolean Mouse::IsAvailable()
    {
        return isInitialized;
    }

    void Mouse::ShowCursor()
    {
        if (!isInitialized)
            return;

        // INT 33h, AX=0001h - Show cursor
        __dpmi_regs regs;
        regs.x.ax = 0x0001;
        __dpmi_int(0x33, &regs);
    }

    void Mouse::HideCursor()
    {
        if (!isInitialized)
            return;

        // INT 33h, AX=0002h - Hide cursor
        __dpmi_regs regs;
        regs.x.ax = 0x0002;
        __dpmi_int(0x33, &regs);
    }

    MouseStatus Mouse::GetStatus()
    {
        if (!isInitialized)
            return {};

        // INT 33h, AX=0003h - Get position and button status
        __dpmi_regs regs;
        regs.x.ax = 0x0003;
        __dpmi_int(0x33, &regs);

        return {
            regs.x.cx,
            regs.x.dx,
            (regs.x.bx & 0x01) != 0,
            (regs.x.bx & 0x02) != 0,
            (regs.x.bx & 0x04) != 0
        };
    }

    Int32 Mouse::GetX()
    {
        return GetStatus().x;
    }

    Int32 Mouse::GetY()
    {
        return GetStatus().y;
    }

    void Mouse::SetPosition(const Int32 x, const Int32 y)
    {
        if (!isInitialized)
            return;

        // INT 33h, AX=0004h - Set position
        __dpmi_regs regs;
        regs.x.ax = 0x0004;
        regs.x.cx = x;
        regs.x.dx = y;
        __dpmi_int(0x33, &regs);
    }

    void Mouse::SetBounds(const Int32 minX, const Int32 minY, const Int32 maxX, const Int32 maxY)
    {
        if (!isInitialized)
            return;

        // INT 33h, AX=0007h - Set horizontal bounds
        __dpmi_regs h_regs;
        h_regs.x.ax = 0x0007;
        h_regs.x.cx = minX;
        h_regs.x.dx = maxX;
        __dpmi_int(0x33, &h_regs);

        // INT 33h, AX=0008h - Set vertical bounds
        __dpmi_regs v_regs;
        v_regs.x.ax = 0x0008;
        v_regs.x.cx = minY;
        v_regs.x.dx = maxY;
        __dpmi_int(0x33, &v_regs);
    }

    void Mouse::SetSensitivity(const Int32 xMickeys, const Int32 yMickeys)
    {
        if (!isInitialized)
            return;

        // INT 33h, AX=000Fh - Set mickey/pixel ratio
        __dpmi_regs regs;
        regs.x.ax = 0x000F;
        regs.x.cx = xMickeys; // Horizontal mickeys per 8 pixels
        regs.x.dx = yMickeys; // Vertical mickeys per 8 pixels
        __dpmi_int(0x33, &regs);
    }

    Boolean Mouse::IsLeftButtonPressed()
    {
        return GetStatus().leftButton;
    }

    Boolean Mouse::IsRightButtonPressed()
    {
        return GetStatus().rightButton;
    }

    Boolean Mouse::IsMiddleButtonPressed()
    {
        return GetStatus().middleButton;
    }
} // namespace System::IO::Devices
