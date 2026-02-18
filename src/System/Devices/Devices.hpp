#ifndef SYSTEM_DEVICES_HPP
#define SYSTEM_DEVICES_HPP

#include "../Types.hpp"

/// @file Devices.hpp
/// @brief Hardware device abstraction layer for DOS systems.
///
/// Provides high-level interfaces for display, mouse, and keyboard hardware.
/// Uses BIOS interrupts and direct port I/O for hardware access.

namespace System::Devices
{

/******************************************************************************/
/*    System::Devices::Display                                                */
/******************************************************************************/

/// @brief Provides display mode management and VGA/VBE graphics output.
///
/// The Display class manages video modes including standard VGA modes (text,
/// 320x200x8, 640x480x4) and VESA BIOS Extensions (VBE) high-resolution modes.
/// It supports linear framebuffer access for VBE modes and provides fade
/// effects using either VBE 3.0 gamma ramps or VGA palette manipulation.
///
/// @note This class uses a singleton-like pattern where GetCurrent() returns
///       the active display mode and SetMode() changes it.
///
/// @see GraphicsBuffer for drawing operations
class Display
{
    static const int PALETTE_SIZE = 256;
    static const int FRAME_MS = 1000 / 60;  // ~16ms per frame at 60fps

    unsigned char _mode;          // Standard VGA mode (0 for VBE modes)
    unsigned char _bitsPerPixel;
    unsigned short _width;
    unsigned short _height;
    unsigned short _vbeMode;      // VBE mode number (0 for standard VGA)
    unsigned int _lfbPhysAddr;    // Physical address of linear framebuffer
    unsigned int _lfbPitch;       // Bytes per scanline

    /// @brief Constructs a standard VGA display mode.
    /// @param mode VGA mode number (e.g., 0x03 for text, 0x13 for 320x200x8)
    /// @param bpp Bits per pixel
    /// @param width Screen width in pixels
    /// @param height Screen height in pixels
    Display(unsigned char mode, unsigned char bpp, unsigned short width, unsigned short height);

    /// @brief Constructs a VBE display mode with linear framebuffer.
    /// @param vbeMode VBE mode number (e.g., 0x115 for 800x600x32)
    /// @param bpp Bits per pixel (24 or 32)
    /// @param width Screen width in pixels
    /// @param height Screen height in pixels
    /// @param lfbAddr Physical address of the linear framebuffer
    /// @param pitch Bytes per scanline (may include padding)
    Display(unsigned short vbeMode, unsigned char bpp, unsigned short width, unsigned short height,
            unsigned int lfbAddr, unsigned int pitch);

    static Display _current;
    static Boolean _vbeAvailable;
    static Boolean _vbeChecked;
    static void* _mappedLfb;

    // Palette storage for fade effects (VGA modes)
    static unsigned char _originalPalette[PALETTE_SIZE][3];
    static Boolean _paletteStashed;

    /// @brief Saves the current VGA palette for fade effects.
    static void StashPalette();

    /// @brief Sets VGA palette to a scaled version of the stashed palette.
    /// @param scale Scale factor (0.0 = black, 1.0 = original)
    static void SetPaletteScale(Float32 scale);

    // VBE 3.0 gamma ramp support
    static const int GAMMA_TABLE_SIZE = 256 * 3;  // 256 entries per R/G/B channel
    static unsigned char _originalGamma[GAMMA_TABLE_SIZE];
    static Boolean _gammaStashed;
    static Boolean _gammaSupported;
    static Boolean _gammaChecked;

    /// @brief Checks if VBE 3.0 gamma ramp functions are available.
    /// @return True if gamma ramp control is supported
    static Boolean CheckGammaSupport();

    /// @brief Saves the current gamma ramp for fade effects.
    static void StashGamma();

    /// @brief Sets gamma ramp to a scaled version of the stashed gamma.
    /// @param scale Scale factor (0.0 = black, 1.0 = original)
    static void SetGammaScale(Float32 scale);

public:
    /// @brief Copy constructor.
    /// @param other Display to copy from
    Display(const Display& other);

    /// @brief Copy assignment operator.
    /// @param other Display to copy from
    /// @return Reference to this display
    Display& operator=(const Display& other);

    /// @brief Gets the VGA mode number.
    /// @return VGA mode number (0 if using VBE mode)
    UInt8 Mode() const
    {
        return UInt8(_mode);
    }

    /// @brief Gets the color depth in bits per pixel.
    /// @return Bits per pixel (4, 8, 24, or 32)
    UInt8 BitsPerPixel() const
    {
        return UInt8(_bitsPerPixel);
    }

    /// @brief Gets the screen width in pixels.
    /// @return Screen width
    UInt16 Width() const
    {
        return UInt16(_width);
    }

    /// @brief Gets the screen height in pixels.
    /// @return Screen height
    UInt16 Height() const
    {
        return UInt16(_height);
    }

