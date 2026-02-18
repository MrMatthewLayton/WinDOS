#ifndef PLATFORM_DOS_GRAPHICS_HPP
#define PLATFORM_DOS_GRAPHICS_HPP

/// @file Graphics.hpp
/// @brief Low-level VGA and VBE graphics functions for DOS protected mode.
///
/// This header provides direct hardware access to VGA registers and VESA BIOS
/// Extensions (VBE) 2.0+ for high-resolution graphics modes with linear
/// framebuffer support. All functions use DJGPP's protected mode facilities
/// for safe access to video memory and BIOS calls.

namespace Platform::DOS
{

/******************************************************************************/
/*    VBE 2.0+ Structures (packed for BIOS compatibility)                     */
/******************************************************************************/

#pragma pack(push, 1)

/// @brief VBE Controller Information Block returned by INT 10h AX=4F00h.
///
/// This structure contains information about the VBE implementation and
/// available video modes. It must be passed to DetectVBE() to query the
/// graphics hardware capabilities. The structure is packed to match the
/// exact memory layout expected by the VBE BIOS.
///
/// @note Before calling INT 10h AX=4F00h, set signature to "VBE2" to request
///       VBE 2.0+ extended information (oemVersion, oemVendor, etc.).
struct VbeInfoBlock
{
    /// @brief VBE signature, "VESA" on return, set to "VBE2" before call for extended info.
    char     signature[4];

    /// @brief VBE version number (e.g., 0x0200 = 2.0, 0x0300 = 3.0).
    unsigned short version;

    /// @brief Far pointer (real mode segment:offset) to OEM name string.
    unsigned int   oemString;

    /// @brief Capability flags indicating DAC width, VGA compatibility, etc.
    unsigned int   capabilities;

    /// @brief Far pointer to null-terminated list of supported mode numbers.
    unsigned int   modeList;

    /// @brief Total video memory in 64KB blocks.
    unsigned short totalMemory;

    /// @brief OEM software revision number (VBE 2.0+).
    unsigned short oemVersion;

    /// @brief Far pointer to OEM vendor name string (VBE 2.0+).
    unsigned int   oemVendor;

    /// @brief Far pointer to OEM product name string (VBE 2.0+).
    unsigned int   oemProduct;

    /// @brief Far pointer to OEM product revision string (VBE 2.0+).
    unsigned int   oemRevision;

    /// @brief Reserved for VBE implementation use.
    unsigned char  reserved[222];

    /// @brief OEM data area for VBE implementation use.
    unsigned char  oemData[256];
};

/// @brief VBE Mode Information Block returned by INT 10h AX=4F01h.
///
/// This structure contains detailed information about a specific video mode,
/// including resolution, color depth, memory layout, and linear framebuffer
/// address. It must be passed to GetVBEModeInfo() to query mode capabilities
/// before attempting to set the mode.
///
/// @note The physBasePtr field (VBE 2.0+) contains the physical address of
///       the linear framebuffer, which must be mapped into the process address
///       space using DPMI functions before access.
struct VbeModeInfoBlock
{
    /// @brief Mode attribute flags (see VBE_ATTR_* constants).
    unsigned short modeAttributes;

    /// @brief Window A attributes (relocatable, readable, writeable).
    unsigned char  winAAttributes;

    /// @brief Window B attributes (relocatable, readable, writeable).
    unsigned char  winBAttributes;

    /// @brief Window granularity in KB (size of smallest addressable unit).
    unsigned short winGranularity;

    /// @brief Window size in KB.
    unsigned short winSize;

    /// @brief Real mode segment for window A (banked modes).
    unsigned short winASegment;

    /// @brief Real mode segment for window B (banked modes).
    unsigned short winBSegment;

    /// @brief Far pointer to windowing function (banked modes).
    unsigned int   winFuncPtr;

    /// @brief Bytes per horizontal scan line (pitch).
    unsigned short bytesPerScanLine;

    /// @brief Horizontal resolution in pixels (VBE 1.2+).
    unsigned short xResolution;

    /// @brief Vertical resolution in pixels (VBE 1.2+).
    unsigned short yResolution;

    /// @brief Character cell width in pixels (text modes).
    unsigned char  xCharSize;

    /// @brief Character cell height in pixels (text modes).
    unsigned char  yCharSize;

    /// @brief Number of memory planes (1 for packed pixel modes).
    unsigned char  numberOfPlanes;

    /// @brief Bits per pixel (color depth: 8, 16, 24, or 32).
    unsigned char  bitsPerPixel;

