#include "Devices.hpp"
#include "../Drawing/Drawing.hpp"
#include "../../Platform/DOS/Graphics.hpp"
#include "../../Platform/DOS/Mouse.hpp"
#include "../../Platform/DOS/Keyboard.hpp"
#include <dos.h>
#include <dpmi.h>
#include <conio.h>
#include <pc.h>
#include <cstring>
#include <cstdlib>

namespace System { namespace Devices {

/******************************************************************************/
/*    Display implementation                                                  */
/******************************************************************************/

Display Display::_current = Display::TextMode;
Boolean Display::_vbeAvailable = Boolean(false);
Boolean Display::_vbeChecked = Boolean(false);
void* Display::_mappedLfb = nullptr;
unsigned char Display::_originalPalette[PALETTE_SIZE][3] = {{0}};
Boolean Display::_paletteStashed = Boolean(false);

// VBE 3.0 gamma ramp support
unsigned char Display::_originalGamma[GAMMA_TABLE_SIZE] = {0};
Boolean Display::_gammaStashed = Boolean(false);
Boolean Display::_gammaSupported = Boolean(false);
Boolean Display::_gammaChecked = Boolean(false);

// Static VbeSurface for tracking current VBE mode state
static Platform::DOS::VbeSurface g_vbeSurface = {0, 0, 0, 0, 0, 0, 0, false};

Display::Display(unsigned char mode, unsigned char bpp, unsigned short width, unsigned short height)
    : _mode(mode), _bitsPerPixel(bpp), _width(width), _height(height)
    , _vbeMode(0), _lfbPhysAddr(0), _lfbPitch(0) {}

Display::Display(unsigned short vbeMode, unsigned char bpp, unsigned short width, unsigned short height,
                 unsigned int lfbAddr, unsigned int pitch)
    : _mode(0), _bitsPerPixel(bpp), _width(width), _height(height)
    , _vbeMode(vbeMode), _lfbPhysAddr(lfbAddr), _lfbPitch(pitch) {}

Display::Display(const Display& other)
    : _mode(other._mode), _bitsPerPixel(other._bitsPerPixel)
    , _width(other._width), _height(other._height)
    , _vbeMode(other._vbeMode), _lfbPhysAddr(other._lfbPhysAddr)
    , _lfbPitch(other._lfbPitch) {}

Display& Display::operator=(const Display& other) {
    _mode = other._mode;
    _bitsPerPixel = other._bitsPerPixel;
    _width = other._width;
    _height = other._height;
    _vbeMode = other._vbeMode;
    _lfbPhysAddr = other._lfbPhysAddr;
    _lfbPitch = other._lfbPitch;
    return *this;
}

Display Display::GetCurrent() {
    return _current;
}

void Display::SetMode(const Display& display) {
    // Cleanup previous VBE surface if any
    if (g_vbeSurface.valid) {
        Platform::DOS::Graphics::CleanupVBE(&g_vbeSurface);
        _mappedLfb = nullptr;
    }

    if (display._vbeMode != 0) {
        // VBE mode - try to set mode with LFB
        if (Platform::DOS::Graphics::SetVBEMode(display._vbeMode, &g_vbeSurface)) {
            // Success - the surface now contains LFB info
            // Store selector as "mapped LFB" indicator (cast to void* for API compatibility)
            _mappedLfb = reinterpret_cast<void*>(static_cast<unsigned long>(g_vbeSurface.selector));
            _current = display;
        } else {
            // VBE mode setting failed - revert to text mode
            Platform::DOS::Graphics::SetVideoMode(0x03);
            _current = TextMode;
        }
    } else {
        // Standard VGA mode
        Platform::DOS::Graphics::SetVideoMode(display._mode);
        _current = display;
    }
}

void Display::SetDefaultMode() {
    // Cleanup VBE surface if active
    if (g_vbeSurface.valid) {
        Platform::DOS::Graphics::CleanupVBE(&g_vbeSurface);
        _mappedLfb = nullptr;
    }
    SetMode(TextMode);
}

void Display::WaitForVSync() {
    Platform::DOS::Graphics::WaitForVSync();
}

void Display::StashPalette() {
    if (static_cast<bool>(_paletteStashed)) return;

    // Read current palette from VGA DAC
    // Port 0x3C7: Set read index
    // Port 0x3C9: Read RGB values (auto-increments)
    outportb(0x3C7, 0);
    for (Int32 i = Int32(0); static_cast<int>(i) < PALETTE_SIZE; i += 1) {
        _originalPalette[static_cast<int>(i)][0] = inportb(0x3C9);  // R
        _originalPalette[static_cast<int>(i)][1] = inportb(0x3C9);  // G
        _originalPalette[static_cast<int>(i)][2] = inportb(0x3C9);  // B
    }
    _paletteStashed = Boolean(true);
}

void Display::SetPaletteScale(Float32 scale) {
    // Port 0x3C8: Set write index
    // Port 0x3C9: Write RGB values (auto-increments)
    outportb(0x3C8, 0);
    for (Int32 i = Int32(0); static_cast<int>(i) < PALETTE_SIZE; i += 1) {
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][0] * static_cast<float>(scale)));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][1] * static_cast<float>(scale)));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][2] * static_cast<float>(scale)));
    }
}