    /// @brief Gets the VBE mode number.
    /// @return VBE mode number (0 if using standard VGA mode)
    UInt16 VbeMode() const
    {
        return UInt16(_vbeMode);
    }

    /// @brief Gets the physical address of the linear framebuffer.
    /// @return Physical memory address of LFB (VBE modes only)
    UInt32 LfbPhysAddress() const
    {
        return UInt32(_lfbPhysAddr);
    }

    /// @brief Gets the pitch (bytes per scanline) of the framebuffer.
    /// @return Bytes per scanline (may be larger than width * bytesPerPixel)
    UInt32 LfbPitch() const
    {
        return UInt32(_lfbPitch);
    }

    /// @brief Checks if this is a VBE mode.
    /// @return True if VBE mode, false if standard VGA
    Boolean IsVbeMode() const
    {
        return Boolean(_vbeMode != 0);
    }

    /// @brief Gets the currently active display mode.
    /// @return The current display configuration
    static Display GetCurrent();

    /// @brief Sets the display to the specified mode.
    /// @param display The display mode to activate
    ///
    /// For VBE modes, this sets up the linear framebuffer mapping.
    /// For VGA modes, this sets the mode via INT 10h.
    static void SetMode(const Display& display);

    /// @brief Resets the display to 80x25 text mode.
    static void SetDefaultMode();

    /// @brief Waits for the vertical sync signal.
    ///
    /// Synchronizes with the display refresh to prevent tearing.
    /// Waits for the current retrace to end, then for the next to begin.
    static void WaitForVSync();

    /// @brief Fades the screen in from black over the specified duration.
    /// @param milliseconds Duration of the fade effect
    ///
    /// Uses VBE 3.0 gamma ramp if available, otherwise falls back to
    /// VGA palette manipulation or software pixel fading.
    static void FadeIn(Int32 milliseconds);

    /// @brief Fades the screen out to black over the specified duration.
    /// @param milliseconds Duration of the fade effect
    ///
    /// Uses VBE 3.0 gamma ramp if available, otherwise falls back to
    /// VGA palette manipulation or software pixel fading.
    static void FadeOut(Int32 milliseconds);

    /// @brief Checks if VBE 2.0+ extensions are available.
    /// @return True if VESA BIOS Extensions are supported
    static Boolean IsVbeAvailable();

    /// @brief Checks if VBE 3.0 gamma ramp control is available.
    /// @return True if hardware gamma control is supported
    static Boolean IsGammaSupported();

    /// @brief Detects a VBE mode matching the requested parameters.
    /// @param width Desired screen width
    /// @param height Desired screen height
    /// @param bpp Desired bits per pixel (24 or 32)
    /// @return Display configured for the detected mode
    /// @throws InvalidOperationException if no matching mode is found
    static Display DetectVbeMode(UInt16 width, UInt16 height, UInt8 bpp);

    /// @brief Gets the mapped linear framebuffer address.
    /// @return Pointer to the mapped LFB, or nullptr if not mapped
    static void* GetMappedLfb()
    {
        return _mappedLfb;
    }

    /// @brief Text mode (80x25 characters, mode 0x03).
    static const Display TextMode;

    /// @brief VGA mode 0x13: 320x200 pixels, 256 colors.
    static const Display VGA_320x200x8;

    /// @brief VGA mode 0x12: 640x480 pixels, 16 colors.
    static const Display VGA_640x480x4;

    /// @brief Common VBE mode number for 800x600x32.
    static const unsigned short VBE_800x600x32 = 0x115;
};

/******************************************************************************/
/*    System::Devices::MouseStatus                                            */
/******************************************************************************/

/// @brief Represents the current state of the mouse.
///
/// Contains the cursor position and button states. Obtained via Mouse::GetStatus().
class MouseStatus
{
public:
    /// @brief X coordinate of the mouse cursor.
    Int32 x;

    /// @brief Y coordinate of the mouse cursor.
    Int32 y;

    /// @brief True if the left mouse button is pressed.
    Boolean leftButton;

    /// @brief True if the right mouse button is pressed.
    Boolean rightButton;

    /// @brief True if the middle mouse button is pressed.
    Boolean middleButton;

    /// @brief Constructs a default MouseStatus with position (0,0) and no buttons pressed.
    MouseStatus()
        : x(0), y(0), leftButton(false), rightButton(false), middleButton(false)
    {
    }

    /// @brief Constructs a MouseStatus with the specified values.
    /// @param x X coordinate of the cursor
    /// @param y Y coordinate of the cursor
    /// @param left True if left button is pressed
    /// @param right True if right button is pressed
    /// @param middle True if middle button is pressed
    MouseStatus(Int32 x, Int32 y, Boolean left, Boolean right, Boolean middle)
        : x(x), y(y), leftButton(left), rightButton(right), middleButton(middle)
    {
    }
};

/******************************************************************************/
/*    System::Devices::Mouse                                                  */
/******************************************************************************/

/// @brief Provides mouse input functionality using INT 33h.
///
/// The Mouse class is a static facade for DOS mouse driver operations.
/// Call Initialize() before using other methods. Use GetStatus() to poll
/// the current mouse state, or use the individual accessor methods.
///
/// @note The DOS mouse driver must be loaded (e.g., CTMOUSE) for this to work.
class Mouse
{
private:
    Mouse();