    /// @brief Number of memory banks (banked modes).
    unsigned char  numberOfBanks;

    /// @brief Memory model type (4 = packed pixel, 6 = direct color).
    unsigned char  memoryModel;

    /// @brief Bank size in KB.
    unsigned char  bankSize;

    /// @brief Number of complete display pages available.
    unsigned char  numberOfImagePages;

    /// @brief Reserved for page function.
    unsigned char  reserved1;

    /// @brief Size of red color component mask in bits.
    unsigned char  redMaskSize;

    /// @brief Bit position of red color component LSB.
    unsigned char  redFieldPosition;

    /// @brief Size of green color component mask in bits.
    unsigned char  greenMaskSize;

    /// @brief Bit position of green color component LSB.
    unsigned char  greenFieldPosition;

    /// @brief Size of blue color component mask in bits.
    unsigned char  blueMaskSize;

    /// @brief Bit position of blue color component LSB.
    unsigned char  blueFieldPosition;

    /// @brief Size of reserved color component mask in bits.
    unsigned char  rsvdMaskSize;

    /// @brief Bit position of reserved color component LSB.
    unsigned char  rsvdFieldPosition;

    /// @brief Direct color mode attributes.
    unsigned char  directColorModeInfo;

    /// @brief Physical address of linear framebuffer (VBE 2.0+).
    unsigned int   physBasePtr;

    /// @brief Reserved, formerly off-screen memory offset.
    unsigned int   reserved2;

    /// @brief Reserved, formerly off-screen memory size.
    unsigned short reserved3;

    /// @brief Bytes per scan line in linear modes (VBE 3.0+).
    unsigned short linBytesPerScanLine;

    /// @brief Number of images for banked modes (VBE 3.0+).
    unsigned char  bnkNumberOfImagePages;

    /// @brief Number of images for linear modes (VBE 3.0+).
    unsigned char  linNumberOfImagePages;

    /// @brief Red mask size for linear modes (VBE 3.0+).
    unsigned char  linRedMaskSize;

    /// @brief Red field position for linear modes (VBE 3.0+).
    unsigned char  linRedFieldPosition;

    /// @brief Green mask size for linear modes (VBE 3.0+).
    unsigned char  linGreenMaskSize;

    /// @brief Green field position for linear modes (VBE 3.0+).
    unsigned char  linGreenFieldPosition;

    /// @brief Blue mask size for linear modes (VBE 3.0+).
    unsigned char  linBlueMaskSize;

    /// @brief Blue field position for linear modes (VBE 3.0+).
    unsigned char  linBlueFieldPosition;

    /// @brief Reserved mask size for linear modes (VBE 3.0+).
    unsigned char  linRsvdMaskSize;

    /// @brief Reserved field position for linear modes (VBE 3.0+).
    unsigned char  linRsvdFieldPosition;

    /// @brief Maximum pixel clock frequency in Hz (VBE 3.0+).
    unsigned int   maxPixelClock;