/******************************************************************************/
/*    VBE 3.0 Gamma Ramp Functions                                             */
/******************************************************************************/

Boolean Display::CheckGammaSupport() {
    if (static_cast<bool>(_gammaChecked)) {
        return _gammaSupported;
    }
    _gammaChecked = Boolean(true);
    _gammaSupported = Boolean(Platform::DOS::Graphics::IsGammaSupported());
    return _gammaSupported;
}

Boolean Display::IsGammaSupported() {
    return CheckGammaSupport();
}

void Display::StashGamma() {
    if (static_cast<bool>(_gammaStashed)) return;

    // Try to get current gamma table
    if (Platform::DOS::Graphics::GetGammaTable(_originalGamma)) {
        _gammaStashed = Boolean(true);
    } else {
        // Initialize with identity gamma (no correction)
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1) {
            _originalGamma[static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));         // R
            _originalGamma[256 + static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));   // G
            _originalGamma[512 + static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));   // B
        }
        _gammaStashed = Boolean(true);
    }
}

void Display::SetGammaScale(Float32 scale) {
    unsigned char scaledGamma[GAMMA_TABLE_SIZE];

    // Scale each channel's gamma entries
    for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1) {
        scaledGamma[static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[static_cast<int>(i)] * static_cast<float>(scale));         // R
        scaledGamma[256 + static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[256 + static_cast<int>(i)] * static_cast<float>(scale));   // G
        scaledGamma[512 + static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[512 + static_cast<int>(i)] * static_cast<float>(scale));   // B
    }

    Platform::DOS::Graphics::SetGammaTable(scaledGamma);
}

/******************************************************************************/
/*    Fade Effects                                                             */
/******************************************************************************/

