/// @file Drawing.hpp
/// @brief System.Drawing namespace - Graphics primitives, images, fonts, and rendering.
///
/// This file contains the core graphics types for the WinDOS GUI framework:
/// - Color: 32-bit ARGB color representation
/// - Point, Size, Rectangle: Geometric primitives
/// - Image: 32-bit ARGB bitmap images with file loading support
/// - Font: Bitmap (FON) and TrueType (TTF) font rendering
/// - Graphics: Drawing context for rendering primitives, text, and images
/// - GraphicsBuffer: Framebuffer management for VGA and VBE modes
/// - HatchStyle: Fill patterns for hatched brushes
/// - SystemIcons: Named constants for system icon library
///
/// @note All colors and images use 32-bit ARGB format internally. For low-color
/// display modes (4bpp/8bpp VGA), content is dithered at render time.

#ifndef SYSTEM_DRAWING_HPP
#define SYSTEM_DRAWING_HPP

#include "../Types.hpp"
#include "../Array.hpp"
#include "../Exception.hpp"
#include "Enums.hpp"
#include "Color.hpp"
#include "Point.hpp"
#include "Size.hpp"
#include "Rectangle.hpp"

namespace System::Drawing
{

/******************************************************************************/
/*    BMP File Structures (packed for binary compatibility)                   */
/******************************************************************************/

#pragma pack(push, 1)

/// @brief BMP file header structure (14 bytes).
/// @details Binary-compatible structure for reading BMP file headers.
/// Uses pragma pack to ensure correct field alignment.
struct BitmapFileHeader
{
    unsigned short type;        ///< File type signature, 'BM' = 0x4D42
    unsigned int   size;        ///< Total file size in bytes
    unsigned short reserved1;   ///< Reserved, must be 0
    unsigned short reserved2;   ///< Reserved, must be 0
    unsigned int   offset;      ///< Offset from file start to pixel data

    /// @brief Gets the file type signature.
    /// @return File type as UInt16 (should be 0x4D42 for valid BMP).
    UInt16 Type() const
    {
        return UInt16(type);
    }

    /// @brief Gets the total file size.
    /// @return File size in bytes as UInt32.
    UInt32 Size() const
    {
        return UInt32(size);
    }

    /// @brief Gets the pixel data offset.
    /// @return Offset from file start to pixel data as UInt32.
    UInt32 Offset() const
    {
        return UInt32(offset);
    }
};

/// @brief BMP info header structure (40 bytes for BITMAPINFOHEADER).
/// @details Binary-compatible structure for reading BMP image information.
/// Supports BITMAPINFOHEADER format (40-byte header).
struct BitmapInfoHeader
{
    unsigned int   headerSize;      ///< Size of this header (40 bytes)
    int            width;           ///< Image width in pixels
    int            height;          ///< Image height in pixels (positive = bottom-up)
    unsigned short planes;          ///< Number of color planes (must be 1)
    unsigned short bitCount;        ///< Bits per pixel (1, 4, 8, 24, or 32)
    unsigned int   compression;     ///< Compression type (0 = BI_RGB uncompressed)
    unsigned int   imageSize;       ///< Size of image data (may be 0 for BI_RGB)
    int            xPixelsPerMeter; ///< Horizontal resolution
    int            yPixelsPerMeter; ///< Vertical resolution
    unsigned int   usedColors;      ///< Number of colors in palette (0 = max)
    unsigned int   importantColors; ///< Number of important colors (0 = all)

    /// @brief Gets the header size.
    /// @return Header size in bytes as UInt32.
    UInt32 HeaderSize() const
    {
        return UInt32(headerSize);
    }

    /// @brief Gets the image width.
    /// @return Width in pixels as Int32.
    Int32 Width() const
    {
        return Int32(width);
    }

    /// @brief Gets the image height.
    /// @return Height in pixels as Int32. Positive = bottom-up, negative = top-down.
    Int32 Height() const
    {
        return Int32(height);
    }

    /// @brief Gets the number of color planes.
    /// @return Number of planes as UInt16 (always 1).
    UInt16 Planes() const
    {
        return UInt16(planes);
    }

    /// @brief Gets the bits per pixel.
    /// @return Bit depth as UInt16 (1, 4, 8, 24, or 32).
    UInt16 BitCount() const
    {
        return UInt16(bitCount);
    }

    /// @brief Gets the compression type.
    /// @return Compression type as UInt32 (0 = uncompressed).
    UInt32 Compression() const
    {
        return UInt32(compression);
    }

    /// @brief Gets the image data size.
    /// @return Size of pixel data in bytes as UInt32.
    UInt32 ImageSize() const
    {
        return UInt32(imageSize);
    }

    /// @brief Gets the number of colors used in the palette.
    /// @return Number of palette entries as UInt32.
    UInt32 UsedColors() const
    {
        return UInt32(usedColors);
    }
};

#pragma pack(pop)

// Forward declarations
class Graphics;
class GraphicsBuffer;
class Image;

/******************************************************************************/
/*    System::Drawing::Image                                                  */
/*                                                                            */
/*    Unified 32-bit image class. All pixels are stored as 32-bit ARGB.       */
/*    For low-color display modes, images are dithered at render time.        */
/******************************************************************************/

/// @brief Represents a 32-bit ARGB bitmap image.
///
/// All pixels are stored as 32-bit ARGB values (0xAARRGGBB), matching the
/// Color class format. Images can be loaded from various file formats or
/// created programmatically. For low-color display modes (4bpp/8bpp VGA),
/// images are dithered at render time.
///
/// @par Supported file formats
/// - BMP: 4bpp, 8bpp, 24bpp, 32bpp (native loader)
/// - PNG: All bit depths via stb_image
/// - JPEG: All bit depths via stb_image
/// - GIF, TGA, PSD: Via stb_image
/// - ICO: Standalone icon files
/// - ICL/DLL/EXE: Icon resources from PE files
///
/// @par Example
/// @code
/// // Load from file (auto-detects format)
/// Image splash = Image::FromFile("C:\\SPLASH.PNG");
///
/// // Create and fill manually
/// Image img(100, 100, Color::Blue);
/// img.SetPixel(50, 50, Color::White);
///
/// // Copy with alpha transparency
/// Image overlay = Image::FromFile("C:\\OVERLAY.PNG");
/// img.CopyFromWithAlpha(overlay, 10, 10);
///
/// // Scale to new size
/// Image scaled = splash.ScaleTo(320, 240);
/// @endcode
class Image
{
    unsigned int* _data;  ///< ARGB pixels (4 bytes per pixel)
    int _width;           ///< Width in pixels
    int _height;          ///< Height in pixels