    static Boolean _initialized;

public:
    /// @brief Initializes the mouse driver.
    /// @return True if initialization succeeded, false otherwise
    ///
    /// This must be called before using any other mouse methods.
    /// Returns false if no mouse driver is installed.
    static Boolean Initialize();

    /// @brief Checks if a mouse is available and initialized.
    /// @return True if mouse is ready for use
    static Boolean IsAvailable();

    /// @brief Shows the mouse cursor.
    ///
    /// Makes the hardware cursor visible. For GUI applications, this is
    /// typically not used since the desktop draws its own cursor.
    static void ShowCursor();

    /// @brief Hides the mouse cursor.
    ///
    /// Hides the hardware cursor. Called when drawing a custom cursor.
    static void HideCursor();

    /// @brief Gets the complete mouse status.
    /// @return MouseStatus containing position and button states
    static MouseStatus GetStatus();

    /// @brief Gets the mouse X coordinate.
    /// @return Current X position of the cursor
    static Int32 GetX();

    /// @brief Gets the mouse Y coordinate.
    /// @return Current Y position of the cursor
    static Int32 GetY();

    /// @brief Sets the mouse cursor position.
    /// @param x New X coordinate
    /// @param y New Y coordinate
    static void SetPosition(Int32 x, Int32 y);

    /// @brief Sets the mouse movement bounds.
    /// @param minX Minimum X coordinate
    /// @param minY Minimum Y coordinate
    /// @param maxX Maximum X coordinate
    /// @param maxY Maximum Y coordinate
    ///
    /// Constrains the cursor to the specified rectangular area.
    static void SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY);

    /// @brief Sets mouse sensitivity (mickeys per 8 pixels).
    /// @param horizontalMickeys Horizontal sensitivity (higher = slower)
    /// @param verticalMickeys Vertical sensitivity (higher = slower)
    ///
    /// Controls how much physical mouse movement is required to move the cursor.
    /// Default values are typically horizontal=8, vertical=16.
    static void SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys);

    /// @brief Checks if the left mouse button is pressed.
    /// @return True if left button is currently down
    static Boolean IsLeftButtonPressed();

    /// @brief Checks if the right mouse button is pressed.
    /// @return True if right button is currently down
    static Boolean IsRightButtonPressed();
};

/******************************************************************************/
/*    System::Devices::KeyboardStatus                                         */
/******************************************************************************/

/// @brief Represents the current state of keyboard modifier keys.
///
/// Contains the states of shift, control, alt, and lock keys.
/// Obtained via Keyboard::GetStatus().
class KeyboardStatus
{
public:
    /// @brief True if either Shift key is pressed.
    Boolean shiftPressed;

    /// @brief True if either Ctrl key is pressed.
    Boolean ctrlPressed;

    /// @brief True if either Alt key is pressed.
    Boolean altPressed;

    /// @brief True if Caps Lock is active.
    Boolean capsLock;

    /// @brief True if Num Lock is active.
    Boolean numLock;

    /// @brief True if Scroll Lock is active.
    Boolean scrollLock;

    /// @brief Constructs a default KeyboardStatus with no modifiers active.
    KeyboardStatus()
        : shiftPressed(false), ctrlPressed(false), altPressed(false)
        , capsLock(false), numLock(false), scrollLock(false)
    {
    }
};

/******************************************************************************/
/*    System::Devices::Keyboard                                               */
/******************************************************************************/

/// @brief Provides keyboard input functionality using BIOS interrupts.
///
/// The Keyboard class is a static facade for DOS keyboard operations.
/// It provides polling-based keyboard input via IsKeyPressed(), ReadKey(),
/// and PeekKey() methods, as well as modifier key status via GetStatus().
class Keyboard
{
private:
    Keyboard();

public:
    /// @brief Checks if a key is available in the keyboard buffer.
    /// @return True if a keypress is waiting to be read
    static Boolean IsKeyPressed();

    /// @brief Reads and removes a key from the keyboard buffer.
    /// @return The ASCII character of the pressed key
    ///
    /// Blocks until a key is pressed if the buffer is empty.
    /// Use IsKeyPressed() to check for available input first.
    static Char ReadKey();

    /// @brief Peeks at the next key without removing it from the buffer.
    /// @return The ASCII character of the next key, or '\0' if none available
    ///
    /// Does not block; returns immediately. The key remains in the buffer
    /// for subsequent ReadKey() or PeekKey() calls.
    static Char PeekKey();

    /// @brief Gets the current keyboard modifier status.
    /// @return KeyboardStatus containing modifier and lock key states
    static KeyboardStatus GetStatus();
};

} // namespace System::Devices

#endif // SYSTEM_DEVICES_HPP