void Display::FadeIn(Int32 milliseconds) {
    Int32 ms = milliseconds;
    if (static_cast<int>(ms) < FRAME_MS) ms = Int32(FRAME_MS);

    Int32 steps = Int32(static_cast<int>(ms) / FRAME_MS);
    if (static_cast<int>(steps) < 1) steps = Int32(1);

    // For VBE modes, try hardware gamma ramp first (VBE 3.0)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24) {
        // Try VBE 3.0 gamma ramp (hardware-accelerated, like VGA palette)
        if (static_cast<bool>(CheckGammaSupport())) {
            StashGamma();

            // Fade from black to full brightness using gamma ramp
            for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(steps); step += 1) {
                Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
                SetGammaScale(scale);
                WaitForVSync();
            }
            return;
        }

        // Fallback: software pixel-based fade (slower)
        // Use fewer steps since each step is expensive
        const Int32 VBE_FADE_STEPS = Int32(8);
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        Int32 width = img.Width();
        Int32 height = img.Height();

        // Store original pixel data
        UInt32* original = static_cast<UInt32*>(std::malloc(static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));

        // Fade from black to original
        for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(VBE_FADE_STEPS); step += 1) {
            Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(VBE_FADE_STEPS));

            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(width) * static_cast<int>(height); i += 1) {
                UInt32 p = original[static_cast<int>(i)];
                UInt8 a = UInt8((static_cast<unsigned int>(p) >> 24) & 0xFF);
                UInt8 r = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 16) & 0xFF) * static_cast<float>(scale)));
                UInt8 g = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 8) & 0xFF) * static_cast<float>(scale)));
                UInt8 b = UInt8(static_cast<unsigned char>((static_cast<unsigned int>(p) & 0xFF) * static_cast<float>(scale)));
                pixels[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) | (static_cast<unsigned int>(r) << 16) | (static_cast<unsigned int>(g) << 8) | static_cast<unsigned int>(b);
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        // Restore original
        std::memcpy(pixels, original, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));
        Drawing::GraphicsBuffer::FlushFrameBuffer();

        std::free(original);
        return;
    }

    // VGA palette-based fade (hardware-accelerated)
    StashPalette();

    // Fade from black to full palette
    for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(steps); step += 1) {
        Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

void Display::FadeOut(Int32 milliseconds) {
    Int32 ms = milliseconds;
    if (static_cast<int>(ms) < FRAME_MS) ms = Int32(FRAME_MS);

    Int32 steps = Int32(static_cast<int>(ms) / FRAME_MS);
    if (static_cast<int>(steps) < 1) steps = Int32(1);

    // For VBE modes, try hardware gamma ramp first (VBE 3.0)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24) {
        // Try VBE 3.0 gamma ramp (hardware-accelerated, like VGA palette)
        if (static_cast<bool>(CheckGammaSupport())) {
            StashGamma();

            // Fade from full brightness to black using gamma ramp
            for (Int32 step = steps; static_cast<int>(step) >= 0; step -= 1) {
                Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
                SetGammaScale(scale);
                WaitForVSync();
            }
            return;
        }

        // Fallback: software pixel-based fade (slower)
        // Use fewer steps since each step is expensive
        const Int32 VBE_FADE_STEPS = Int32(8);
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        Int32 width = img.Width();
        Int32 height = img.Height();

        // Store original pixel data
        UInt32* original = static_cast<UInt32*>(std::malloc(static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));

        // Fade from original to black
        for (Int32 step = VBE_FADE_STEPS; static_cast<int>(step) >= 0; step -= 1) {
            Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(VBE_FADE_STEPS));

            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(width) * static_cast<int>(height); i += 1) {
                UInt32 p = original[static_cast<int>(i)];
                UInt8 a = UInt8((static_cast<unsigned int>(p) >> 24) & 0xFF);
                UInt8 r = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 16) & 0xFF) * static_cast<float>(scale)));
                UInt8 g = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 8) & 0xFF) * static_cast<float>(scale)));
                UInt8 b = UInt8(static_cast<unsigned char>((static_cast<unsigned int>(p) & 0xFF) * static_cast<float>(scale)));
                pixels[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) | (static_cast<unsigned int>(r) << 16) | (static_cast<unsigned int>(g) << 8) | static_cast<unsigned int>(b);
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        std::free(original);
        return;
    }

    // VGA palette-based fade (hardware-accelerated)
    StashPalette();

    // Fade from full palette to black
    for (Int32 step = steps; static_cast<int>(step) >= 0; step -= 1) {
        Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

Boolean Display::IsVbeAvailable() {
    if (static_cast<bool>(_vbeChecked)) {
        return _vbeAvailable;
    }

    _vbeChecked = Boolean(true);

    // Check if VBE 2.0+ is available
    static Platform::DOS::VbeInfoBlock vbeInfo;
    _vbeAvailable = Boolean(Platform::DOS::Graphics::DetectVBE(&vbeInfo));

    // Also verify it's VBE 2.0 or higher (we need LFB support)
    if (static_cast<bool>(_vbeAvailable) && vbeInfo.version < 0x0200) {
        _vbeAvailable = Boolean(false);
    }

    return _vbeAvailable;
}

Display Display::DetectVbeMode(UInt16 width, UInt16 height, UInt8 bpp) {
    unsigned short targetWidth = static_cast<unsigned short>(width);
    unsigned short targetHeight = static_cast<unsigned short>(height);
    UInt8 targetBpp = bpp;

    // Use static buffers to avoid large stack allocations
    static Platform::DOS::VbeInfoBlock vbeInfo;
    static Platform::DOS::VbeModeInfoBlock modeInfo;

    // Check if VBE is available
    if (!Platform::DOS::Graphics::DetectVBE(&vbeInfo)) {
        return TextMode;  // VBE not available
    }

    // Common mode numbers to try for 800x600x32
    // These may vary by graphics card, so we try several
    unsigned short modesToTry[] = {
        0x115,  // Standard VESA 800x600x24/32
        0x143,  // Some cards use this
        0x190,  // Alternative
        0x192,  // Alternative
        0x118,  // 1024x768x24 (fallback)
        0x112,  // 640x480x24 (fallback)
        0xFFFF  // End marker
    };

    for (Int32 i = Int32(0); modesToTry[static_cast<int>(i)] != 0xFFFF; i += 1) {
        unsigned short mode = modesToTry[static_cast<int>(i)];

        if (!Platform::DOS::Graphics::GetVBEModeInfo(mode, &modeInfo)) {
            continue;
        }

        // Check if mode is supported and has LFB
        if (!(modeInfo.modeAttributes & Platform::DOS::VBE_ATTR_SUPPORTED)) {
            continue;
        }
        if (!(modeInfo.modeAttributes & Platform::DOS::VBE_ATTR_LFB_AVAIL)) {
            continue;
        }

        // Check dimensions and color depth
        Boolean matchesWidth = Boolean(modeInfo.xResolution == targetWidth);
        Boolean matchesHeight = Boolean(modeInfo.yResolution == targetHeight);
        Boolean matchesBpp = Boolean(modeInfo.bitsPerPixel == static_cast<unsigned char>(targetBpp) ||
                          (static_cast<unsigned char>(targetBpp) == 32 && modeInfo.bitsPerPixel == 24));  // 24/32 often interchangeable

        if (static_cast<bool>(matchesWidth) && static_cast<bool>(matchesHeight) && static_cast<bool>(matchesBpp)) {
            return Display(mode, modeInfo.bitsPerPixel,
                          modeInfo.xResolution, modeInfo.yResolution,
                          modeInfo.physBasePtr, modeInfo.bytesPerScanLine);
        }
    }

    // Mode not found
    return TextMode;
}

const Display Display::TextMode(0x03, 0, 80, 25);
const Display Display::VGA_320x200x8(0x13, 8, 320, 200);
const Display Display::VGA_640x480x4(0x12, 4, 640, 480);

/******************************************************************************/
/*    Mouse implementation                                                    */
/******************************************************************************/

Boolean Mouse::_initialized = Boolean(false);

Boolean Mouse::Initialize() {
    _initialized = Boolean(Platform::DOS::Mouse::Initialize());
    return _initialized;
}

Boolean Mouse::IsAvailable() {
    return _initialized;
}

void Mouse::ShowCursor() {
    if (static_cast<bool>(_initialized)) {
        Platform::DOS::Mouse::ShowCursor();
    }
}

void Mouse::HideCursor() {
    if (static_cast<bool>(_initialized)) {
        Platform::DOS::Mouse::HideCursor();
    }
}

MouseStatus Mouse::GetStatus() {
    if (!static_cast<bool>(_initialized)) {
        return MouseStatus();
    }

    Platform::DOS::MouseState state = Platform::DOS::Mouse::GetState();
    return MouseStatus(Int32(state.x), Int32(state.y),
                       Boolean(state.leftButton), Boolean(state.rightButton),
                       Boolean(state.middleButton));
}

Int32 Mouse::GetX() {
    return GetStatus().x;
}

Int32 Mouse::GetY() {
    return GetStatus().y;
}

void Mouse::SetPosition(Int32 x, Int32 y) {
    if (static_cast<bool>(_initialized)) {
        Platform::DOS::Mouse::SetPosition(static_cast<int>(x), static_cast<int>(y));
    }
}

void Mouse::SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY) {
    if (static_cast<bool>(_initialized)) {
        Platform::DOS::Mouse::SetHorizontalBounds(static_cast<int>(minX), static_cast<int>(maxX));
        Platform::DOS::Mouse::SetVerticalBounds(static_cast<int>(minY), static_cast<int>(maxY));
    }
}