    void _allocate(Int32 w, Int32 h, UInt32 fill);
    void _free();
    void _copy(const Image& other);

public:
    /// @brief Constructs an empty image with no pixel data.
    Image();

    /// @brief Constructs an image with the specified dimensions.
    /// @param width Width in pixels.
    /// @param height Height in pixels.
    /// @param fillColor Initial fill color (default: black).
    Image(Int32 width, Int32 height, const Color& fillColor = Color::Black);

    /// @brief Constructs an image from a Size.
    /// @param size Width and height.
    /// @param fillColor Initial fill color (default: black).
    Image(const Size& size, const Color& fillColor = Color::Black);

    /// @brief Copy constructor (deep copy).
    /// @param other The image to copy.
    Image(const Image& other);

    /// @brief Move constructor.
    /// @param other The image to move from.
    Image(Image&& other) noexcept;

    /// @brief Destructor, frees pixel data.
    ~Image();

    /// @brief Copy assignment operator (deep copy).
    /// @param other The image to copy.
    /// @return Reference to this image.
    Image& operator=(const Image& other);

    /// @brief Move assignment operator.
    /// @param other The image to move from.
    /// @return Reference to this image.
    Image& operator=(Image&& other) noexcept;

    /// @brief Gets the image width.
    /// @return Width in pixels.
    Int32 Width() const
    {
        return Int32(_width);
    }

    /// @brief Gets the image height.
    /// @return Height in pixels.
    Int32 Height() const
    {
        return Int32(_height);
    }

    /// @brief Gets the image dimensions as a Size.
    /// @return Size containing (width, height).
    Size GetSize() const
    {
        return Size(Int32(_width), Int32(_height));
    }

    /// @brief Gets the total number of pixels.
    /// @return Width * height.
    Int32 Length() const
    {
        return Int32(_width * _height);
    }

    /// @brief Gets the total size of pixel data in bytes.
    /// @return Width * height * 4 (4 bytes per pixel).
    Int32 ByteLength() const
    {
        return Int32(_width * _height * 4);
    }

    /// @brief Gets a pointer to the raw pixel data.
    /// @return Pointer to ARGB pixel array.
    ///
    /// @warning Direct pixel manipulation bypasses bounds checking.
    unsigned int* Data()
    {
        return _data;
    }

    /// @brief Gets a const pointer to the raw pixel data.
    /// @return Const pointer to ARGB pixel array.
    const unsigned int* Data() const
    {
        return _data;
    }

    /// @brief Gets the color of a pixel.
    /// @param x X coordinate.
    /// @param y Y coordinate.
    /// @return Color at the specified position.
    /// @throws IndexOutOfRangeException if coordinates are out of bounds.
    Color GetPixel(Int32 x, Int32 y) const;

    /// @brief Sets the color of a pixel.
    /// @param x X coordinate.
    /// @param y Y coordinate.
    /// @param color Color to set.
    /// @throws IndexOutOfRangeException if coordinates are out of bounds.
    void SetPixel(Int32 x, Int32 y, const Color& color);

    /// @brief Sets the color of a pixel.
    /// @param pt Point coordinates.
    /// @param color Color to set.
    /// @throws IndexOutOfRangeException if coordinates are out of bounds.
    void SetPixel(const Point& pt, const Color& color);

    /// @brief Fills the entire image with a color.
    /// @param color Fill color.
    void Clear(const Color& color);

    /// @brief Copies pixels from another image (opaque copy).
    /// @param src Source image.
    /// @param destX Destination X coordinate.
    /// @param destY Destination Y coordinate.
    ///
    /// All pixels are copied, ignoring alpha channel. Source pixels outside
    /// this image's bounds are clipped.
    void CopyFrom(const Image& src, Int32 destX, Int32 destY);

    /// @brief Copies pixels from another image (opaque copy).
    /// @param src Source image.
    /// @param dest Destination position.
    void CopyFrom(const Image& src, const Point& dest);

    /// @brief Copies pixels from another image with alpha blending.
    /// @param src Source image.
    /// @param destX Destination X coordinate.
    /// @param destY Destination Y coordinate.
    ///
    /// Pixels with alpha < 128 are not copied (treated as transparent).
    /// This is useful for sprite rendering where the transparent color
    /// has been converted to alpha = 0.
    void CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY);

