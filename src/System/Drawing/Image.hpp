/// @file Image.hpp
/// @brief System.Drawing.Image - 32-bit ARGB bitmap image class.

#ifndef SYSTEM_DRAWING_IMAGE_HPP
#define SYSTEM_DRAWING_IMAGE_HPP

#include "../Types.hpp"
#include "../Array.hpp"
#include "../String.hpp"
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

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_IMAGE_HPP