    /// @brief Reserved for future expansion.
    unsigned char  reserved4[189];
};

#pragma pack(pop)

/******************************************************************************/
/*    VBE Constants                                                           */
/******************************************************************************/

/// @brief VBE function success return value (AL=4Fh, AH=00h).
const unsigned short VBE_SUCCESS = 0x004F;

/// @brief Mode flag to enable linear framebuffer access (OR with mode number).
const unsigned short VBE_MODE_LFB = 0x4000;

/// @brief Mode flag to preserve display memory contents (don't clear screen).
const unsigned short VBE_MODE_CLEAR = 0x8000;

/// @brief VBE mode number for 640x480 with 256 colors (8bpp).
const unsigned short VBE_MODE_640x480x8    = 0x101;

/// @brief VBE mode number for 800x600 with 256 colors (8bpp).
const unsigned short VBE_MODE_800x600x8    = 0x103;

/// @brief VBE mode number for 1024x768 with 256 colors (8bpp).
const unsigned short VBE_MODE_1024x768x8   = 0x105;

/// @brief VBE mode number for 640x480 with 65K colors (16bpp).
const unsigned short VBE_MODE_640x480x16   = 0x111;

/// @brief VBE mode number for 800x600 with 65K colors (16bpp).
const unsigned short VBE_MODE_800x600x16   = 0x114;

/// @brief VBE mode number for 640x480 with 16M colors (24bpp).
const unsigned short VBE_MODE_640x480x24   = 0x112;

/// @brief VBE mode number for 800x600 with 16M colors (24bpp).
const unsigned short VBE_MODE_800x600x24   = 0x115;

/// @brief VBE mode number for 640x480 with 32bpp (often same as 24bpp mode).
const unsigned short VBE_MODE_640x480x32   = 0x112;

/// @brief VBE mode number for 800x600 with 32bpp (may need probing).
const unsigned short VBE_MODE_800x600x32   = 0x115;

/// @brief Mode attribute flag: mode is supported by hardware.
const unsigned short VBE_ATTR_SUPPORTED    = 0x0001;

/// @brief Mode attribute flag: linear framebuffer is available.
const unsigned short VBE_ATTR_LFB_AVAIL    = 0x0080;

/// @brief VBE 3.0 gamma function: set gamma correction table.
const unsigned char VBE_GAMMA_SET          = 0x01;

/// @brief VBE 3.0 gamma function: get current gamma table.
const unsigned char VBE_GAMMA_GET          = 0x02;

/// @brief Size of gamma table in bytes (256 entries x 3 channels).
const int VBE_GAMMA_TABLE_SIZE = 256 * 3;

/******************************************************************************/
/*    Graphics class                                                          */
/******************************************************************************/

/// @brief VBE linear framebuffer surface information.
///
/// This structure holds the mapping information for a VBE linear framebuffer,
/// including the LDT selector for protected mode access, dimensions, and
/// pixel format. It is populated by SetVBEMode() and should be passed to
/// CleanupVBE() when the mode is no longer needed.
///
/// @note Use movedata() with the selector field to copy pixel data to the
///       framebuffer. Do not use near pointers as they may cause page faults
///       with some DPMI hosts.
struct VbeSurface
{
    /// @brief LDT selector for accessing the linear framebuffer via movedata().
    int selector;

    /// @brief Linear (physical) address of the mapped framebuffer.
    unsigned long linearAddr;

    /// @brief Size of the mapped framebuffer region in bytes.
    unsigned long size;

    /// @brief Bytes per horizontal scan line (pitch), may include padding.
    unsigned int pitch;

    /// @brief Horizontal resolution in pixels.
    unsigned short width;

    /// @brief Vertical resolution in pixels.
    unsigned short height;

    /// @brief Bits per pixel (color depth: 8, 16, 24, or 32).
    unsigned char bpp;

    /// @brief True if the surface is valid and ready for rendering.
    bool valid;
};

/// @brief Low-level graphics functions for VGA and VBE video modes.
///
/// The Graphics class provides static methods for direct hardware access to
/// VGA registers and VESA BIOS Extensions (VBE). It supports both legacy VGA
/// modes (text mode, 640x480x16 planar) and high-resolution VBE modes with
/// linear framebuffer access.
///
/// @note All methods are static as this class represents hardware state.
///       There is only one display adapter, so no instances are needed.
///
/// @see VbeInfoBlock, VbeModeInfoBlock, VbeSurface
class Graphics
{
public:
    /// @brief Set the VGA video mode using INT 10h AH=00h.
    /// @param mode VGA mode number (e.g., 0x03 = text, 0x12 = 640x480x16, 0x13 = 320x200x256).
    static void SetVideoMode(unsigned char mode);

    /// @brief Get the current VGA video mode using INT 10h AH=0Fh.
    /// @return Current VGA mode number.
    static unsigned char GetVideoMode();

    /// @brief Wait for the next vertical retrace (vsync) period.
    ///
    /// Busy-waits by polling the VGA input status register (port 0x3DA) until
    /// the vertical retrace bit (bit 3) is set. Use this to synchronize buffer
    /// updates with the display refresh to prevent tearing.
    static void WaitForVSync();

    /// @brief Select a VGA bit plane for writing in planar modes.
    ///
    /// In VGA mode 0x12 (640x480x16), each pixel's 4-bit color is spread across
    /// 4 bit planes. This function selects which plane subsequent writes will
    /// affect by programming the VGA sequencer's Map Mask register.
    ///
    /// @param plane Plane number (0-3) to select for writing.
    static void SelectPlane(int plane);

    /// @brief Copy data to VGA video memory using protected mode transfer.
    /// @param data Pointer to source data buffer.
    /// @param offset Byte offset within VGA memory (from 0xA0000).
    /// @param length Number of bytes to copy.
    static void CopyToVGA(const void* data, unsigned int offset, unsigned int length);