    /// @brief Copies pixels from another image with clipping (opaque copy).
    /// @param src Source image.
    /// @param destX Destination X coordinate.
    /// @param destY Destination Y coordinate.
    /// @param clipRect Clip rectangle in destination coordinates.
    ///
    /// Only pixels within the clipRect are copied. Useful for child control
    /// clipping where drawing should be limited to parent bounds.
    void CopyFromClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect);

    /// @brief Copies pixels from another image with alpha blending and clipping.
    /// @param src Source image.
    /// @param destX Destination X coordinate.
    /// @param destY Destination Y coordinate.
    /// @param clipRect Clip rectangle in destination coordinates.
    ///
    /// Combines alpha blending with clip region. Only pixels within clipRect
    /// that have alpha >= 128 are copied.
    void CopyFromWithAlphaClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect);

    /// @brief Extracts a rectangular region as a new image.
    /// @param x Left edge of region.
    /// @param y Top edge of region.
    /// @param width Width of region.
    /// @param height Height of region.
    /// @return New image containing the specified region.
    Image GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const;

    /// @brief Extracts a rectangular region as a new image.
    /// @param rect Region to extract.
    /// @return New image containing the specified region.
    Image GetRegion(const Rectangle& rect) const;

    /// @brief Loads a BMP file from disk.
    /// @param path Path to the BMP file.
    /// @return Loaded image.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid BMP.
    ///
    /// Supports 4bpp, 8bpp, 24bpp, and 32bpp uncompressed BMPs.
    static Image FromBitmap(const char* path);

    /// @brief Loads an icon from a standalone .ico file.
    /// @param path Path to the ICO file.
    /// @param size Desired icon size (finds closest match).
    /// @return Loaded icon image.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid ICO.
    ///
    /// Supported sizes: 16x16, 32x32, 48x48. Use Size::IconSmall/Medium/Large.
    static Image FromIcon(const char* path, const Size& size);

    /// @brief Loads an icon from a PE-based icon library by index.
    /// @param path Path to the icon library (.icl, .dll, .exe).
    /// @param iconIndex Zero-based index of the icon group.
    /// @param size Desired icon size.
    /// @return Loaded icon image, scaled to the requested size.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid PE.
    /// @throws ArgumentOutOfRangeException if iconIndex is out of range.
    static Image FromIconLibrary(const char* path, Int32 iconIndex, const Size& size);

    /// @brief Loads an icon from a PE-based icon library by name.
    /// @param path Path to the icon library (.icl, .dll, .exe).
    /// @param iconName Name of the icon resource (case-insensitive).
    /// @param size Desired icon size.
    /// @return Loaded icon image, scaled to the requested size.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid PE or icon not found.
    static Image FromIconLibrary(const char* path, const char* iconName, const Size& size);

    /// @brief Gets the number of icon groups in a PE-based icon library.
    /// @param path Path to the icon library.
    /// @return Number of icon groups in the library.
    /// @throws FileNotFoundException if the file does not exist.
    static Int32 GetIconLibraryCount(const char* path);

    /// @brief Gets the names of all icons in a PE-based icon library.
    /// @param path Path to the icon library.
    /// @return Array of icon names (empty strings for ID-based icons).
    /// @throws FileNotFoundException if the file does not exist.
    static Array<String> GetIconLibraryNames(const char* path);

    /// @brief Gets the index of a named icon in a PE-based icon library.
    /// @param path Path to the icon library.
    /// @param iconName Name to search for (case-insensitive).
    /// @return Zero-based index, or -1 if not found.
    /// @throws FileNotFoundException if the file does not exist.
    static Int32 GetIconLibraryIndex(const char* path, const char* iconName);

    /// @brief Loads an image from file with auto-detected format.
    /// @param path Path to the image file.
    /// @return Loaded image.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the format is not recognized.
    ///
    /// Detects format by file signature (magic bytes). Supports PNG, JPEG,
    /// GIF, TGA, PSD via stb_image, and BMP via native loader.
    static Image FromFile(const char* path);

    /// @brief Loads a PNG image from file.
    /// @param path Path to the PNG file.
    /// @return Loaded image.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid PNG.
    static Image FromPng(const char* path);

    /// @brief Loads a JPEG image from file.
    /// @param path Path to the JPEG file.
    /// @return Loaded image.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid JPEG.
    static Image FromJpeg(const char* path);

    /// @brief Scales this image to a new size using bilinear interpolation.
    /// @param newWidth Target width in pixels.
    /// @param newHeight Target height in pixels.
    /// @return New scaled image.
    ///
    /// Uses 16.16 fixed-point arithmetic for accurate sub-pixel sampling.
    Image ScaleTo(Int32 newWidth, Int32 newHeight) const;

    /// @brief Scales this image to a new size using bilinear interpolation.
    /// @param newSize Target dimensions.
    /// @return New scaled image.
    Image ScaleTo(const Size& newSize) const;
};

/// @brief Backwards compatibility alias for Image.
/// @deprecated Use Image instead. Will be removed in a future version.
typedef Image Image32;

/******************************************************************************/
/*    System::Drawing::SystemIcons                                            */
/*                                                                            */
/*    Named constants for icons in sysicons.icl. Use these string names with  */
/*    Image::FromIconLibrary(path, name, size) or SystemIcons::Load(name).    */
/******************************************************************************/

/// @brief Provides named constants for icons in the system icon library.
///
/// This class contains 98 named constants corresponding to icons in sysicons.icl.
/// Use these constants with Image::FromIconLibrary() or the convenience method
/// SystemIcons::Load().
///
/// @par Example
/// @code
/// // Load using SystemIcons helper
/// Image computer = SystemIcons::Load(SystemIcons::Computer, Size::IconLarge);
///
/// // Or load directly from any icon library
/// Image folder = Image::FromIconLibrary(
///     SystemIcons::LibraryPath,
///     SystemIcons::FolderOpen,
///     Size(32, 32)
/// );
/// @endcode
///
/// @par Icon Categories
/// - Application icons: AppLogo, AppMsdos, AppWindos, etc.
/// - Recycle bin: BinEmpty, BinFull
/// - Computer/network: Computer, ComputerNet, ComputerSync
/// - Cursors: CursorHand, CursorLoading, CursorPointer
/// - Devices: DeviceKeyboard, DeviceMouse
/// - Dialogs: DialogError, DialogInfo, DialogQuestion, DialogSuccess, DialogWarning
/// - Drives: DriveCdrom, DriveFloppy, DriveHdd, DriveUsb
/// - Files: File, FileAudioMidi, FileImage, FileTxt, etc.
/// - Folders: FolderApps, FolderClosed, FolderDocs, FolderOpen, etc.
/// - UI elements: UiArrowDown, UiCheck, UiRadio, UiScroll, etc.
class SystemIcons
{
public:
    /// @brief Path to the system icon library file.
    static constexpr const char* LibraryPath = "sysicons.icl";

    // Application icons
    static constexpr const char* AppLogo        = "app-logo";       ///< Application logo
    static constexpr const char* AppMixer       = "app-mixer";      ///< Volume mixer app
    static constexpr const char* AppMsdos       = "app-msdos";      ///< MS-DOS prompt
    static constexpr const char* AppWindos      = "app-windos";     ///< WinDOS logo
    static constexpr const char* AppWinfx1      = "app-winfx-1";    ///< Windows effects 1
    static constexpr const char* AppWinfx2      = "app-winfx-2";    ///< Windows effects 2

    // Recycle bin
    static constexpr const char* BinEmpty       = "bin-empty";      ///< Empty recycle bin
    static constexpr const char* BinFull        = "bin-full";       ///< Full recycle bin

    // Computer and network
    static constexpr const char* Computer       = "computer";       ///< Desktop computer
    static constexpr const char* ComputerNet    = "computer-net";   ///< Networked computer
    static constexpr const char* ComputerSync   = "computer-sync";  ///< Syncing computer

    // Cursors
    static constexpr const char* CursorHand     = "cursor-hand";    ///< Hand/link cursor
    static constexpr const char* CursorLoading  = "cursor-loading"; ///< Loading/wait cursor
    static constexpr const char* CursorPointer  = "cursor-pointer"; ///< Standard arrow cursor