void Mouse::SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys) {
    if (static_cast<bool>(_initialized)) {
        Platform::DOS::Mouse::SetSensitivity(
            static_cast<int>(horizontalMickeys),
            static_cast<int>(verticalMickeys)
        );
    }
}

Boolean Mouse::IsLeftButtonPressed() {
    return GetStatus().leftButton;
}

Boolean Mouse::IsRightButtonPressed() {
    return GetStatus().rightButton;
}

/******************************************************************************/
/*    Keyboard implementation                                                 */
/******************************************************************************/

Boolean Keyboard::IsKeyPressed() {
    return Boolean(kbhit() != 0);
}

Char Keyboard::ReadKey() {
    return Char(getch());
}

Char Keyboard::PeekKey() {
    if (!kbhit()) return Char('\0');
    // DJGPP doesn't have _peek behavior, so we need to use interrupt
    __dpmi_regs regs;
    regs.h.ah = 0x01;  // Check for keystroke
    __dpmi_int(0x16, &regs);
    if (regs.x.flags & 0x40) {  // Zero flag set = no key
        return Char('\0');
    }
    return Char(regs.h.al);
}

KeyboardStatus Keyboard::GetStatus() {
    __dpmi_regs regs;
    regs.h.ah = 0x02;  // Get shift flags
    __dpmi_int(0x16, &regs);

    KeyboardStatus status;
    status.shiftPressed = Boolean((regs.h.al & 0x03) != 0);
    status.ctrlPressed = Boolean((regs.h.al & 0x04) != 0);
    status.altPressed = Boolean((regs.h.al & 0x08) != 0);
    status.scrollLock = Boolean((regs.h.al & 0x10) != 0);
    status.numLock = Boolean((regs.h.al & 0x20) != 0);
    status.capsLock = Boolean((regs.h.al & 0x40) != 0);

    return status;
}

}} // namespace System::Devices
