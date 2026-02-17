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
bool Display::_vbeAvailable = false;
bool Display::_vbeChecked = false;
void* Display::_mappedLfb = nullptr;
unsigned char Display::_originalPalette[PALETTE_SIZE][3] = {{0}};
bool Display::_paletteStashed = false;

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
    if (_paletteStashed) return;

    // Read current palette from VGA DAC
    // Port 0x3C7: Set read index
    // Port 0x3C9: Read RGB values (auto-increments)
    outportb(0x3C7, 0);
    for (int i = 0; i < PALETTE_SIZE; i++) {
        _originalPalette[i][0] = inportb(0x3C9);  // R
        _originalPalette[i][1] = inportb(0x3C9);  // G
        _originalPalette[i][2] = inportb(0x3C9);  // B
    }
    _paletteStashed = true;
}

void Display::SetPaletteScale(float scale) {
    // Port 0x3C8: Set write index
    // Port 0x3C9: Write RGB values (auto-increments)
    outportb(0x3C8, 0);
    for (int i = 0; i < PALETTE_SIZE; i++) {
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[i][0] * scale));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[i][1] * scale));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[i][2] * scale));
    }
}

void Display::FadeIn(Int32 milliseconds) {
    int ms = static_cast<int>(milliseconds);
    if (ms < FRAME_MS) ms = FRAME_MS;

    // For VBE 32-bit modes, use framebuffer-based fade with fewer steps
    // (each step is expensive: per-pixel processing + full buffer flush)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24) {
        // Use 8 steps for VBE - smooth enough visually, completes in ~500ms
        const int VBE_FADE_STEPS = 8;
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        int width = static_cast<int>(img.Width());
        int height = static_cast<int>(img.Height());

        // Store original pixel data
        unsigned int* original = static_cast<unsigned int*>(std::malloc(width * height * sizeof(unsigned int)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, width * height * sizeof(unsigned int));

        // Fade from black to original
        for (int step = 0; step <= VBE_FADE_STEPS; step++) {
            float scale = static_cast<float>(step) / VBE_FADE_STEPS;

            for (int i = 0; i < width * height; i++) {
                unsigned int p = original[i];
                unsigned char a = (p >> 24) & 0xFF;
                unsigned char r = static_cast<unsigned char>(((p >> 16) & 0xFF) * scale);
                unsigned char g = static_cast<unsigned char>(((p >> 8) & 0xFF) * scale);
                unsigned char b = static_cast<unsigned char>((p & 0xFF) * scale);
                pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        // Restore original
        std::memcpy(pixels, original, width * height * sizeof(unsigned int));
        Drawing::GraphicsBuffer::FlushFrameBuffer();

        std::free(original);
        return;
    }

    // VGA palette-based fade (fast - only 768 bytes to update)
    int steps = ms / FRAME_MS;
    if (steps < 1) steps = 1;

    StashPalette();

    // Fade from black to full palette
    for (int step = 0; step <= steps; step++) {
        float scale = static_cast<float>(step) / steps;
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

void Display::FadeOut(Int32 milliseconds) {
    int ms = static_cast<int>(milliseconds);
    if (ms < FRAME_MS) ms = FRAME_MS;

    // For VBE 32-bit modes, use framebuffer-based fade with fewer steps
    // (each step is expensive: per-pixel processing + full buffer flush)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24) {
        // Use 8 steps for VBE - smooth enough visually, completes in ~500ms
        const int VBE_FADE_STEPS = 8;
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        int width = static_cast<int>(img.Width());
        int height = static_cast<int>(img.Height());

        // Store original pixel data
        unsigned int* original = static_cast<unsigned int*>(std::malloc(width * height * sizeof(unsigned int)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, width * height * sizeof(unsigned int));

        // Fade from original to black
        for (int step = VBE_FADE_STEPS; step >= 0; step--) {
            float scale = static_cast<float>(step) / VBE_FADE_STEPS;

            for (int i = 0; i < width * height; i++) {
                unsigned int p = original[i];
                unsigned char a = (p >> 24) & 0xFF;
                unsigned char r = static_cast<unsigned char>(((p >> 16) & 0xFF) * scale);
                unsigned char g = static_cast<unsigned char>(((p >> 8) & 0xFF) * scale);
                unsigned char b = static_cast<unsigned char>((p & 0xFF) * scale);
                pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        std::free(original);
        return;
    }

    // VGA palette-based fade (fast - only 768 bytes to update)
    int steps = ms / FRAME_MS;
    if (steps < 1) steps = 1;

    StashPalette();

    // Fade from full palette to black
    for (int step = steps; step >= 0; step--) {
        float scale = static_cast<float>(step) / steps;
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

Boolean Display::IsVbeAvailable() {
    if (_vbeChecked) {
        return Boolean(_vbeAvailable);
    }

    _vbeChecked = true;

    // Check if VBE 2.0+ is available
    static Platform::DOS::VbeInfoBlock vbeInfo;
    _vbeAvailable = Platform::DOS::Graphics::DetectVBE(&vbeInfo);

    // Also verify it's VBE 2.0 or higher (we need LFB support)
    if (_vbeAvailable && vbeInfo.version < 0x0200) {
        _vbeAvailable = false;
    }

    return Boolean(_vbeAvailable);
}

Display Display::DetectVbeMode(UInt16 width, UInt16 height, UInt8 bpp) {
    unsigned short targetWidth = static_cast<unsigned short>(width);
    unsigned short targetHeight = static_cast<unsigned short>(height);
    unsigned char targetBpp = static_cast<unsigned char>(bpp);

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

    for (int i = 0; modesToTry[i] != 0xFFFF; i++) {
        unsigned short mode = modesToTry[i];

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
        bool matchesWidth = (modeInfo.xResolution == targetWidth);
        bool matchesHeight = (modeInfo.yResolution == targetHeight);
        bool matchesBpp = (modeInfo.bitsPerPixel == targetBpp ||
                          (targetBpp == 32 && modeInfo.bitsPerPixel == 24));  // 24/32 often interchangeable

        if (matchesWidth && matchesHeight && matchesBpp) {
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

bool Mouse::_initialized = false;

Boolean Mouse::Initialize() {
    _initialized = Platform::DOS::Mouse::Initialize();
    return Boolean(_initialized);
}

Boolean Mouse::IsAvailable() {
    return Boolean(_initialized);
}

void Mouse::ShowCursor() {
    if (_initialized) {
        Platform::DOS::Mouse::ShowCursor();
    }
}

void Mouse::HideCursor() {
    if (_initialized) {
        Platform::DOS::Mouse::HideCursor();
    }
}

MouseStatus Mouse::GetStatus() {
    if (!_initialized) {
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
    if (_initialized) {
        Platform::DOS::Mouse::SetPosition(static_cast<int>(x), static_cast<int>(y));
    }
}

void Mouse::SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY) {
    if (_initialized) {
        Platform::DOS::Mouse::SetHorizontalBounds(static_cast<int>(minX), static_cast<int>(maxX));
        Platform::DOS::Mouse::SetVerticalBounds(static_cast<int>(minY), static_cast<int>(maxY));
    }
}

void Mouse::SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys) {
    if (_initialized) {
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