    // Devices
    static constexpr const char* DeviceKeyboard = "device-keyboard"; ///< Keyboard device
    static constexpr const char* DeviceMouse    = "device-mouse";    ///< Mouse device

    // Dialog icons (message boxes)
    static constexpr const char* DialogError1   = "dialog-error-1";    ///< Error icon variant 1
    static constexpr const char* DialogError2   = "dialog-error-2";    ///< Error icon variant 2
    static constexpr const char* DialogInfo1    = "dialog-info-1";     ///< Info icon variant 1
    static constexpr const char* DialogInfo2    = "dialog-info-2";     ///< Info icon variant 2
    static constexpr const char* DialogQuestion1= "dialog-question-1"; ///< Question icon variant 1
    static constexpr const char* DialogQuestion2= "dialog-question-2"; ///< Question icon variant 2
    static constexpr const char* DialogSuccess1 = "dialog-success-1";  ///< Success icon variant 1
    static constexpr const char* DialogSuccess2 = "dialog-success-2";  ///< Success icon variant 2
    static constexpr const char* DialogWarning1 = "dialog-warning-1";  ///< Warning icon variant 1
    static constexpr const char* DialogWarning2 = "dialog-warning-2";  ///< Warning icon variant 2

    // Display
    static constexpr const char* Display        = "display";           ///< Monitor/display
    static constexpr const char* DisplaySettings1 = "display-settings-1"; ///< Display settings 1
    static constexpr const char* DisplaySettings2 = "display-settings-2"; ///< Display settings 2

    // Drives
    static constexpr const char* DriveCdrom     = "drive-cdrom";   ///< CD-ROM drive
    static constexpr const char* DriveFloppy    = "drive-floppy";  ///< Floppy disk drive
    static constexpr const char* DriveHdd       = "drive-hdd";     ///< Hard disk drive
    static constexpr const char* DriveUsb       = "drive-usb";     ///< USB drive

    // Files
    static constexpr const char* File           = "file";              ///< Generic file
    static constexpr const char* FileAudioMidi  = "file-audio-midi";   ///< MIDI audio file
    static constexpr const char* FileAudioPcm   = "file-audio-pcm";    ///< PCM/WAV audio file
    static constexpr const char* FileBinary     = "file-binary";       ///< Binary file
    static constexpr const char* FileFont       = "file-font";         ///< Font file
    static constexpr const char* FileImage      = "file-image";        ///< Image file
    static constexpr const char* FileIso        = "file-iso";          ///< ISO disc image
    static constexpr const char* FileMedia      = "file-media";        ///< Media file
    static constexpr const char* FileRtf        = "file-rtf";          ///< Rich text file
    static constexpr const char* FileSrcAssembly= "file-src-assembly"; ///< Assembly source
    static constexpr const char* FileSrcBasic   = "file-src-basic";    ///< BASIC source
    static constexpr const char* FileSrcC       = "file-src-c";        ///< C source file
    static constexpr const char* FileSrcCpp     = "file-src-cpp";      ///< C++ source file
    static constexpr const char* FileSrcH       = "file-src-h";        ///< C header file
    static constexpr const char* FileSrcHpp     = "file-src-hpp";      ///< C++ header file
    static constexpr const char* FileSystem     = "file-system";       ///< System file
    static constexpr const char* FileTxt        = "file-txt";          ///< Text file
    static constexpr const char* FileXlChart    = "file-xl-chart";     ///< Excel chart
    static constexpr const char* FileXlSheet    = "file-xl-sheet";     ///< Excel spreadsheet

    // Folders
    static constexpr const char* FolderApps     = "folder-apps";       ///< Applications folder
    static constexpr const char* FolderClosed   = "folder-closed";     ///< Closed folder
    static constexpr const char* FolderDocs     = "folder-docs";       ///< Documents folder
    static constexpr const char* FolderLibrary  = "folder-library";    ///< Library folder
    static constexpr const char* FolderOpen     = "folder-open";       ///< Open folder
    static constexpr const char* FolderOpenFiles= "folder-open-files"; ///< Folder with files

    // Mixer/Sound
    static constexpr const char* Mixer          = "mixer";             ///< Audio mixer
    static constexpr const char* Sound          = "sound";             ///< Sound/speaker

    // Network signal strength
    static constexpr const char* NetworkSignal0 = "network-signal-0";  ///< No signal
    static constexpr const char* NetworkSignal1 = "network-signal-1";  ///< Weak signal
    static constexpr const char* NetworkSignal2 = "network-signal-2";  ///< Medium signal
    static constexpr const char* NetworkSignal3 = "network-signal-3";  ///< Strong signal

    // Overlays (for composite icons)
    static constexpr const char* OverlayError   = "overlay-error";     ///< Error overlay (X)
    static constexpr const char* OverlayShortcut= "overlay-shortcut";  ///< Shortcut arrow
    static constexpr const char* OverlaySuccess = "overlay-success";   ///< Success checkmark
    static constexpr const char* OverlayWarning = "overlay-warning";   ///< Warning overlay

    // Shields (security)
    static constexpr const char* Shield         = "shield";            ///< Shield icon
    static constexpr const char* ShieldDanger   = "shield-danger";     ///< Danger shield
    static constexpr const char* ShieldFull     = "shield-full";       ///< Full protection
    static constexpr const char* ShieldInfo     = "shield-info";       ///< Info shield
    static constexpr const char* ShieldSuccess  = "shield-success";    ///< Success shield
    static constexpr const char* ShieldWarning  = "shield-warning";    ///< Warning shield

    // Storage
    static constexpr const char* StoreCdrom     = "store-cdrom";       ///< CD-ROM media
    static constexpr const char* StoreFloppy    = "store-floppy";      ///< Floppy disk media
    static constexpr const char* StoreUsb       = "store-usb";         ///< USB media

    // Text
    static constexpr const char* TextPwrd       = "text-pwrd";         ///< WordPad/word processor
    static constexpr const char* TextSelect     = "text-select";       ///< Text selection

    // Transfer indicators
    static constexpr const char* TxAsync        = "tx-async";          ///< Async transfer
    static constexpr const char* TxIdle         = "tx-idle";           ///< Idle transfer
    static constexpr const char* TxReceive      = "tx-receive";        ///< Receiving data
    static constexpr const char* TxSend         = "tx-send";           ///< Sending data

