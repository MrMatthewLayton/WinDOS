#ifndef SYSTEM_IO_DEVICES_DISPLAY_HPP
#define SYSTEM_IO_DEVICES_DISPLAY_HPP

#include "../../Types.hpp"

namespace System::IO::Devices
{

/******************************************************************************/
/*    Display                                                                 */
/******************************************************************************/

/// @brief Provides display mode management and VGA/VBE graphics/text operations.
///
/// The Display class manages video modes including standard VGA modes (text,
/// 320x200x8, 640x480x4) and VESA BIOS Extensions (VBE) high-resolution modes.
/// It supports linear framebuffer access for VBE modes and provides fade
/// effects using either VBE 3.0 gamma ramps or VGA palette manipulation.
/// Also provides text-mode cursor operations and screen scrolling.
///
/// @note This class uses a singleton-like pattern where GetCurrent() returns
///       the active display mode and SetMode() changes it.
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
    Display(unsigned char mode, unsigned char bpp, unsigned short width, unsigned short height);

    /// @brief Constructs a VBE display mode with linear framebuffer.
    Display(unsigned short vbeMode, unsigned char bpp, unsigned short width, unsigned short height,
            unsigned int lfbAddr, unsigned int pitch);

    static Display _current;
    static Boolean _vbeAvailable;
    static Boolean _vbeChecked;
    static void* _mappedLfb;

    // Palette storage for fade effects (VGA modes)
    static unsigned char _originalPalette[PALETTE_SIZE][3];
    static Boolean _paletteStashed;

    // VBE 3.0 gamma ramp support
    static const int GAMMA_TABLE_SIZE = 256 * 3;  // 256 entries per R/G/B channel
    static unsigned char _originalGamma[GAMMA_TABLE_SIZE];
    static Boolean _gammaStashed;
    static Boolean _gammaSupported;
    static Boolean _gammaChecked;

    // Private helper methods
    static void StashPalette();
    static void SetPaletteScale(Float32 scale);
    static Boolean CheckGammaSupport();
    static void StashGamma();
    static void SetGammaScale(Float32 scale);

    // Low-level BIOS/hardware operations - truly private
    static void BiosSetVideoMode(unsigned char mode);
    static unsigned char BiosGetVideoMode();
    static void BiosWaitForVSync();
    static void BiosSelectPlane(int plane);
    static void BiosCopyToVGA(const void* data, unsigned int offset, unsigned int length);
    static void BiosOutPort(unsigned short port, unsigned char value);
    static unsigned char BiosInPort(unsigned short port);
    static bool BiosDetectVBE(void* vbeInfo);
    static bool BiosGetVBEModeInfo(unsigned short mode, void* modeInfo);
    static bool BiosSetVBEMode(unsigned short mode, void* vbeSurface);
    static void BiosCleanupVBE(void* vbeSurface);
    static int BiosGetLfbSelector();
    static bool BiosIsGammaSupported();
    static bool BiosSetGammaTable(const unsigned char* gammaTable);
    static bool BiosGetGammaTable(unsigned char* gammaTable);

    // Text mode operations - private BIOS calls
    static void BiosSetCursorPosition(int row, int col);
    static void BiosGetCursorPosition(int& row, int& col);
    static void BiosWriteChar(char c, unsigned char attr);
    static void BiosScrollUp(int lines, unsigned char attr, int top, int left, int bottom, int right);
    static void BiosGetScreenSize(int& rows, int& cols);

public:
    /// @brief Copy constructor.
    Display(const Display& other);

    /// @brief Copy assignment operator.
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

    /**************************************************************************/
    /*    Text Mode Operations                                                */
    /**************************************************************************/

    /// @brief Sets the text cursor position on screen.
    /// @param row The row position (0-based, where 0 is the top row)
    /// @param col The column position (0-based, where 0 is the leftmost column)
    ///
    /// Moves the blinking text cursor to the specified row and column.
    /// Only affects text modes (e.g., 80x25).
    static void SetCursorPosition(Int32 row, Int32 col);

