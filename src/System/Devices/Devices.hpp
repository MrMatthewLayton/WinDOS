#ifndef SYSTEM_DEVICES_HPP
#define SYSTEM_DEVICES_HPP

#include "../Types.hpp"

namespace System { namespace Devices {

/******************************************************************************/
/*    System::Devices::Display                                                */
/******************************************************************************/

class Display {
private:
    static const int PALETTE_SIZE = 256;
    static const int FRAME_MS = 1000 / 60;  // ~16ms per frame at 60fps

    unsigned char _mode;          // Standard VGA mode (0 for VBE modes)
    unsigned char _bitsPerPixel;
    unsigned short _width;
    unsigned short _height;
    unsigned short _vbeMode;      // VBE mode number (0 for standard VGA)
    unsigned int _lfbPhysAddr;    // Physical address of linear framebuffer
    unsigned int _lfbPitch;       // Bytes per scanline

    Display(unsigned char mode, unsigned char bpp, unsigned short width, unsigned short height);
    Display(unsigned short vbeMode, unsigned char bpp, unsigned short width, unsigned short height,
            unsigned int lfbAddr, unsigned int pitch);

    static Display _current;
    static bool _vbeAvailable;
    static bool _vbeChecked;
    static void* _mappedLfb;

    // Palette storage for fade effects (VGA modes)
    static unsigned char _originalPalette[PALETTE_SIZE][3];
    static bool _paletteStashed;

    static void StashPalette();
    static void SetPaletteScale(float scale);

    // VBE 3.0 gamma ramp support
    static const int GAMMA_TABLE_SIZE = 256 * 3;  // 256 entries per R/G/B channel
    static unsigned char _originalGamma[GAMMA_TABLE_SIZE];
    static bool _gammaStashed;
    static bool _gammaSupported;
    static bool _gammaChecked;

    static bool CheckGammaSupport();
    static void StashGamma();
    static void SetGammaScale(float scale);

public:
    Display(const Display& other);
    Display& operator=(const Display& other);

    UInt8 Mode() const { return UInt8(_mode); }
    UInt8 BitsPerPixel() const { return UInt8(_bitsPerPixel); }
    UInt16 Width() const { return UInt16(_width); }
    UInt16 Height() const { return UInt16(_height); }
    UInt16 VbeMode() const { return UInt16(_vbeMode); }
    UInt32 LfbPhysAddress() const { return UInt32(_lfbPhysAddr); }
    UInt32 LfbPitch() const { return UInt32(_lfbPitch); }
    Boolean IsVbeMode() const { return Boolean(_vbeMode != 0); }

    static Display GetCurrent();
    static void SetMode(const Display& display);
    static void SetDefaultMode();
    static void WaitForVSync();

    // Palette fade effects (for smooth transitions)
    // Priority: VBE 3.0 gamma ramp (hardware) > VGA palette (hardware) > pixel fade (software)
    static void FadeIn(Int32 milliseconds);
    static void FadeOut(Int32 milliseconds);

    // VBE support
    static Boolean IsVbeAvailable();
    static Boolean IsGammaSupported();  // VBE 3.0+ gamma ramp available
    static Display DetectVbeMode(UInt16 width, UInt16 height, UInt8 bpp);
    static void* GetMappedLfb() { return _mappedLfb; }

    // Common display modes
    static const Display TextMode;           // Mode 0x03: 80x25 text
    static const Display VGA_320x200x8;      // Mode 0x13: 320x200, 256 colors
    static const Display VGA_640x480x4;      // Mode 0x12: 640x480, 16 colors

    // VBE display modes (detected at runtime)
    static const unsigned short VBE_800x600x32 = 0x115;  // Common mode number
};

/******************************************************************************/
/*    System::Devices::MouseStatus                                            */
/******************************************************************************/

class MouseStatus {
public:
    Int32 x;
    Int32 y;
    Boolean leftButton;
    Boolean rightButton;
    Boolean middleButton;

    MouseStatus()
        : x(0), y(0), leftButton(false), rightButton(false), middleButton(false) {}
    MouseStatus(Int32 x, Int32 y, Boolean left, Boolean right, Boolean middle)
        : x(x), y(y), leftButton(left), rightButton(right), middleButton(middle) {}
};

/******************************************************************************/
/*    System::Devices::Mouse                                                  */
/******************************************************************************/

class Mouse {
private:
    Mouse();

    static bool _initialized;

public:
    static Boolean Initialize();
    static Boolean IsAvailable();

    static void ShowCursor();
    static void HideCursor();

    static MouseStatus GetStatus();
    static Int32 GetX();
    static Int32 GetY();

    static void SetPosition(Int32 x, Int32 y);
    static void SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY);

    // Set mouse sensitivity (mickeys per 8 pixels)
    // Higher values = slower mouse. Default is horizontal=8, vertical=16
    static void SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys);

    static Boolean IsLeftButtonPressed();
    static Boolean IsRightButtonPressed();
};

/******************************************************************************/
/*    System::Devices::KeyboardStatus                                         */
/******************************************************************************/

class KeyboardStatus {
public:
    Boolean shiftPressed;
    Boolean ctrlPressed;
    Boolean altPressed;
    Boolean capsLock;
    Boolean numLock;
    Boolean scrollLock;

    KeyboardStatus()
        : shiftPressed(false), ctrlPressed(false), altPressed(false)
        , capsLock(false), numLock(false), scrollLock(false) {}
};

/******************************************************************************/
/*    System::Devices::Keyboard                                               */
/******************************************************************************/

class Keyboard {
private:
    Keyboard();

public:
    static Boolean IsKeyPressed();
    static Char ReadKey();
    static Char PeekKey();
    static KeyboardStatus GetStatus();
};

}} // namespace System::Devices

#endif // SYSTEM_DEVICES_HPP