    // UI elements
    static constexpr const char* UiArrowDown    = "ui-arrow-down";     ///< Down arrow
    static constexpr const char* UiArrowLeft    = "ui-arrow-left";     ///< Left arrow
    static constexpr const char* UiArrowRight   = "ui-arrow-right";    ///< Right arrow
    static constexpr const char* UiArrowUp      = "ui-arrow-up";       ///< Up arrow
    static constexpr const char* UiCheck0       = "ui-check-0";        ///< Unchecked checkbox
    static constexpr const char* UiCheck1       = "ui-check-1";        ///< Checked checkbox
    static constexpr const char* UiCheck2       = "ui-check-2";        ///< Indeterminate checkbox
    static constexpr const char* UiRadio0       = "ui-radio-0";        ///< Unselected radio
    static constexpr const char* UiRadio1       = "ui-radio-1";        ///< Selected radio
    static constexpr const char* UiScroll       = "ui-scroll";         ///< Scrollbar grip
    static constexpr const char* UiScrollDown   = "ui-scroll-down";    ///< Scroll down
    static constexpr const char* UiScrollLeft   = "ui-scroll-left";    ///< Scroll left
    static constexpr const char* UiScrollRight  = "ui-scroll-right";   ///< Scroll right
    static constexpr const char* UiScrollUp     = "ui-scroll-up";      ///< Scroll up

    /// @brief Loads an icon by name from the system icon library.
    /// @param iconName One of the SystemIcons constants (e.g., SystemIcons::Computer).
    /// @param size Desired icon size.
    /// @return Loaded icon image.
    /// @throws FileNotFoundException if sysicons.icl is not found.
    /// @throws InvalidDataException if the icon name is not found.
    static Image Load(const char* iconName, const Size& size)
    {
        return Image::FromIconLibrary(LibraryPath, iconName, size);
    }
};

/******************************************************************************/
/*    System::Drawing::HatchStyle                                             */
/*                                                                            */
/*    Defines fill patterns for hatched brushes. Each pattern is an 8x8       */
/*    bitmap where 1 bits are drawn in the foreground color and 0 bits        */
/*    are drawn in the background color.                                      */
/******************************************************************************/

/// @brief Defines 8x8 fill patterns for hatched brushes.
///
/// Each HatchStyle contains an 8x8 bitmap pattern where 1 bits are drawn
/// in the foreground color and 0 bits in the background color. Use with
/// Graphics::FillRectangle() to create patterned fills.
///
/// @par Example
/// @code
/// Graphics g(BufferMode::Double, rect);
/// g.FillRectangle(rect, HatchStyle::DiagonalCross, Color::Black, Color::White);
/// @endcode
class HatchStyle
{
    unsigned char _pattern[8];  ///< 8x8 pattern bitmap (1 row per byte)

    /// @brief Constructs a hatch style from 8 pattern bytes.
    HatchStyle(const unsigned char p0, const unsigned char p1,
               const unsigned char p2, const unsigned char p3,
               const unsigned char p4, const unsigned char p5,
               const unsigned char p6, const unsigned char p7)
    {
        _pattern[0] = p0; _pattern[1] = p1; _pattern[2] = p2; _pattern[3] = p3;
        _pattern[4] = p4; _pattern[5] = p5; _pattern[6] = p6; _pattern[7] = p7;
    }

public:
    /// @brief Gets the pattern bit at a position.
    /// @param x X coordinate (will be wrapped to 0-7).
    /// @param y Y coordinate (will be wrapped to 0-7).
    /// @return True for foreground color, false for background color.
    bool GetBit(int x, int y) const
    {
        return (_pattern[y & 7] >> (7 - (x & 7))) & 1;
    }

    /// @brief Gets direct access to the pattern bytes.
    /// @return Pointer to 8-byte pattern array.
    const unsigned char* Pattern() const
    {
        return _pattern;
    }

    // Solid patterns
    static const HatchStyle Solid;           ///< All foreground (solid fill)
    static const HatchStyle Empty;           ///< All background (empty)

    // Horizontal/Vertical lines
    static const HatchStyle Horizontal;      ///< Horizontal lines
    static const HatchStyle Vertical;        ///< Vertical lines
    static const HatchStyle Cross;           ///< Grid (horizontal + vertical)

    // Diagonal lines (forward = top-left to bottom-right)
    static const HatchStyle ForwardDiagonal; ///< Forward diagonal (///)
    static const HatchStyle BackwardDiagonal;///< Backward diagonal (\\\)
    static const HatchStyle DiagonalCross;   ///< Diagonal cross (X pattern)

    // Dot patterns (percentage indicates foreground coverage)
    static const HatchStyle Percent05;       ///< 5% dots
    static const HatchStyle Percent10;       ///< 10% dots
    static const HatchStyle Percent20;       ///< 20% dots
    static const HatchStyle Percent25;       ///< 25% dots
    static const HatchStyle Percent30;       ///< 30% dots
    static const HatchStyle Percent40;       ///< 40% dots
    static const HatchStyle Percent50;       ///< 50% checkerboard
    static const HatchStyle Percent60;       ///< 60% dots
    static const HatchStyle Percent70;       ///< 70% dots
    static const HatchStyle Percent75;       ///< 75% dots
    static const HatchStyle Percent80;       ///< 80% dots
    static const HatchStyle Percent90;       ///< 90% dots

    // Special patterns
    static const HatchStyle LightHorizontal; ///< Thin horizontal lines
    static const HatchStyle LightVertical;   ///< Thin vertical lines
    static const HatchStyle DarkHorizontal;  ///< Thick horizontal lines
    static const HatchStyle DarkVertical;    ///< Thick vertical lines
    static const HatchStyle DashedHorizontal;///< Dashed horizontal lines
    static const HatchStyle DashedVertical;  ///< Dashed vertical lines
    static const HatchStyle SmallGrid;       ///< Small grid pattern
    static const HatchStyle LargeGrid;       ///< Large grid pattern
    static const HatchStyle DottedGrid;      ///< Dotted grid pattern
    static const HatchStyle DottedDiamond;   ///< Dotted diamond pattern
    static const HatchStyle Brick;           ///< Brick wall pattern
    static const HatchStyle Weave;           ///< Weave pattern
    static const HatchStyle Trellis;         ///< Trellis pattern
    static const HatchStyle Sphere;          ///< Sphere/circle pattern
    static const HatchStyle Wave;            ///< Wave pattern
    static const HatchStyle ZigZag;          ///< Zig-zag pattern
    static const HatchStyle Shingle;         ///< Shingle pattern
    static const HatchStyle Plaid;           ///< Plaid pattern
};