    /// @brief Gets the current text cursor position.
    /// @param row Receives the current row position (0-based)
    /// @param col Receives the current column position (0-based)
    ///
    /// Retrieves the current cursor row and column from the BIOS data area.
    static void GetCursorPosition(Int32& row, Int32& col);

    /// @brief Gets the current screen size in text mode.
    /// @param rows Receives the number of rows (typically 25, 43, or 50)
    /// @param cols Receives the number of columns (typically 40 or 80)
    ///
    /// Retrieves the current screen dimensions from the BIOS data area.
    /// Only valid in text modes.
    static void GetScreenSize(Int32& rows, Int32& cols);

    /// @brief Clears the entire screen with a specified attribute.
    /// @param attr The color attribute to fill the screen with
    ///
    /// Fills the entire screen with space characters using the given color
    /// attribute, and resets the cursor to the home position (0, 0).
    static void ClearScreen(UInt8 attr);

    /// @brief Writes a character with a color attribute at the current cursor position.
    /// @param c The character to write
    /// @param attr The color attribute (foreground and background color combined)
    ///
    /// Writes the character to the screen at the current cursor position using
    /// the specified color attribute. Does not advance the cursor.
    static void WriteChar(char c, UInt8 attr);

    /// @brief Scrolls a screen region up by the specified number of lines.
    /// @param lines Number of lines to scroll up
    /// @param attr Color attribute for the blank line at the bottom
    /// @param left Left column of the scroll region (0-based)
    /// @param top Top row of the scroll region (0-based)
    /// @param right Right column of the scroll region (0-based)
    /// @param bottom Bottom row of the scroll region (0-based)
    ///
    /// Scrolls the specified rectangular region upward, filling the bottom
    /// with blank characters using the given attribute.
    static void ScrollUp(Int32 lines, UInt8 attr, Int32 left, Int32 top, Int32 right, Int32 bottom);

    /**************************************************************************/
    /*    Low-Level VGA Operations (for GraphicsBuffer use)                   */
    /**************************************************************************/

    /// @brief Select a VGA bit plane for writing in planar modes.
    /// @param plane Plane number (0-3) to select for writing
    ///
    /// In VGA mode 0x12 (640x480x16), each pixel's 4-bit color is spread across
    /// 4 bit planes. This function selects which plane subsequent writes will affect.
    static void SelectPlane(Int32 plane);

    /// @brief Copy data to VGA video memory.
    /// @param data Pointer to source data buffer
    /// @param offset Byte offset within VGA memory (from 0xA0000)
    /// @param length Number of bytes to copy
    static void CopyToVGA(const void* data, UInt32 offset, UInt32 length);

    /// @brief Write a byte value to a VGA I/O port.
    /// @param port I/O port address
    /// @param value Byte value to write
    static void OutPort(UInt16 port, UInt8 value);

    /// @brief Read a byte value from a VGA I/O port.
    /// @param port I/O port address
    /// @return Byte value read from the port
    static UInt8 InPort(UInt16 port);

    /// @brief Gets the LDT selector for linear framebuffer access.
    /// @return LDT selector for LFB access, or 0 if no VBE mode is active
    ///
    /// Returns the selector allocated for VBE linear framebuffer access.
    /// Use with movedata() or far pointer functions.
    static Int32 GetLfbSelector();

    /**************************************************************************/
    /*    Predefined Display Modes                                            */
    /**************************************************************************/

    /// @brief Text mode (80x25 characters, mode 0x03).
    static const Display TextMode;

    /// @brief VGA mode 0x13: 320x200 pixels, 256 colors.
    static const Display VGA_320x200x8;

    /// @brief VGA mode 0x12: 640x480 pixels, 16 colors.
    static const Display VGA_640x480x4;

    /// @brief Common VBE mode number for 800x600x32.
    static const unsigned short VBE_800x600x32 = 0x115;
};

} // namespace System::IO::Devices

#endif // SYSTEM_IO_DEVICES_DISPLAY_HPP