    /// @brief Write a byte value to a VGA I/O port.
    /// @param port I/O port address (e.g., 0x3C4 = sequencer index).
    /// @param value Byte value to write.
    static void OutPort(unsigned short port, unsigned char value);

    /// @brief Read a byte value from a VGA I/O port.
    /// @param port I/O port address (e.g., 0x3DA = input status 1).
    /// @return Byte value read from the port.
    static unsigned char InPort(unsigned short port);

    /// @brief Detect VBE presence and get controller information.
    ///
    /// Calls INT 10h AX=4F00h to detect VESA BIOS Extensions and retrieve
    /// information about the graphics controller including available memory
    /// and supported modes.
    ///
    /// @param info Pointer to VbeInfoBlock to receive controller information.
    /// @return True if VBE 2.0 or higher is available, false otherwise.
    static bool DetectVBE(VbeInfoBlock* info);

    /// @brief Get detailed information about a specific VBE mode.
    ///
    /// Calls INT 10h AX=4F01h to retrieve mode-specific information including
    /// resolution, color depth, memory layout, and linear framebuffer address.
    ///
    /// @param mode VBE mode number to query (e.g., VBE_MODE_800x600x24).
    /// @param info Pointer to VbeModeInfoBlock to receive mode information.
    /// @return True if the mode information was retrieved successfully.
    static bool GetVBEModeInfo(unsigned short mode, VbeModeInfoBlock* info);

    /// @brief Set a VBE video mode with linear framebuffer.
    ///
    /// Calls INT 10h AX=4F02h to set the video mode, then maps the linear
    /// framebuffer into the process address space using DPMI. The VbeSurface
    /// structure is populated with the mapping information needed for rendering.
    ///
    /// @param mode VBE mode number (VBE_MODE_LFB is automatically added).
    /// @param surface Pointer to VbeSurface to receive framebuffer mapping info.
    /// @return True if the mode was set and LFB mapped successfully.
    static bool SetVBEMode(unsigned short mode, VbeSurface* surface);

    /// @brief Clean up VBE resources and release the framebuffer mapping.
    ///
    /// Frees the LDT descriptor and physical memory mapping allocated by
    /// SetVBEMode(). Should be called before exiting or switching to a
    /// different video mode.
    ///
    /// @param surface Pointer to VbeSurface to clean up (valid flag set to false).
    static void CleanupVBE(VbeSurface* surface);

    /// @brief Get the LDT selector for linear framebuffer access.
    ///
    /// Returns the selector allocated by SetVBEMode() for use with far pointer
    /// functions like _farsetsel(), _farnspokeb(), or movedata().
    ///
    /// @return LDT selector for LFB access, or 0 if no VBE mode is active.
    static int GetLfbSelector();

    /// @brief Check if VBE 3.0 gamma control is supported.
    ///
    /// Tests whether the VBE implementation supports INT 10h AX=4F15h gamma
    /// ramp functions for hardware brightness/contrast control.
    ///
    /// @return True if gamma control is available, false otherwise.
    /// @note QEMU's vgabios only implements VBE 2.0; this returns false in QEMU.
    static bool IsGammaSupported();

    /// @brief Set the VBE 3.0 gamma correction table.
    ///
    /// Programs the DAC gamma lookup table to adjust display brightness and
    /// contrast. The table contains 256 entries for each of R, G, B channels
    /// (768 bytes total), mapping input intensity to output intensity.
    ///
    /// @param gammaTable Pointer to 768-byte gamma table (256 R, 256 G, 256 B).
    /// @return True if the gamma table was set successfully.
    ///
    /// @par Example - Identity (no change):
    /// @code
    /// unsigned char gamma[768];
    /// for (int i = 0; i < 256; i++) {
    ///     gamma[i] = gamma[i+256] = gamma[i+512] = i;
    /// }
    /// Graphics::SetGammaTable(gamma);
    /// @endcode
    ///
    /// @par Example - Fade to 50% brightness:
    /// @code
    /// for (int i = 0; i < 256; i++) {
    ///     gamma[i] = gamma[i+256] = gamma[i+512] = i / 2;
    /// }
    /// @endcode
    static bool SetGammaTable(const unsigned char* gammaTable);

    /// @brief Get the current VBE 3.0 gamma correction table.
    ///
    /// Retrieves the current DAC gamma lookup table values.
    ///
    /// @param gammaTable Pointer to 768-byte buffer to receive gamma values.
    /// @return True if the gamma table was retrieved successfully.
    static bool GetGammaTable(unsigned char* gammaTable);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_GRAPHICS_HPP