/******************************************************************************/
/*    System::Drawing::Font                                                   */
/*                                                                            */
/*    Represents a font for rendering text. Supports Windows FON bitmap fonts.*/
/*    Use Font::FromFile() to load fonts, or Font::SystemFont()/FixedFont()   */
/*    for built-in fonts.                                                     */
/******************************************************************************/

/// @brief Represents a font for rendering text.
///
/// Supports both Windows FON bitmap fonts (NE format) and TrueType fonts (TTF).
/// Bitmap fonts are loaded from .FON files and provide crisp rendering at specific
/// sizes. TrueType fonts are rasterized at load time using stb_truetype.
///
/// @par Example
/// @code
/// // Load a TrueType font
/// Font arial = Font::FromTrueType("ARIAL.TTF", 14);
///
/// // Load a bitmap font
/// Font fixed = Font::FromFile("FIXEDSYS.FON", 8);
///
/// // Use system defaults
/// Font sysFont = Font::SystemFont();
///
/// // Measure text
/// Size textSize = arial.MeasureString("Hello World");
/// @endcode
class Font
{
    /// @brief Forward declaration of internal font data structure.
    struct FontData;
    FontData* _data;  ///< Pointer to internal font data

    /// @brief Private constructor from internal data.
    Font(FontData* data);

public:
    /// @brief Constructs an invalid/empty font.
    Font();

    /// @brief Copy constructor (deep copy of glyph data).
    /// @param other The font to copy.
    Font(const Font& other);

    /// @brief Move constructor.
    /// @param other The font to move from.
    Font(Font&& other) noexcept;

    /// @brief Destructor, frees font data.
    ~Font();

    /// @brief Copy assignment operator.
    /// @param other The font to copy.
    /// @return Reference to this font.
    Font& operator=(const Font& other);

    /// @brief Move assignment operator.
    /// @param other The font to move from.
    /// @return Reference to this font.
    Font& operator=(Font&& other) noexcept;

    /// @brief Loads a bitmap font from a FON file (NE format).
    /// @param path Path to the .FON file.
    /// @param size Desired point size (finds closest match in file).
    /// @param style Font style flags.
    /// @return Loaded font.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid FON.
    static Font FromFile(const char* path, Int32 size, FontStyle style = FontStyle::Regular);

    /// @brief Loads a TrueType font from a TTF file.
    /// @param path Path to the .TTF file.
    /// @param pixelHeight Desired font height in pixels.
    /// @param style Font style flags (Bold causes fake bold effect).
    /// @return Loaded font.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid TTF.
    ///
    /// Glyphs are rendered using stb_truetype with sharp threshold rendering
    /// for crisp text. Anti-aliasing is not used to prevent blur at small sizes.
    static Font FromTrueType(const char* path, Int32 pixelHeight, FontStyle style = FontStyle::Regular);

    /// @brief Gets the default system font (MS Sans Serif 8pt).
    /// @return System font.
    /// @throws FileNotFoundException if MSSANS.FON is not found.
    static Font SystemFont();

    /// @brief Gets the bold system font (MS Sans Serif 8pt Bold).
    /// @return Bold system font.
    /// @throws FileNotFoundException if MSSANS.FON is not found.
    ///
    /// @note Uses fake bold effect (not a true bold variant).
    static Font SystemFontBold();

    /// @brief Gets the fixed-width system font (Fixedsys 8pt).
    /// @return Fixed-width font.
    /// @throws FileNotFoundException if FIXEDSYS.FON is not found.
    static Font FixedFont();

    /// @brief Gets the font family name.
    /// @return Font name (e.g., "MS Sans Serif", "Arial").
    String Name() const;

    /// @brief Gets the point size.
    /// @return Point size as specified when loading.
    Int32 Size() const;

    /// @brief Gets the line height in pixels.
    /// @return Height from one baseline to the next.
    Int32 Height() const;

    /// @brief Gets the ascent in pixels.
    /// @return Pixels from baseline to top of tallest glyph.
    Int32 Ascent() const;

    /// @brief Gets the font style.
    /// @return FontStyle flags.
    FontStyle Style() const;

    /// @brief Checks if this font is valid and usable.
    /// @return True if font data was loaded successfully.
    Boolean IsValid() const;

    /// @brief Checks if this is a TrueType font.
    /// @return True for TTF fonts, false for bitmap FON fonts.
    Boolean IsTrueType() const;

    /// @brief Gets the internal stb_truetype font info (for TTF fonts only).
    /// @return Pointer to stbtt_fontinfo, or nullptr for non-TTF fonts.
    ///
    /// @warning For internal use only. The returned pointer is owned by the Font.
    void* GetTTFInfo() const;

    /// @brief Gets the TTF scale factor (for TTF fonts only).
    /// @return Scale factor for stbtt functions, or 0 for non-TTF fonts.
    float GetTTFScale() const;

    /// @brief Gets the width of a character in pixels.
    /// @param c Character to measure.
    /// @return Width in pixels.
    Int32 GetCharWidth(Char c) const;

    /// @brief Measures the size of rendered text.
    /// @param text Text to measure.
    /// @return Size in pixels (width, height).
    class Size MeasureString(const String& text) const;

    /// @brief Measures the size of rendered text.
    /// @param text Null-terminated C string to measure.
    /// @return Size in pixels (width, height).
    class Size MeasureString(const char* text) const;

    /// @brief Gets the cached glyph bitmap for a character.
    /// @param c Character to get glyph for.
    /// @return Image containing glyph (white on transparent).
    ///
    /// @warning For internal use by Graphics::DrawString.
    const Image& GetGlyph(Char c) const;
};

/******************************************************************************/
/*    System::Drawing::BufferWriter (function pointer type)                   */
/******************************************************************************/

/// @brief Function pointer type for flushing graphics buffer to display.
/// @param buffer The buffer to flush to the display.
typedef void (*BufferWriter)(const GraphicsBuffer& buffer);

/******************************************************************************/
/*    System::Drawing::GraphicsBuffer                                         */
/*                                                                            */
/*    Unified graphics buffer. All rendering is done to a 32-bit Image.       */
/*    For low-color display modes, content is dithered when flushed.          */
/******************************************************************************/

/// @brief Graphics buffer for rendering and display.
///
/// Manages a 32-bit ARGB image buffer and handles flushing to the display
/// hardware. For VGA modes (4bpp/8bpp), content is dithered using Bayer
/// dithering when flushed. For VBE modes (24bpp/32bpp), content is copied
/// directly to the linear framebuffer.
///
/// @par Static Framebuffer
/// The static framebuffer is shared by all graphics operations. Create it
/// once at startup using CreateFrameBuffer() or CreateFrameBuffer32(), and
/// flush periodically using FlushFrameBuffer().
///
/// @par Example
/// @code
/// // Initialize VGA mode 12h (640x480 16-color)
/// Display::SetMode(VideoMode::VGA640x480);
/// GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
///
/// // Draw to framebuffer
/// GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
/// fb->GetImage().Clear(Color::Blue);
///
/// // Flush to screen
/// GraphicsBuffer::FlushFrameBuffer();
/// @endcode
class GraphicsBuffer
{
    BufferWriter _writer;       ///< Function to flush buffer to display
    Rectangle _bounds;          ///< Buffer dimensions
    Image _image;               ///< 32-bit ARGB image buffer
    unsigned int _lfbPitch;     ///< Bytes per scanline for LFB
    unsigned char _bpp;         ///< Display bits per pixel (4, 8, 24, or 32)
    unsigned char _videoMode;   ///< VGA mode (0x12, 0x13) or 0 for VBE

    static GraphicsBuffer* _frameBuffer;  ///< Static framebuffer instance
    static void* _lfbAddress;             ///< Mapped linear framebuffer address
    static unsigned int _lfbSize;         ///< Size of mapped LFB

    /// @brief Private constructor (use factory methods).
    GraphicsBuffer(BufferWriter writer, const Rectangle& bounds, unsigned char bpp, unsigned char videoMode);

public:
    /// @brief Destructor.
    ~GraphicsBuffer();

    /// @brief Gets the buffer dimensions.
    /// @return Rectangle describing buffer position and size.
    const Rectangle& Bounds() const
    {
        return _bounds;
    }

    /// @brief Gets the internal image buffer for direct access.
    /// @return Reference to the 32-bit ARGB image.
    Image& GetImage()
    {
        return _image;
    }

    /// @brief Gets the internal image buffer (const version).
    /// @return Const reference to the 32-bit ARGB image.
    const Image& GetImage() const
    {
        return _image;
    }

    /// @brief Gets the linear framebuffer pitch (VBE modes only).
    /// @return Bytes per scanline.
    UInt32 LfbPitch() const
    {
        return UInt32(_lfbPitch);
    }

    /// @brief Gets the display bits per pixel.
    /// @return Bits per pixel (4, 8, 24, or 32).
    UInt8 Bpp() const
    {
        return UInt8(_bpp);
    }

    /// @brief Gets the VGA video mode.
    /// @return VGA mode (0x12, 0x13) or 0 for VBE mode.
    UInt8 VideoMode() const
    {
        return UInt8(_videoMode);
    }

    /// @brief Checks if this buffer is using VBE mode.
    /// @return True for VBE modes, false for VGA modes.
    Boolean IsVbeMode() const
    {
        return Boolean(_videoMode == 0);
    }

    /// @brief Marks the buffer as needing flush.
    void Invalidate();

    /// @brief Creates the static framebuffer for VGA modes.
    /// @param width Buffer width in pixels.
    /// @param height Buffer height in pixels.
    /// @param videoMode VGA mode number (0x12 for 16-color, 0x13 for 256-color).
    ///
    /// Call this after setting the video mode with Display::SetMode().
    static void CreateFrameBuffer(Int32 width, Int32 height, UInt8 videoMode);

    /// @brief Creates the static framebuffer for VBE modes.
    /// @param width Buffer width in pixels.
    /// @param height Buffer height in pixels.
    /// @param vbeMode VBE mode number.
    /// @param lfbAddr Physical address of the linear framebuffer.
    /// @param pitch Bytes per scanline in the LFB.
    /// @param bpp Bits per pixel (24 or 32).
    ///
    /// Call this after setting the video mode with Display::SetMode() for VBE modes.
    static void CreateFrameBuffer32(Int32 width, Int32 height, UInt16 vbeMode,
                                    void* lfbAddr, UInt32 pitch, UInt8 bpp);

    /// @brief Destroys the static framebuffer and frees resources.
    static void DestroyFrameBuffer();

    /// @brief Flushes the static framebuffer to the display.
    ///
    /// For VGA modes, applies Bayer dithering and writes to video memory.
    /// For VBE modes, copies directly to the linear framebuffer.
    static void FlushFrameBuffer();

    /// @brief Gets the static framebuffer instance.
    /// @return Pointer to framebuffer, or nullptr if not created.
    static GraphicsBuffer* GetFrameBuffer();

    /// @brief Creates a graphics buffer for a specific region.
    /// @param mode Buffering mode (Single or Double).
    /// @param bounds Buffer dimensions.
    /// @return Pointer to new buffer (caller owns).
    static GraphicsBuffer* Create(BufferMode mode, const Rectangle& bounds);

    /// @brief Gets the mapped linear framebuffer address.
    /// @return LFB address for VBE modes, nullptr otherwise.
    static void* GetLFBAddress()
    {
        return _lfbAddress;
    }
};

/******************************************************************************/
/*    System::Drawing::Graphics                                               */
/*                                                                            */
/*    Graphics drawing context. All drawing operations use 32-bit colors.     */
/******************************************************************************/

/// @brief Provides methods for drawing graphics primitives, text, and images.
///
/// Graphics objects provide a drawing surface with methods for rendering
/// lines, rectangles, text, and images. All drawing uses 32-bit ARGB colors.
///
/// @par Example
/// @code
/// // Create a graphics context
/// Graphics g(BufferMode::Double, Rectangle(0, 0, 640, 480));
///
/// // Clear background
/// g.Clear(Color::DarkBlue);
///
/// // Draw shapes
/// g.FillRectangle(10, 10, 100, 50, Color::Red);
/// g.DrawRectangle(10, 10, 100, 50, Color::White);
///
/// // Draw text
/// Font font = Font::SystemFont();
/// g.DrawString("Hello World", font, Color::Yellow, 20, 70);
///
/// // Flush to screen
/// g.Invalidate(true);
/// @endcode
class Graphics
{
    GraphicsBuffer* _buffer;  ///< Associated buffer
    Rectangle _bounds;        ///< Drawing area bounds
    bool _ownsBuffer;         ///< True if this Graphics owns the buffer

public:
    /// @brief Creates a graphics context with the specified buffering mode.
    /// @param mode Buffering mode (Single or Double).
    /// @param bounds Drawing area dimensions.
    Graphics(BufferMode mode, const Rectangle& bounds);

    /// @brief Creates a graphics context with the specified buffering mode.
    /// @param mode Buffering mode (Single or Double).
    /// @param x Left edge of drawing area.
    /// @param y Top edge of drawing area.
    /// @param width Width of drawing area.
    /// @param height Height of drawing area.
    Graphics(BufferMode mode, Int32 x, Int32 y, Int32 width, Int32 height);

    /// @brief Destructor, releases buffer if owned.
    ~Graphics();

    /// @brief Gets the drawing area bounds.
    /// @return Rectangle describing the drawing area.
    const Rectangle& Bounds() const
    {
        return _bounds;
    }

    /// @brief Clears the drawing area to a solid color.
    /// @param color Fill color.
    void Clear(const Color& color);

    /// @brief Draws a single pixel.
    /// @param x X coordinate.
    /// @param y Y coordinate.
    /// @param color Pixel color.
    void DrawPixel(Int32 x, Int32 y, const Color& color);

    /// @brief Draws a single pixel.
    /// @param pt Pixel coordinates.
    /// @param color Pixel color.
    void DrawPixel(const Point& pt, const Color& color);

    /// @brief Draws a line between two points.
    /// @param x1 Start X coordinate.
    /// @param y1 Start Y coordinate.
    /// @param x2 End X coordinate.
    /// @param y2 End Y coordinate.
    /// @param color Line color.
    ///
    /// Uses Bresenham's line algorithm for efficient integer-only calculation.
    void DrawLine(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const Color& color);

    /// @brief Draws a line between two points.
    /// @param p1 Start point.
    /// @param p2 End point.
    /// @param color Line color.
    void DrawLine(const Point& p1, const Point& p2, const Color& color);

    /// @brief Draws an unfilled rectangle outline.
    /// @param x Left edge.
    /// @param y Top edge.
    /// @param width Width.
    /// @param height Height.
    /// @param color Line color.
    void DrawRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color);

    /// @brief Draws an unfilled rectangle outline.
    /// @param rect Rectangle bounds.
    /// @param color Line color.
    void DrawRectangle(const Rectangle& rect, const Color& color);

    /// @brief Draws a filled rectangle.
    /// @param x Left edge.
    /// @param y Top edge.
    /// @param width Width.
    /// @param height Height.
    /// @param color Fill color.
    void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color);

    /// @brief Draws a filled rectangle.
    /// @param rect Rectangle bounds.
    /// @param color Fill color.
    void FillRectangle(const Rectangle& rect, const Color& color);

    /// @brief Draws a filled rectangle with a 3D border style.
    /// @param rect Rectangle bounds.
    /// @param style Border style (Raised, Sunken, etc.).
    void FillRectangle(const Rectangle& rect, BorderStyle style);

    /// @brief Draws a filled rectangle with a hatch pattern.
    /// @param x Left edge.
    /// @param y Top edge.
    /// @param width Width.
    /// @param height Height.
    /// @param hatch Pattern style.
    /// @param foreColor Pattern foreground color.
    /// @param backColor Pattern background color.
    void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                       const HatchStyle& hatch, const Color& foreColor, const Color& backColor);

    /// @brief Draws a filled rectangle with a hatch pattern.
    /// @param rect Rectangle bounds.
    /// @param hatch Pattern style.
    /// @param foreColor Pattern foreground color.
    /// @param backColor Pattern background color.
    void FillRectangle(const Rectangle& rect,
                       const HatchStyle& hatch, const Color& foreColor, const Color& backColor);

    /// @brief Draws an image at the specified position.
    /// @param image Image to draw.
    /// @param x Left edge destination.
    /// @param y Top edge destination.
    void DrawImage(const Image& image, Int32 x, Int32 y);

    /// @brief Draws an image at the specified position.
    /// @param image Image to draw.
    /// @param location Destination position.
    void DrawImage(const Image& image, const Point& location);

    /// @brief Draws text at the specified position.
    /// @param text Text to draw.
    /// @param font Font to use.
    /// @param color Text color.
    /// @param x Left edge.
    /// @param y Top edge (baseline depends on font).
    void DrawString(const String& text, const Font& font, const Color& color, Int32 x, Int32 y);

    /// @brief Draws text at the specified position.
    /// @param text Null-terminated C string to draw.
    /// @param font Font to use.
    /// @param color Text color.
    /// @param x Left edge.
    /// @param y Top edge.
    void DrawString(const char* text, const Font& font, const Color& color, Int32 x, Int32 y);

    /// @brief Draws text within a rectangle with alignment.
    /// @param text Text to draw.
    /// @param font Font to use.
    /// @param color Text color.
    /// @param rect Bounding rectangle.
    /// @param hAlign Horizontal alignment (Near=left, Center, Far=right).
    /// @param vAlign Vertical alignment (Near=top, Center, Far=bottom).
    void DrawString(const String& text, const Font& font, const Color& color,
                    const Rectangle& rect, StringAlignment hAlign = StringAlignment::Near,
                    StringAlignment vAlign = StringAlignment::Near);

    /// @brief Measures the size that text would occupy when rendered.
    /// @param text Text to measure.
    /// @param font Font to use.
    /// @return Size in pixels.
    class Size MeasureString(const String& text, const Font& font) const;

    /// @brief Measures the size that text would occupy when rendered.
    /// @param text Null-terminated C string to measure.
    /// @param font Font to use.
    /// @return Size in pixels.
    class Size MeasureString(const char* text, const Font& font) const;

    /// @brief Marks the drawing area as needing update.
    /// @param flushFrameBuffer If true, also flush the framebuffer to display.
    void Invalidate(Boolean flushFrameBuffer = Boolean(false));
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_HPP
