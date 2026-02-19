#include "Drawing.hpp"
#include "../Exception.hpp"
#include "../IO/File.hpp"
#include "../IO/Devices/Display.hpp"
#include "../../ThirdParty/stb_truetype.h"
#include "../../ThirdParty/stb_image.h"
#include <cstdlib>
#include <cstring>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <dpmi.h>

namespace System::Drawing
{

/******************************************************************************/
/*    Planar conversion lookup table                                          */
/*    Pre-computed table for fast chunky-to-planar conversion                 */
/*    Index: 2 pixels packed (p0<<4 | p1) = 256 entries                       */
/*    Each entry: 4 bytes (one per plane), 2 bits set per byte                */
/******************************************************************************/

static unsigned char g_c2p_table[256][4];
static bool g_c2p_initialized = false;

static void InitC2PTable()
{
    if (g_c2p_initialized) return;

    for (Int32 p0 = Int32(0); static_cast<int>(p0) < 16; p0 += 1)
    {
        for (Int32 p1 = Int32(0); static_cast<int>(p1) < 16; p1 += 1)
        {
            Int32 idx = Int32((static_cast<int>(p0) << 4) | static_cast<int>(p1));
            for (Int32 plane = Int32(0); static_cast<int>(plane) < 4; plane += 1)
            {
                g_c2p_table[static_cast<int>(idx)][static_cast<int>(plane)] =
                    (((static_cast<int>(p0) >> static_cast<int>(plane)) & 1) << 1) |
                    ((static_cast<int>(p1) >> static_cast<int>(plane)) & 1);
            }
        }
    }
    g_c2p_initialized = true;
}

/******************************************************************************/
/*    Bayer ordered dithering matrix (4x4)                                    */
/*    Used for converting 32-bit images to 4bpp VGA palette                   */
/******************************************************************************/

static const int g_bayerMatrix[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

// Apply Bayer dithering to find VGA palette index
static unsigned char DitherToVga(Int32 x, Int32 y, unsigned char r, unsigned char g, unsigned char b)
{
    // Get threshold from Bayer matrix (0-15, scaled to color range)
    Int32 threshold = Int32((g_bayerMatrix[static_cast<int>(y) & 3][static_cast<int>(x) & 3] - 8) * 8);  // -64 to +56

    // Apply threshold to each channel
    Int32 rq = Int32(static_cast<int>(r) + static_cast<int>(threshold));
    Int32 gq = Int32(static_cast<int>(g) + static_cast<int>(threshold));
    Int32 bq = Int32(static_cast<int>(b) + static_cast<int>(threshold));

    // Clamp to valid range
    if (static_cast<int>(rq) < 0) rq = Int32(0);
    if (static_cast<int>(rq) > 255) rq = Int32(255);
    if (static_cast<int>(gq) < 0) gq = Int32(0);
    if (static_cast<int>(gq) > 255) gq = Int32(255);
    if (static_cast<int>(bq) < 0) bq = Int32(0);
    if (static_cast<int>(bq) > 255) bq = Int32(255);

    // Find closest VGA color
    return static_cast<unsigned char>(Color::RgbToVgaIndex(static_cast<int>(rq), static_cast<int>(gq), static_cast<int>(bq)));
}

/******************************************************************************/
/*    Image implementation (unified 32-bit ARGB)                              */
/******************************************************************************/

void Image::_allocate(Int32 w, Int32 h, UInt32 fill)
{
    _width = static_cast<int>(w);
    _height = static_cast<int>(h);
    Int32 size = Int32(static_cast<int>(w) * static_cast<int>(h));
    if (static_cast<int>(size) > 0)
    {
        _data = static_cast<unsigned int*>(std::malloc(static_cast<int>(size) * sizeof(unsigned int)));
        if (_data)
        {
            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(size); i += 1)
            {
                _data[static_cast<int>(i)] = static_cast<unsigned int>(fill);
            }
        }
    }
    else
    {
        _data = nullptr;
    }
}

void Image::_free()
{
    if (_data)
    {
        std::free(_data);
        _data = nullptr;
    }
    _width = 0;
    _height = 0;
}

void Image::_copy(const Image& other)
{
    _allocate(Int32(other._width), Int32(other._height), UInt32(0));
    if (_data && other._data)
    {
        std::memcpy(_data, other._data, _width * _height * sizeof(unsigned int));
    }
}

Image::Image() : _data(nullptr), _width(0), _height(0) {}

Image::Image(Int32 width, Int32 height, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0)
{
    _allocate(static_cast<int>(width), static_cast<int>(height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Size& size, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0)
{
    _allocate(static_cast<int>(size.width), static_cast<int>(size.height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Image& other) : _data(nullptr), _width(0), _height(0)
{
    _copy(other);
}

Image::Image(Image&& other) noexcept
    : _data(other._data), _width(other._width), _height(other._height)
{
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
}

Image::~Image()
{
    _free();
}

Image& Image::operator=(const Image& other)
{
    if (this != &other)
    {
        _free();
        _copy(other);
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept
{
    if (this != &other)
    {
        _free();
        _data = other._data;
        _width = other._width;
        _height = other._height;
        other._data = nullptr;
        other._width = 0;
        other._height = 0;
    }
    return *this;
}

Color Image::GetPixel(Int32 x, Int32 y) const
{
    Int32 xi = x;
    Int32 yi = y;
    if (static_cast<int>(xi) < 0 || static_cast<int>(yi) < 0 || static_cast<int>(xi) >= _width || static_cast<int>(yi) >= _height || !_data)
    {
        return Color::Transparent;
    }
    return Color(_data[static_cast<int>(yi) * _width + static_cast<int>(xi)]);
}

void Image::SetPixel(Int32 x, Int32 y, const Color& color)
{
    Int32 xi = x;
    Int32 yi = y;
    if (static_cast<int>(xi) < 0 || static_cast<int>(yi) < 0 || static_cast<int>(xi) >= _width || static_cast<int>(yi) >= _height || !_data)
    {
        return;
    }
    _data[static_cast<int>(yi) * _width + static_cast<int>(xi)] = static_cast<unsigned int>(color);
}

void Image::SetPixel(const Point& pt, const Color& color)
{
    SetPixel(pt.x, pt.y, color);
}

void Image::Clear(const Color& color)
{
    if (_data && _width > 0 && _height > 0)
    {
        UInt32 val = UInt32(static_cast<unsigned int>(color));
        Int32 size = Int32(_width * _height);
        for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(size); i += 1)
        {
            _data[static_cast<int>(i)] = static_cast<unsigned int>(val);
        }
    }
}

void Image::CopyFrom(const Image& src, Int32 destX, Int32 destY)
{
    if (!_data || !src._data) return;

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;

        Int32 srcStartX = Int32(0);
        Int32 dstStartX = dstX;
        Int32 copyWidth = Int32(src._width);

        if (static_cast<int>(dstStartX) < 0)
        {
            srcStartX = Int32(-static_cast<int>(dstStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(dstStartX));
            dstStartX = Int32(0);
        }
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(dstStartX));
        }
        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = _data + static_cast<int>(dy) * _width + static_cast<int>(dstStartX);
        const unsigned int* srcRow = src._data + static_cast<int>(sy) * src._width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
}

void Image::CopyFrom(const Image& src, const Point& dest)
{
    CopyFrom(src, dest.x, dest.y);
}

void Image::CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY)
{
    if (!_data || !src._data) return;

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;

        for (Int32 sx = Int32(0); static_cast<int>(sx) < src._width; sx += 1)
        {
            Int32 dx = Int32(static_cast<int>(dstX) + static_cast<int>(sx));
            if (static_cast<int>(dx) < 0 || static_cast<int>(dx) >= _width) continue;

            UInt32 pixel = UInt32(src._data[static_cast<int>(sy) * src._width + static_cast<int>(sx)]);
            // Only copy if alpha >= 128 (semi-opaque or opaque)
            if ((static_cast<unsigned int>(pixel) >> 24) >= 128)
            {
                _data[static_cast<int>(dy) * _width + static_cast<int>(dx)] = static_cast<unsigned int>(pixel);
            }
        }
    }
}

void Image::CopyFromClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect)
{
    if (!_data || !src._data) return;

    // Get clip bounds
    int clipLeft = static_cast<int>(clipRect.x);
    int clipTop = static_cast<int>(clipRect.y);
    int clipRight = clipLeft + static_cast<int>(clipRect.width);
    int clipBottom = clipTop + static_cast<int>(clipRect.height);

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;
        if (static_cast<int>(dy) < clipTop || static_cast<int>(dy) >= clipBottom) continue;

        Int32 srcStartX = Int32(0);
        Int32 dstStartX = dstX;
        Int32 copyWidth = Int32(src._width);

        // Clip to image bounds
        if (static_cast<int>(dstStartX) < 0)
        {
            srcStartX = Int32(-static_cast<int>(dstStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(dstStartX));
            dstStartX = Int32(0);
        }
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(dstStartX));
        }

        // Clip to clip rectangle (left)
        if (static_cast<int>(dstStartX) < clipLeft)
        {
            Int32 diff = Int32(clipLeft - static_cast<int>(dstStartX));
            srcStartX = Int32(static_cast<int>(srcStartX) + static_cast<int>(diff));
            copyWidth = Int32(static_cast<int>(copyWidth) - static_cast<int>(diff));
            dstStartX = Int32(clipLeft);
        }
        // Clip to clip rectangle (right)
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > clipRight)
        {
            copyWidth = Int32(clipRight - static_cast<int>(dstStartX));
        }

        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = _data + static_cast<int>(dy) * _width + static_cast<int>(dstStartX);
        const unsigned int* srcRow = src._data + static_cast<int>(sy) * src._width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
}

void Image::CopyFromWithAlphaClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect)
{
    if (!_data || !src._data) return;

    // Get clip bounds
    int clipLeft = static_cast<int>(clipRect.x);
    int clipTop = static_cast<int>(clipRect.y);
    int clipRight = clipLeft + static_cast<int>(clipRect.width);
    int clipBottom = clipTop + static_cast<int>(clipRect.height);

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;
        if (static_cast<int>(dy) < clipTop || static_cast<int>(dy) >= clipBottom) continue;

        for (Int32 sx = Int32(0); static_cast<int>(sx) < src._width; sx += 1)
        {
            Int32 dx = Int32(static_cast<int>(dstX) + static_cast<int>(sx));
            if (static_cast<int>(dx) < 0 || static_cast<int>(dx) >= _width) continue;
            if (static_cast<int>(dx) < clipLeft || static_cast<int>(dx) >= clipRight) continue;

            UInt32 pixel = UInt32(src._data[static_cast<int>(sy) * src._width + static_cast<int>(sx)]);
            // Only copy if alpha >= 128 (semi-opaque or opaque)
            if ((static_cast<unsigned int>(pixel) >> 24) >= 128)
            {
                _data[static_cast<int>(dy) * _width + static_cast<int>(dx)] = static_cast<unsigned int>(pixel);
            }
        }
    }
}

Image Image::GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const
{
    Int32 xi = x;
    Int32 yi = y;
    Int32 wi = width;
    Int32 hi = height;

    Image result(width, height, Color::Transparent);
    if (!_data || !result._data) return result;

    for (Int32 dy = Int32(0); static_cast<int>(dy) < static_cast<int>(hi); dy += 1)
    {
        Int32 sy = Int32(static_cast<int>(yi) + static_cast<int>(dy));
        if (static_cast<int>(sy) < 0 || static_cast<int>(sy) >= _height) continue;

        Int32 srcStartX = xi;
        Int32 dstStartX = Int32(0);
        Int32 copyWidth = wi;

        if (static_cast<int>(srcStartX) < 0)
        {
            dstStartX = Int32(-static_cast<int>(srcStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(srcStartX));
            srcStartX = Int32(0);
        }
        if (static_cast<int>(srcStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(srcStartX));
        }
        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = result._data + static_cast<int>(dy) * static_cast<int>(wi) + static_cast<int>(dstStartX);
        const unsigned int* srcRow = _data + static_cast<int>(sy) * _width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
    return result;
}

Image Image::GetRegion(const Rectangle& rect) const
{
    return GetRegion(rect.x, rect.y, rect.width, rect.height);
}

Image Image::FromBitmap(const char* path)
{
    const UInt16 BMP_SIGNATURE = UInt16(0x4D42);  // 'BM'
    const Int32 FILE_HEADER_SIZE = Int32(sizeof(BitmapFileHeader));
    const Int32 INFO_HEADER_SIZE = Int32(sizeof(BitmapInfoHeader));

    // Validate path
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < static_cast<int>(FILE_HEADER_SIZE) + static_cast<int>(INFO_HEADER_SIZE))
    {
        throw InvalidDataException("File is too small to be a valid BMP.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory for file data.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse headers
    const BitmapFileHeader* fileHeader = reinterpret_cast<const BitmapFileHeader*>(fileData);
    if (static_cast<unsigned short>(fileHeader->Type()) != static_cast<unsigned short>(BMP_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("File is not a valid BMP (invalid signature).");
    }

    const BitmapInfoHeader* infoHeader = reinterpret_cast<const BitmapInfoHeader*>(fileData + static_cast<int>(FILE_HEADER_SIZE));
    Int32 bitCount = Int32(static_cast<int>(infoHeader->BitCount()));

    if (static_cast<unsigned int>(infoHeader->Compression()) != 0)
    {
        std::free(fileData);
        throw InvalidDataException("Compressed BMP files are not supported.");
    }

    Int32 width = Int32(static_cast<int>(infoHeader->Width()));
    Int32 height = Int32(static_cast<int>(infoHeader->Height()));

    if (static_cast<int>(width) <= 0 || static_cast<int>(height) <= 0)
    {
        std::free(fileData);
        throw InvalidDataException("BMP has invalid dimensions.");
    }

    // Get pixel data
    const unsigned char* pixelData = fileData + static_cast<unsigned int>(fileHeader->Offset());

    // Create output image (32-bit ARGB)
    Image result(width, height);
    unsigned int* outPixels = result.Data();

    // Handle different bit depths
    if (static_cast<int>(bitCount) == 4)
    {
        // 4bpp - need palette
        UInt32 paletteCount = UInt32(static_cast<unsigned int>(infoHeader->UsedColors()));
        if (static_cast<unsigned int>(paletteCount) == 0) paletteCount = UInt32(16);
        const unsigned char* paletteData = fileData + static_cast<int>(FILE_HEADER_SIZE) +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        Int32 bytesPerLine = Int32((((static_cast<int>(width) + 1) / 2) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char byteVal = row[static_cast<int>(x) / 2];
                unsigned char index = ((static_cast<int>(x) & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                if (index < static_cast<unsigned int>(paletteCount))
                {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else if (static_cast<int>(bitCount) == 8)
    {
        // 8bpp - need palette
        UInt32 paletteCount = UInt32(static_cast<unsigned int>(infoHeader->UsedColors()));
        if (static_cast<unsigned int>(paletteCount) == 0) paletteCount = UInt32(256);
        const unsigned char* paletteData = fileData + static_cast<int>(FILE_HEADER_SIZE) +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        Int32 bytesPerLine = Int32((static_cast<int>(width) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char index = row[static_cast<int>(x)];
                if (index < static_cast<unsigned int>(paletteCount))
                {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else if (static_cast<int>(bitCount) == 24)
    {
        // 24bpp - direct RGB
        Int32 bytesPerLine = Int32(((static_cast<int>(width) * 3) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 3 + 0];
                unsigned char g = row[static_cast<int>(x) * 3 + 1];
                unsigned char r = row[static_cast<int>(x) * 3 + 2];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    }
    else if (static_cast<int>(bitCount) == 32)
    {
        // 32bpp - direct BGRA
        Int32 bytesPerLine = Int32(static_cast<int>(width) * 4);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 4 + 0];
                unsigned char g = row[static_cast<int>(x) * 4 + 1];
                unsigned char r = row[static_cast<int>(x) * 4 + 2];
                unsigned char a = row[static_cast<int>(x) * 4 + 3];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    }
    else
    {
        std::free(fileData);
        throw InvalidDataException("Unsupported BMP bit depth. Supported: 4, 8, 24, 32.");
    }

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    PE Format Structures for Icon Library Parsing                            */
/******************************************************************************/

#pragma pack(push, 1)

struct MsDosExecutableHeader {
    unsigned short signature;      // 'MZ' = 0x5A4D
    unsigned short lastPageBytes;
    unsigned short pageCount;
    unsigned short relocationCount;
    unsigned short headerParagraphs;
    unsigned short minAllocParagraphs;
    unsigned short maxAllocParagraphs;
    unsigned short initialSS;
    unsigned short initialSP;
    unsigned short checksum;
    unsigned short initialIP;
    unsigned short initialCS;
    unsigned short relocationOffset;
    unsigned short overlayNumber;
    unsigned short reserved1[4];
    unsigned short oemId;
    unsigned short oemInfo;
    unsigned short reserved2[10];
    unsigned int   newHeaderOffset;   // Offset to PE header

    unsigned short Signature() const { return signature; }
    unsigned int   NewHeaderOffset() const { return newHeaderOffset; }
};

struct PortableExecutableFileHeader {
    unsigned short machine;
    unsigned short sectionCount;
    unsigned int   timestamp;
    unsigned int   symbolTablePointer;
    unsigned int   symbolCount;
    unsigned short optionalHeaderSize;
    unsigned short characteristics;

    unsigned short SectionCount() const { return sectionCount; }
    unsigned short OptionalHeaderSize() const { return optionalHeaderSize; }
};

struct PortableExecutableDataDirectory {
    unsigned int virtualAddress;
    unsigned int size;

    unsigned int VirtualAddress() const { return virtualAddress; }
    unsigned int Size() const { return size; }
};

struct PortableExecutableOptionalHeader {
    unsigned short magic;
    unsigned char  majorLinkerVersion;
    unsigned char  minorLinkerVersion;
    unsigned int   sizeOfCode;
    unsigned int   sizeOfInitializedData;
    unsigned int   sizeOfUninitializedData;
    unsigned int   entryPointAddress;
    unsigned int   baseOfCode;
    unsigned int   baseOfData;
    unsigned int   imageBase;
    unsigned int   sectionAlignment;
    unsigned int   fileAlignment;
    unsigned short majorOSVersion;
    unsigned short minorOSVersion;
    unsigned short majorImageVersion;
    unsigned short minorImageVersion;
    unsigned short majorSubsystemVersion;
    unsigned short minorSubsystemVersion;
    unsigned int   win32VersionValue;
    unsigned int   sizeOfImage;
    unsigned int   sizeOfHeaders;
    unsigned int   checksum;
    unsigned short subsystem;
    unsigned short dllCharacteristics;
    unsigned int   sizeOfStackReserve;
    unsigned int   sizeOfStackCommit;
    unsigned int   sizeOfHeapReserve;
    unsigned int   sizeOfHeapCommit;
    unsigned int   loaderFlags;
    unsigned int   numberOfRvaAndSizes;
    PortableExecutableDataDirectory dataDirectories[16];

    const PortableExecutableDataDirectory& GetDataDirectory(int index) const {
        return dataDirectories[index];
    }
};

struct PortableExecutableNTHeaders {
    unsigned int signature;   // 'PE\0\0' = 0x00004550
    PortableExecutableFileHeader fileHeader;
    PortableExecutableOptionalHeader optionalHeader;

    unsigned int Signature() const { return signature; }
    const PortableExecutableFileHeader& GetFileHeader() const { return fileHeader; }
    const PortableExecutableOptionalHeader& GetOptionalHeader() const { return optionalHeader; }
};

struct PortableExecutableSectionHeader {
    char name[8];
    unsigned int virtualSize;
    unsigned int virtualAddress;
    unsigned int sizeOfRawData;
    unsigned int pointerToRawData;
    unsigned int pointerToRelocations;
    unsigned int pointerToLinenumbers;
    unsigned short numberOfRelocations;
    unsigned short numberOfLinenumbers;
    unsigned int characteristics;

    unsigned int VirtualAddress() const { return virtualAddress; }
    unsigned int VirtualSize() const { return virtualSize; }
    unsigned int RawDataPointer() const { return pointerToRawData; }
};

struct PortableExecutableResourceDirectory {
    unsigned int characteristics;
    unsigned int timestamp;
    unsigned short majorVersion;
    unsigned short minorVersion;
    unsigned short namedEntryCount;
    unsigned short idEntryCount;

    unsigned short NamedEntryCount() const { return namedEntryCount; }
    unsigned short IdEntryCount() const { return idEntryCount; }
    unsigned short TotalEntries() const { return namedEntryCount + idEntryCount; }
};

struct PortableExecutableResourceDirectoryEntry {
    unsigned int nameOrId;
    unsigned int offsetToData;

    bool IsNamed() const { return (nameOrId & 0x80000000) != 0; }
    unsigned int GetId() const { return nameOrId & 0x7FFFFFFF; }
    unsigned int GetNameOffset() const { return nameOrId & 0x7FFFFFFF; }
    bool IsDirectory() const { return (offsetToData & 0x80000000) != 0; }
    unsigned int GetOffsetToData() const { return offsetToData & 0x7FFFFFFF; }
};

struct PortableExecutableResourceDataEntry {
    unsigned int dataRva;
    unsigned int size;
    unsigned int codepage;
    unsigned int reserved;

    unsigned int DataRva() const { return dataRva; }
    unsigned int Size() const { return size; }
};

/******************************************************************************/
/*    Icon File Format Structures                                              */
/******************************************************************************/

struct IconDirectory {
    unsigned short reserved;    // Always 0
    unsigned short type;        // 1=ICO, 2=CUR
    unsigned short count;       // Number of images

    unsigned short Type() const { return type; }
    unsigned short Count() const { return count; }
};

struct IconDirectoryHeader {
    unsigned char  width;       // 0 means 256
    unsigned char  height;      // 0 means 256
    unsigned char  colorCount;  // Number of colors
    unsigned char  reserved;
    unsigned short planes;
    unsigned short bitCount;
    unsigned int   size;        // Size of image data

    int Width() const { return width ? width : 256; }
    int Height() const { return height ? height : 256; }
    unsigned short BitCount() const { return bitCount; }
    unsigned int Size() const { return size; }
};

struct IconDirectoryEntry {
    IconDirectoryHeader header;
    unsigned int offset;        // Offset to image data in file

    int Width() const { return header.Width(); }
    int Height() const { return header.Height(); }
    unsigned short BitCount() const { return header.BitCount(); }
    unsigned int Size() const { return header.Size(); }
    unsigned int Offset() const { return offset; }
};

struct GroupIconDirectoryEntry {
    IconDirectoryHeader header;
    unsigned short identifier; // Resource ID

    int Width() const { return header.Width(); }
    int Height() const { return header.Height(); }
    unsigned short BitCount() const { return header.BitCount(); }
    unsigned int Size() const { return header.Size(); }
    unsigned short Identifier() const { return identifier; }
};

/******************************************************************************/
/*    NE (New Executable) Format Structures for FON file parsing              */
/******************************************************************************/

struct NeHeader {
    unsigned short signature;           // 'NE' = 0x454E
    unsigned char  majorLinkerVersion;
    unsigned char  minorLinkerVersion;
    unsigned short entryTableOffset;
    unsigned short entryTableLength;
    unsigned int   fileCRC;
    unsigned short flags;
    unsigned short autoDataSegment;
    unsigned short initialHeapSize;
    unsigned short initialStackSize;
    unsigned short csIp;
    unsigned short sssSp;
    unsigned short segmentTableEntries;
    unsigned short moduleReferenceEntries;
    unsigned short nonResidentNameTableSize;
    unsigned short segmentTableOffset;
    unsigned short resourceTableOffset;
    unsigned short residentNameTableOffset;
    unsigned short moduleReferenceTableOffset;
    unsigned short importedNamesTableOffset;
    unsigned int   nonResidentNameTableOffset;
    unsigned short movableEntryPoints;
    unsigned short alignmentShiftCount;     // Shift count for resource alignment
    unsigned short resourceSegments;
    unsigned char  targetOS;
    unsigned char  otherFlags;
    unsigned short fastLoadAreaOffset;
    unsigned short fastLoadAreaLength;
    unsigned short reserved;
    unsigned short windowsVersion;

    unsigned short Signature() const { return signature; }
    unsigned short ResourceTableOffset() const { return resourceTableOffset; }
    unsigned short AlignmentShiftCount() const { return alignmentShiftCount; }
};

struct NeResourceTypeInfo {
    unsigned short typeId;              // Type ID (0x8008 = RT_FONT)
    unsigned short count;               // Number of resources of this type
    unsigned int   reserved;

    unsigned short TypeId() const { return typeId; }
    unsigned short Count() const { return count; }
};

struct NeResourceNameInfo {
    unsigned short offset;              // Offset to resource data (shifted)
    unsigned short length;              // Length of resource data (shifted)
    unsigned short flags;
    unsigned short id;                  // Resource ID
    unsigned int   reserved;

    unsigned short Offset() const { return offset; }
    unsigned short Length() const { return length; }
    unsigned short Id() const { return id; }
};

/******************************************************************************/
/*    FNT Font Header Structure (Windows 2.0/3.0 bitmap font format)          */
/******************************************************************************/

struct FntHeader {
    unsigned short dfVersion;           // 0x0200 or 0x0300
    unsigned int   dfSize;              // Total file size
    char           dfCopyright[60];     // Copyright string
    unsigned short dfType;              // 0 = raster, 1 = vector
    unsigned short dfPoints;            // Nominal point size
    unsigned short dfVertRes;           // Vertical resolution (dpi)
    unsigned short dfHorizRes;          // Horizontal resolution (dpi)
    unsigned short dfAscent;            // Ascent (baseline to top)
    unsigned short dfInternalLeading;   // Internal leading
    unsigned short dfExternalLeading;   // External leading
    unsigned char  dfItalic;            // 1 if italic
    unsigned char  dfUnderline;         // 1 if underline
    unsigned char  dfStrikeOut;         // 1 if strikeout
    unsigned short dfWeight;            // Weight (400=regular, 700=bold)
    unsigned char  dfCharSet;           // Character set (0=ANSI, 255=OEM)
    unsigned short dfPixWidth;          // Pixel width (0 = variable)
    unsigned short dfPixHeight;         // Pixel height
    unsigned char  dfPitchAndFamily;    // Pitch and family
    unsigned short dfAvgWidth;          // Average character width
    unsigned short dfMaxWidth;          // Maximum character width
    unsigned char  dfFirstChar;         // First character code
    unsigned char  dfLastChar;          // Last character code
    unsigned char  dfDefaultChar;       // Default character (relative to dfFirstChar)
    unsigned char  dfBreakChar;         // Break character (relative to dfFirstChar)
    unsigned short dfWidthBytes;        // Bytes per row (stride)
    unsigned int   dfDevice;            // Offset to device name
    unsigned int   dfFace;              // Offset to face name
    unsigned int   dfBitsPointer;       // Absolute pointer to bitmap data
    unsigned int   dfBitsOffset;        // Offset to bitmap data
    unsigned char  dfReserved;          // Reserved (V2.0)
    // V3.0 extensions follow...

    unsigned short Version() const { return dfVersion; }
    unsigned short Points() const { return dfPoints; }
    unsigned short PixHeight() const { return dfPixHeight; }
    unsigned short PixWidth() const { return dfPixWidth; }
    unsigned short Ascent() const { return dfAscent; }
    unsigned char  FirstChar() const { return dfFirstChar; }
    unsigned char  LastChar() const { return dfLastChar; }
    unsigned short AvgWidth() const { return dfAvgWidth; }
    unsigned short MaxWidth() const { return dfMaxWidth; }
    unsigned short Weight() const { return dfWeight; }
    unsigned char  Italic() const { return dfItalic; }
};

// Character width table entry
// V2.0: 4 bytes per entry (width: WORD, offset: WORD)
// V3.0: 6 bytes per entry (width: WORD, offset: DWORD)
struct FntCharEntryV2 {
    unsigned short width;               // Character width in pixels
    unsigned short offset;              // Offset to glyph bitmap (from start of FNT)
};

struct FntCharEntryV3 {
    unsigned short width;               // Character width in pixels
    unsigned int offset;                // Offset to glyph bitmap (from start of FNT)
};

#pragma pack(pop)

/******************************************************************************/
/*    Helper: Decode icon DIB data to 32-bit ARGB image                        */
/******************************************************************************/

static void DecodeIconDIB(const unsigned char* iconData, Int32 targetSize, Image& result)
{
    const BitmapInfoHeader* bmpHeader = reinterpret_cast<const BitmapInfoHeader*>(iconData);
    Int32 width = Int32(static_cast<int>(bmpHeader->Width()));
    Int32 height = Int32(static_cast<int>(bmpHeader->Height()) / 2);  // DIB height includes mask
    Int32 bitCount = Int32(static_cast<int>(bmpHeader->BitCount()));

    if (static_cast<int>(width) != static_cast<int>(targetSize) || static_cast<int>(height) != static_cast<int>(targetSize))
    {
        throw InvalidDataException("Icon DIB dimensions don't match expected size.");
    }

    // Get palette (if any)
    UInt32 paletteCount = UInt32(static_cast<unsigned int>(bmpHeader->UsedColors()));
    if (static_cast<unsigned int>(paletteCount) == 0 && static_cast<int>(bitCount) <= 8)
    {
        paletteCount = UInt32(1u << static_cast<int>(bitCount));
    }

    const unsigned char* paletteData = iconData + static_cast<unsigned int>(bmpHeader->HeaderSize());
    const unsigned char* xorMask = paletteData + static_cast<unsigned int>(paletteCount) * 4;

    // Calculate strides
    Int32 xorStride = Int32(((static_cast<int>(bitCount) * static_cast<int>(width) + 31) / 32) * 4);
    Int32 andStride = Int32(((static_cast<int>(width) + 31) / 32) * 4);
    const unsigned char* andMask = xorMask + static_cast<int>(xorStride) * static_cast<int>(height);

    unsigned int* outPixels = result.Data();

    // Decode based on bit depth
    if (static_cast<int>(bitCount) == 32)
    {
        // 32bpp BGRA with alpha
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 4 + 0];
                unsigned char g = row[static_cast<int>(x) * 4 + 1];
                unsigned char r = row[static_cast<int>(x) * 4 + 2];
                unsigned char a = row[static_cast<int>(x) * 4 + 3];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    } else if (static_cast<int>(bitCount) == 24)
    {
        // 24bpp RGB, use AND mask for transparency
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char b = row[static_cast<int>(x) * 3 + 0];
                    unsigned char g = row[static_cast<int>(x) * 3 + 1];
                    unsigned char r = row[static_cast<int>(x) * 3 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 8)
    {
        // 8bpp indexed
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char index = row[static_cast<int>(x)];
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 4)
    {
        // 4bpp indexed
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char byteVal = row[static_cast<int>(x) / 2];
                    unsigned char index = ((static_cast<int>(x) & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 1)
    {
        // 1bpp monochrome
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    Boolean pixel = Boolean(((row[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                    unsigned char index = static_cast<bool>(pixel) ? 1 : 0;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else
    {
        throw InvalidDataException("Unsupported icon bit depth.");
    }
}

/******************************************************************************/
/*    Image::FromIcon - Load icon from standalone .ico file                    */
/******************************************************************************/

Image Image::FromIcon(const char* path, const Size& size)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = Int32(static_cast<int>(size.width));
    if (static_cast<int>(targetSize) != 16 && static_cast<int>(targetSize) != 24 && static_cast<int>(targetSize) != 32 && static_cast<int>(targetSize) != 48)
    {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < static_cast<int>(sizeof(IconDirectory)))
    {
        throw InvalidDataException("File is too small to be a valid ICO.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    const IconDirectory* dir = reinterpret_cast<const IconDirectory*>(fileData);
    if (dir->Type() != 1 || dir->Count() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("Invalid ICO file format.");
    }

    const IconDirectoryEntry* entries = reinterpret_cast<const IconDirectoryEntry*>(
        fileData + sizeof(IconDirectory));

    // Find matching size
    const IconDirectoryEntry* chosen = nullptr;
    for (UInt16 i = UInt16(0); static_cast<unsigned short>(i) < dir->Count(); i += 1)
    {
        if (entries[static_cast<unsigned short>(i)].Width() == static_cast<int>(targetSize) && entries[static_cast<unsigned short>(i)].Height() == static_cast<int>(targetSize))
        {
            chosen = &entries[static_cast<unsigned short>(i)];
            break;
        }
    }

    if (!chosen)
    {
        std::free(fileData);
        throw InvalidDataException("Requested icon size not found in file.");
    }

    Image result(targetSize, targetSize);
    const unsigned char* iconData = fileData + chosen->Offset();
    DecodeIconDIB(iconData, targetSize, result);

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    Image::FromIconLibrary - Load icon from PE-based icon library            */
/******************************************************************************/

Image Image::FromIconLibrary(const char* path, Int32 iconIndex, const Size& size)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_ICON = UInt32(3);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = Int32(static_cast<int>(size.width));
    if (static_cast<int>(targetSize) != 16 && static_cast<int>(targetSize) != 24 && static_cast<int>(targetSize) != 32 && static_cast<int>(targetSize) != 48)
    {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("No resource section in file.");
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        throw InvalidDataException("Resource section not found.");
    }

    UInt32 rsrcRva = UInt32(rsrcSection->VirtualAddress());
    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - static_cast<unsigned int>(rsrcRva));

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDirectoryEntry* groupIconEntry = nullptr;
    const PortableExecutableResourceDirectoryEntry* iconEntry = nullptr;

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed())
        {
            if (rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
            {
                groupIconEntry = &rootEntries[static_cast<int>(i)];
            } else if (rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_ICON))
            {
                iconEntry = &rootEntries[static_cast<int>(i)];
            }
        }
    }

    if (!groupIconEntry || !iconEntry)
    {
        std::free(fileData);
        throw InvalidDataException("No icon resources found.");
    }

    // Navigate to the RT_GROUP_ICON directory
    const PortableExecutableResourceDirectory* groupIconDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + groupIconEntry->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* groupIconEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(groupIconDir) + sizeof(PortableExecutableResourceDirectory));

    Int32 idx = iconIndex;
    if (static_cast<int>(idx) < 0 || static_cast<int>(idx) >= groupIconDir->TotalEntries())
    {
        std::free(fileData);
        throw ArgumentException("Icon index out of range.");
    }

    // Get the specific icon group
    const PortableExecutableResourceDirectoryEntry* chosenGroup = &groupIconEntries[static_cast<int>(idx)];
    if (!chosenGroup->IsDirectory())
    {
        std::free(fileData);
        throw InvalidDataException("Invalid icon group entry.");
    }

    // Navigate to language level
    const PortableExecutableResourceDirectory* langDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + chosenGroup->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* langEntry =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(langDir) + sizeof(PortableExecutableResourceDirectory));

    if (langEntry->IsDirectory())
    {
        std::free(fileData);
        throw InvalidDataException("Invalid icon resource structure.");
    }

    // Get the data entry
    const PortableExecutableResourceDataEntry* dataEntry =
        reinterpret_cast<const PortableExecutableResourceDataEntry*>(
            rsrcBase + langEntry->GetOffsetToData());

    const unsigned char* groupData = fileData + static_cast<unsigned int>(rsrcOffset) + (dataEntry->DataRva() - static_cast<unsigned int>(rsrcRva));

    // Parse the GROUP_ICON directory
    const IconDirectory* iconDir = reinterpret_cast<const IconDirectory*>(groupData);
    if (iconDir->Type() != 1 || iconDir->Count() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("Invalid GROUP_ICON format.");
    }

    const GroupIconDirectoryEntry* groupEntries =
        reinterpret_cast<const GroupIconDirectoryEntry*>(groupData + sizeof(IconDirectory));

    // Find matching size
    const GroupIconDirectoryEntry* chosenIcon = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < iconDir->Count(); i += 1)
    {
        if (groupEntries[static_cast<int>(i)].Width() == static_cast<int>(targetSize) && groupEntries[static_cast<int>(i)].Height() == static_cast<int>(targetSize))
        {
            chosenIcon = &groupEntries[static_cast<int>(i)];
            break;
        }
    }

    if (!chosenIcon)
    {
        std::free(fileData);
        throw InvalidDataException("Requested icon size not found.");
    }

    // Now find the RT_ICON with matching identifier
    const PortableExecutableResourceDirectory* iconTypeDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + iconEntry->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* iconTypeEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(iconTypeDir) + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDirectoryEntry* matchingIcon = nullptr;
    Int32 totalIconEntries = Int32(iconTypeDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalIconEntries); i += 1)
    {
        if (!iconTypeEntries[static_cast<int>(i)].IsNamed() && iconTypeEntries[static_cast<int>(i)].GetId() == chosenIcon->Identifier())
        {
            matchingIcon = &iconTypeEntries[static_cast<int>(i)];
            break;
        }
    }

    if (!matchingIcon)
    {
        std::free(fileData);
        throw InvalidDataException("Icon resource not found.");
    }

    // Navigate to language level for the icon
    const PortableExecutableResourceDirectory* iconLangDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + matchingIcon->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* iconLangEntry =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(iconLangDir) + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDataEntry* iconDataEntry =
        reinterpret_cast<const PortableExecutableResourceDataEntry*>(
            rsrcBase + iconLangEntry->GetOffsetToData());

    const unsigned char* iconData = fileData + static_cast<unsigned int>(rsrcOffset) + (iconDataEntry->DataRva() - static_cast<unsigned int>(rsrcRva));

    Image result(targetSize, targetSize);
    DecodeIconDIB(iconData, targetSize, result);

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    Image::GetIconLibraryCount - Get number of icon groups in library        */
/******************************************************************************/

Int32 Image::GetIconLibraryCount(const char* path)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        return Int32(0);  // No resources
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        return Int32(0);
    }

    UInt32 rsrcRva = UInt32(rsrcSection->VirtualAddress());
    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - static_cast<unsigned int>(rsrcRva));

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed() && rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
        {
            // Found RT_GROUP_ICON, count entries
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[static_cast<int>(i)].GetOffsetToData());
            Int32 count = Int32(groupIconDir->TotalEntries());
            std::free(fileData);
            return count;
        }
    }

    std::free(fileData);
    return Int32(0);
}

/******************************************************************************/
/*    Image::GetIconLibraryNames - Get names of icons in PE library            */
/******************************************************************************/

// Helper: Read PE resource name (UTF-16LE length-prefixed string)
// Returns empty string for unnamed resources or on error
static String ReadResourceName(const unsigned char* rsrcBase, UInt32 nameOffset)
{
    // Name format: WORD length (characters), followed by UTF-16LE chars (no null)
    const unsigned char* namePtr = rsrcBase + static_cast<unsigned int>(nameOffset);
    UInt16 charCount = UInt16(*reinterpret_cast<const unsigned short*>(namePtr));

    if (static_cast<unsigned short>(charCount) == 0 || static_cast<unsigned short>(charCount) > 256)
    {
        return String();  // Empty or suspiciously long
    }

    // Convert UTF-16LE to ASCII (simple conversion, ignore high bytes)
    char* asciiName = static_cast<char*>(std::malloc(static_cast<unsigned short>(charCount) + 1));
    if (!asciiName) return String();

    const unsigned short* utf16Chars = reinterpret_cast<const unsigned short*>(namePtr + 2);
    for (UInt16 i = UInt16(0); static_cast<unsigned short>(i) < static_cast<unsigned short>(charCount); i += 1)
    {
        // Simple conversion - just take low byte (works for ASCII names)
        asciiName[static_cast<unsigned short>(i)] = static_cast<char>(utf16Chars[static_cast<unsigned short>(i)] & 0xFF);
    }
    asciiName[static_cast<unsigned short>(charCount)] = '\0';

    String result(asciiName);
    std::free(asciiName);
    return result;
}

Array<String> Image::GetIconLibraryNames(const char* path)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        return Array<String>(0);
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData) return Array<String>(0);

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS/PE headers
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        return Array<String>(0);
    }

    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        return Array<String>(0);
    }

    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - rsrcSection->VirtualAddress());

    // Find RT_GROUP_ICON directory
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed() && rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
        {
            // Found RT_GROUP_ICON, get icon names
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[static_cast<int>(i)].GetOffsetToData());
            const PortableExecutableResourceDirectoryEntry* iconEntries =
                reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
                    reinterpret_cast<const unsigned char*>(groupIconDir) + sizeof(PortableExecutableResourceDirectory));

            Int32 count = Int32(groupIconDir->TotalEntries());
            Array<String> names(count);

            for (Int32 j = Int32(0); static_cast<int>(j) < static_cast<int>(count); j += 1)
            {
                if (iconEntries[static_cast<int>(j)].IsNamed())
                {
                    names[static_cast<int>(j)] = ReadResourceName(rsrcBase, UInt32(iconEntries[static_cast<int>(j)].GetNameOffset()));
                }
                else
                {
                    // ID-based entry - return empty string or the ID as string
                    names[static_cast<int>(j)] = String();
                }
            }

            std::free(fileData);
            return names;
        }
    }

    std::free(fileData);
    return Array<String>(0);
}

/******************************************************************************/
/*    Image::GetIconLibraryIndex - Find icon index by name                     */
/******************************************************************************/

Int32 Image::GetIconLibraryIndex(const char* path, const char* iconName)
{
    if (!path || !iconName || iconName[0] == '\0')
    {
        return Int32(-1);
    }

    Array<String> names = GetIconLibraryNames(path);
    String target(iconName);

    for (Int32 i = Int32(0); static_cast<int>(i) < names.Length(); i += 1)
    {
        if (names[static_cast<int>(i)].EqualsIgnoreCase(target))
        {
            return i;
        }
    }

    return Int32(-1);
}

/******************************************************************************/
/*    Image::FromIconLibrary (by name) - Load icon by name                     */
/******************************************************************************/

Image Image::FromIconLibrary(const char* path, const char* iconName, const Size& size)
{
    if (!iconName || iconName[0] == '\0')
    {
        throw ArgumentNullException("iconName");
    }

    Int32 index = GetIconLibraryIndex(path, iconName);
    if (static_cast<int>(index) < 0)
    {
        throw ArgumentException("Icon not found in library.");
    }

    return FromIconLibrary(path, index, size);
}

/******************************************************************************/
/*    Image::FromFile - Load image from file (auto-detect format)              */
/******************************************************************************/

Image Image::FromFile(const char* path)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    if (!IO::File::Exists(path))
    {
        throw System::FileNotFoundException(path);
    }

    // Check file extension to decide loader
    const char* ext = path;
    const char* p = path;
    while (*p)
    {
        if (*p == '.') ext = p;
        p++;
    }

    // Use native BMP loader for .bmp files
    if ((ext[1] == 'b' || ext[1] == 'B') &&
        (ext[2] == 'm' || ext[2] == 'M') &&
        (ext[3] == 'p' || ext[3] == 'P') &&
        ext[4] == '\0')
        {
        return FromBitmap(path);
    }

    // Use stb_image for PNG, JPEG, GIF, TGA, PSD, etc.
    Array<UInt8> fileData = IO::File::ReadAllBytes(path);
    int width, height, channels;

    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(&fileData[0]),
        fileData.Length(),
        &width, &height, &channels,
        4  // Request RGBA output
    );

    if (!pixels)
    {
        throw InvalidDataException("Failed to decode image file.");
    }

    // Create image and copy pixels
    Image img = Image(Int32(width), Int32(height));

    // stb_image returns RGBA, we need ARGB
    unsigned int* dest = img.Data();
    for (Int32 i = Int32(0); static_cast<int>(i) < width * height; i += 1)
    {
        unsigned char r = pixels[static_cast<int>(i) * 4 + 0];
        unsigned char g = pixels[static_cast<int>(i) * 4 + 1];
        unsigned char b = pixels[static_cast<int>(i) * 4 + 2];
        unsigned char a = pixels[static_cast<int>(i) * 4 + 3];
        dest[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) |
                  (static_cast<unsigned int>(r) << 16) |
                  (static_cast<unsigned int>(g) << 8) |
                  static_cast<unsigned int>(b);
    }

    stbi_image_free(pixels);
    return img;
}

/******************************************************************************/
/*    Image::FromPng - Load PNG image from file                                */
/******************************************************************************/

Image Image::FromPng(const char* path)
{
    // FromFile handles PNG via stb_image
    return FromFile(path);
}

/******************************************************************************/
/*    Image::FromJpeg - Load JPEG image from file                              */
/******************************************************************************/

Image Image::FromJpeg(const char* path)
{
    // FromFile handles JPEG via stb_image
    return FromFile(path);
}

/******************************************************************************/
/*    Image::ScaleTo - Scale image using bilinear interpolation                */
/******************************************************************************/

Image Image::ScaleTo(Int32 newWidth, Int32 newHeight) const {
    Int32 nw = newWidth;
    Int32 nh = newHeight;

    if (static_cast<int>(nw) <= 0 || static_cast<int>(nh) <= 0)
    {
        throw ArgumentException("New dimensions must be positive");
    }

    if (_width == 0 || _height == 0 || _data == nullptr)
    {
        return Image(newWidth, newHeight, Color::Black);
    }

    Image result = Image(newWidth, newHeight);
    unsigned int* dest = result.Data();
    const unsigned int* src = _data;

    // Fixed-point scaling factors (16.16 format)
    Int32 scaleX = Int32((_width << 16) / static_cast<int>(nw));
    Int32 scaleY = Int32((_height << 16) / static_cast<int>(nh));

    for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(nh); y += 1)
    {
        Int32 srcY = Int32((static_cast<int>(y) * static_cast<int>(scaleY)) >> 16);
        Int32 fracY = Int32((static_cast<int>(y) * static_cast<int>(scaleY)) & 0xFFFF);

        // Clamp source Y
        if (static_cast<int>(srcY) >= _height - 1)
        {
            srcY = Int32(_height - 2);
            fracY = Int32(0xFFFF);
        }
        if (static_cast<int>(srcY) < 0) srcY = Int32(0);

        for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(nw); x += 1)
        {
            Int32 srcX = Int32((static_cast<int>(x) * static_cast<int>(scaleX)) >> 16);
            Int32 fracX = Int32((static_cast<int>(x) * static_cast<int>(scaleX)) & 0xFFFF);

            // Clamp source X
            if (static_cast<int>(srcX) >= _width - 1)
            {
                srcX = Int32(_width - 2);
                fracX = Int32(0xFFFF);
            }
            if (static_cast<int>(srcX) < 0) srcX = Int32(0);

            // Get 4 neighboring pixels
            UInt32 p00 = UInt32(src[static_cast<int>(srcY) * _width + static_cast<int>(srcX)]);
            UInt32 p10 = UInt32(src[static_cast<int>(srcY) * _width + static_cast<int>(srcX) + 1]);
            UInt32 p01 = UInt32(src[(static_cast<int>(srcY) + 1) * _width + static_cast<int>(srcX)]);
            UInt32 p11 = UInt32(src[(static_cast<int>(srcY) + 1) * _width + static_cast<int>(srcX) + 1]);

            // Bilinear interpolation for each channel
            Int32 fx = Int32(static_cast<int>(fracX) >> 8);  // 0-255
            Int32 fy = Int32(static_cast<int>(fracY) >> 8);  // 0-255
            Int32 fx1 = Int32(256 - static_cast<int>(fx));
            Int32 fy1 = Int32(256 - static_cast<int>(fy));

            // Alpha channel
            Int32 a00 = Int32((static_cast<unsigned int>(p00) >> 24) & 0xFF);
            Int32 a10 = Int32((static_cast<unsigned int>(p10) >> 24) & 0xFF);
            Int32 a01 = Int32((static_cast<unsigned int>(p01) >> 24) & 0xFF);
            Int32 a11 = Int32((static_cast<unsigned int>(p11) >> 24) & 0xFF);
            Int32 a = Int32(((static_cast<int>(a00) * static_cast<int>(fx1) + static_cast<int>(a10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(a01) * static_cast<int>(fx1) + static_cast<int>(a11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Red channel
            Int32 r00 = Int32((static_cast<unsigned int>(p00) >> 16) & 0xFF);
            Int32 r10 = Int32((static_cast<unsigned int>(p10) >> 16) & 0xFF);
            Int32 r01 = Int32((static_cast<unsigned int>(p01) >> 16) & 0xFF);
            Int32 r11 = Int32((static_cast<unsigned int>(p11) >> 16) & 0xFF);
            Int32 r = Int32(((static_cast<int>(r00) * static_cast<int>(fx1) + static_cast<int>(r10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(r01) * static_cast<int>(fx1) + static_cast<int>(r11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Green channel
            Int32 g00 = Int32((static_cast<unsigned int>(p00) >> 8) & 0xFF);
            Int32 g10 = Int32((static_cast<unsigned int>(p10) >> 8) & 0xFF);
            Int32 g01 = Int32((static_cast<unsigned int>(p01) >> 8) & 0xFF);
            Int32 g11 = Int32((static_cast<unsigned int>(p11) >> 8) & 0xFF);
            Int32 g = Int32(((static_cast<int>(g00) * static_cast<int>(fx1) + static_cast<int>(g10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(g01) * static_cast<int>(fx1) + static_cast<int>(g11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Blue channel
            Int32 b00 = Int32(static_cast<unsigned int>(p00) & 0xFF);
            Int32 b10 = Int32(static_cast<unsigned int>(p10) & 0xFF);
            Int32 b01 = Int32(static_cast<unsigned int>(p01) & 0xFF);
            Int32 b11 = Int32(static_cast<unsigned int>(p11) & 0xFF);
            Int32 b = Int32(((static_cast<int>(b00) * static_cast<int>(fx1) + static_cast<int>(b10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(b01) * static_cast<int>(fx1) + static_cast<int>(b11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            dest[static_cast<int>(y) * static_cast<int>(nw) + static_cast<int>(x)] = (static_cast<unsigned int>(a) << 24) |
                               (static_cast<unsigned int>(r) << 16) |
                               (static_cast<unsigned int>(g) << 8) |
                               static_cast<unsigned int>(b);
        }
    }

    return result;
}

Image Image::ScaleTo(const Size& newSize) const {
    return ScaleTo(newSize.width, newSize.height);
}

/******************************************************************************/
/*    Font::FontData - Internal font data storage                              */
/******************************************************************************/

struct Font::FontData {
    String name;                    // Font face name
    Int32 pointSize;                // Nominal point size
    Int32 pixelHeight;              // Actual pixel height
    Int32 ascent;                   // Pixels above baseline
    FontStyle style;                // Font style flags
    Int32 firstChar;                // First character code
    Int32 lastChar;                 // Last character code
    Boolean isTrueType;             // True if TTF, false if FON

    // Character widths (256 entries, 0 for non-existent chars)
    unsigned short charWidths[256];

    // FON: Glyph offsets into bitmap data
    unsigned int charOffsets[256];

    // Raw font file data (FON bitmap or TTF file)
    unsigned char* bitmapData;
    unsigned int bitmapSize;

    // TTF: stbtt font info
    stbtt_fontinfo ttfInfo;
    Float32 ttfScale;               // Scale factor for pixel height

    // Glyph cache (lazily populated)
    mutable Image glyphCache[256];
    mutable Boolean glyphCached[256];

    FontData()
        : pointSize(Int32(0)), pixelHeight(Int32(0)), ascent(Int32(0)), style(FontStyle::Regular)
        , firstChar(Int32(0)), lastChar(Int32(0)), isTrueType(Boolean(false))
        , bitmapData(nullptr), bitmapSize(0), ttfScale(Float32(0.0f))
        {
        std::memset(charWidths, 0, sizeof(charWidths));
        std::memset(charOffsets, 0, sizeof(charOffsets));
        // Initialize Boolean array element by element (can't use memset on non-trivial types)
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
        {
            glyphCached[static_cast<int>(i)] = Boolean(false);
        }
        std::memset(&ttfInfo, 0, sizeof(ttfInfo));
    }

    ~FontData()
    {
        if (bitmapData)
        {
            std::free(bitmapData);
            bitmapData = nullptr;
        }
    }

    // Render a glyph to the cache
    void RenderGlyph(Int32 ch) const {
        if (static_cast<int>(ch) < 0 || static_cast<int>(ch) > 255 || static_cast<bool>(glyphCached[static_cast<int>(ch)])) return;

        if (static_cast<bool>(isTrueType))
        {
            RenderTrueTypeGlyph(ch);
        }
        else
        {
            RenderFonGlyph(ch);
        }
    }

    // Render FON (bitmap) glyph
    void RenderFonGlyph(Int32 ch) const {
        if (static_cast<int>(ch) < static_cast<int>(firstChar) || static_cast<int>(ch) > static_cast<int>(lastChar))
        {
            // Character not in font - create empty glyph
            glyphCache[static_cast<int>(ch)] = Image(Int32(1), pixelHeight, Color::Transparent);
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        Int32 width = Int32(charWidths[static_cast<int>(ch)]);
        Int32 height = pixelHeight;
        if (static_cast<int>(width) <= 0)
        {
            glyphCache[static_cast<int>(ch)] = Image(Int32(1), height, Color::Transparent);
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        // Create transparent glyph image
        glyphCache[static_cast<int>(ch)] = Image(width, height, Color::Transparent);

        // FON bitmap format: column-major by byte-columns (per FreeType winfnt.c)
        // Each byte-column (8 horizontal pixels) is stored as 'height' sequential bytes
        // Source layout: src[byteCol * height + row]
        // Bits: MSB is leftmost pixel within each byte
        const unsigned char* src = bitmapData + charOffsets[static_cast<int>(ch)];

        for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(height); row += 1)
        {
            for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(width); col += 1)
            {
                Int32 byteCol = Int32(static_cast<int>(col) / 8);
                Int32 bitIndex = Int32(7 - (static_cast<int>(col) % 8));  // MSB is leftmost pixel
                // Column-major access: byteCol * height + row
                unsigned char byte = src[static_cast<int>(byteCol) * static_cast<int>(height) + static_cast<int>(row)];
                Boolean pixel = Boolean(((byte >> static_cast<int>(bitIndex)) & 1) != 0);
                if (static_cast<bool>(pixel))
                {
                    glyphCache[static_cast<int>(ch)].SetPixel(col, row, Color::White);
                }
            }
        }

        glyphCached[static_cast<int>(ch)] = Boolean(true);
    }

    // Render TrueType glyph using stb_truetype
    void RenderTrueTypeGlyph(Int32 ch) const {
        // Get horizontal metrics (advance and left side bearing)
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&ttfInfo, static_cast<int>(ch), &advanceWidth, &lsb);

        // Get bitmap bounding box
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&ttfInfo, static_cast<int>(ch), static_cast<float>(ttfScale), static_cast<float>(ttfScale), &x0, &y0, &x1, &y1);

        Int32 glyphWidth = Int32(x1 - x0);
        Int32 glyphHeight = Int32(y1 - y0);

        // Scale lsb to pixels
        Int32 lsbPixels = Int32(static_cast<int>(lsb * static_cast<float>(ttfScale) + 0.5f));

        // Use advance width for image width (for proper character spacing)
        Int32 imageWidth = Int32(charWidths[static_cast<int>(ch)]);
        if (static_cast<int>(imageWidth) <= 0) imageWidth = Int32(1);
        Int32 imageHeight = pixelHeight;

        // Create glyph image
        glyphCache[static_cast<int>(ch)] = Image(imageWidth, imageHeight, Color::Transparent);

        if (static_cast<int>(glyphWidth) <= 0 || static_cast<int>(glyphHeight) <= 0)
        {
            // Empty glyph (e.g., space) - just return the transparent image
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        // Rasterize the glyph (8-bit grayscale)
        unsigned char* bitmap = static_cast<unsigned char*>(
            std::malloc(static_cast<int>(glyphWidth) * static_cast<int>(glyphHeight)));
        if (!bitmap)
        {
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        stbtt_MakeCodepointBitmap(&ttfInfo, bitmap, static_cast<int>(glyphWidth), static_cast<int>(glyphHeight),
                                   static_cast<int>(glyphWidth), static_cast<float>(ttfScale), static_cast<float>(ttfScale), static_cast<int>(ch));

        // Position glyph in image using the working example's approach:
        // - Horizontally: use lsb (left side bearing) scaled to pixels
        // - Vertically: use ascent + y0 for baseline alignment

        // Copy bitmap to image with anti-aliasing (store grayscale as alpha)
        for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(glyphHeight); row += 1)
        {
            Int32 destY = Int32(static_cast<int>(ascent) + y0 + static_cast<int>(row));
            if (static_cast<int>(destY) < 0 || static_cast<int>(destY) >= static_cast<int>(imageHeight)) continue;

            for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(glyphWidth); col += 1)
            {
                Int32 destX = Int32(static_cast<int>(lsbPixels) + static_cast<int>(col));
                if (static_cast<int>(destX) < 0 || static_cast<int>(destX) >= static_cast<int>(imageWidth)) continue;

                unsigned char gray = bitmap[static_cast<int>(row) * static_cast<int>(glyphWidth) + static_cast<int>(col)];
                if (gray > 0)
                {
                    // Store grayscale as alpha for anti-aliasing
                    // White color with variable alpha
                    glyphCache[static_cast<int>(ch)].SetPixel(destX, destY, Color(255, 255, 255, gray));
                }
            }
        }

        std::free(bitmap);
        glyphCached[static_cast<int>(ch)] = Boolean(true);
    }
};

/******************************************************************************/
/*    Font implementation                                                      */
/******************************************************************************/

Font::Font() : _data(nullptr) {}

Font::Font(FontData* data) : _data(data) {}

Font::Font(const Font& other) : _data(nullptr)
{
    if (other._data)
    {
        _data = new FontData();
        _data->name = other._data->name;
        _data->pointSize = other._data->pointSize;
        _data->pixelHeight = other._data->pixelHeight;
        _data->ascent = other._data->ascent;
        _data->style = other._data->style;
        _data->firstChar = other._data->firstChar;
        _data->lastChar = other._data->lastChar;
        _data->isTrueType = other._data->isTrueType;
        _data->ttfScale = other._data->ttfScale;
        std::memcpy(_data->charWidths, other._data->charWidths, sizeof(_data->charWidths));
        std::memcpy(_data->charOffsets, other._data->charOffsets, sizeof(_data->charOffsets));

        if (other._data->bitmapData && other._data->bitmapSize > 0)
        {
            _data->bitmapData = static_cast<unsigned char*>(std::malloc(other._data->bitmapSize));
            if (_data->bitmapData)
            {
                std::memcpy(_data->bitmapData, other._data->bitmapData, other._data->bitmapSize);
                _data->bitmapSize = other._data->bitmapSize;

                // For TTF fonts, re-initialize ttfInfo to point to the new bitmapData
                if (static_cast<bool>(_data->isTrueType))
                {
                    int fontOffset = stbtt_GetFontOffsetForIndex(_data->bitmapData, 0);
                    stbtt_InitFont(&_data->ttfInfo, _data->bitmapData, fontOffset);
                }
            }
        }

        // Copy cached glyphs
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
        {
            if (static_cast<bool>(other._data->glyphCached[static_cast<int>(i)]))
            {
                _data->glyphCache[static_cast<int>(i)] = other._data->glyphCache[static_cast<int>(i)];
                _data->glyphCached[static_cast<int>(i)] = Boolean(true);
            }
        }
    }
}

Font::Font(Font&& other) noexcept : _data(other._data)
{
    other._data = nullptr;
}

Font::~Font()
{
    if (_data)
    {
        delete _data;
        _data = nullptr;
    }
}

Font& Font::operator=(const Font& other)
{
    if (this != &other)
    {
        if (_data)
        {
            delete _data;
            _data = nullptr;
        }
        if (other._data)
        {
            Font temp(other);
            _data = temp._data;
            temp._data = nullptr;
        }
    }
    return *this;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other)
    {
        if (_data)
        {
            delete _data;
        }
        _data = other._data;
        other._data = nullptr;
    }
    return *this;
}

Font Font::FromFile(const char* path, Int32 size, FontStyle style)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt16 NE_SIGNATURE = UInt16(0x454E);
    const UInt16 RT_FONT = UInt16(0x8008);  // NE resource type for fonts

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = size;

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < 64)
    {
        throw InvalidDataException("File is too small to be a valid FON file.");
    }

    // Copy to raw buffer
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse MZ header to find NE header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    UInt32 neOffset = UInt32(dosHeader->NewHeaderOffset());
    if (static_cast<unsigned int>(neOffset) >= static_cast<unsigned int>(fileSize) - sizeof(NeHeader))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid NE header offset.");
    }

    const NeHeader* neHeader = reinterpret_cast<const NeHeader*>(fileData + static_cast<unsigned int>(neOffset));
    if (neHeader->Signature() != static_cast<unsigned short>(NE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid NE signature (not a FON file).");
    }

    // Parse resource table
    UInt32 rsrcTableOffset = UInt32(static_cast<unsigned int>(neOffset) + neHeader->ResourceTableOffset());
    if (static_cast<unsigned int>(rsrcTableOffset) >= static_cast<unsigned int>(fileSize))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid resource table offset.");
    }

    // Resource table starts with alignment shift count
    const unsigned char* rsrcTable = fileData + static_cast<unsigned int>(rsrcTableOffset);
    UInt16 alignShift = UInt16(*reinterpret_cast<const unsigned short*>(rsrcTable));
    rsrcTable += 2;

    // Find RT_FONT resources
    const FntHeader* bestFont = nullptr;
    Int32 bestMatch = Int32(0x7FFFFFFF);
    Boolean isBold = Boolean((static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Bold)) != 0);
    Boolean isItalic = Boolean((static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Italic)) != 0);

    while (true)
    {
        const NeResourceTypeInfo* typeInfo = reinterpret_cast<const NeResourceTypeInfo*>(rsrcTable);
        if (typeInfo->TypeId() == 0) break;  // End of resource table

        rsrcTable += sizeof(NeResourceTypeInfo);

        if (typeInfo->TypeId() == static_cast<unsigned short>(RT_FONT))
        {
            // Found font resources
            for (Int32 i = Int32(0); static_cast<int>(i) < typeInfo->Count(); i += 1)
            {
                const NeResourceNameInfo* nameInfo = reinterpret_cast<const NeResourceNameInfo*>(rsrcTable);
                rsrcTable += sizeof(NeResourceNameInfo);

                // Calculate actual offset
                UInt32 fontOffset = UInt32(static_cast<unsigned int>(nameInfo->Offset()) << static_cast<unsigned short>(alignShift));
                if (static_cast<unsigned int>(fontOffset) >= static_cast<unsigned int>(fileSize)) continue;

                const FntHeader* fntHeader = reinterpret_cast<const FntHeader*>(fileData + static_cast<unsigned int>(fontOffset));

                // Check if this font matches our criteria
                Int32 fontPoints = Int32(fntHeader->Points());
                Boolean fontBold = Boolean(fntHeader->Weight() >= 700);
                Boolean fontItalic = Boolean(fntHeader->Italic() != 0);

                // Calculate match score (lower is better)
                Int32 sizeDiff = Int32(static_cast<int>(fontPoints) > static_cast<int>(targetSize) ? static_cast<int>(fontPoints) - static_cast<int>(targetSize) : static_cast<int>(targetSize) - static_cast<int>(fontPoints));
                Int32 styleMatch = Int32(0);
                if (static_cast<bool>(fontBold) != static_cast<bool>(isBold)) styleMatch = Int32(static_cast<int>(styleMatch) + 100);
                if (static_cast<bool>(fontItalic) != static_cast<bool>(isItalic)) styleMatch = Int32(static_cast<int>(styleMatch) + 100);

                Int32 matchScore = Int32(static_cast<int>(sizeDiff) + static_cast<int>(styleMatch));
                if (static_cast<int>(matchScore) < static_cast<int>(bestMatch))
                {
                    bestMatch = matchScore;
                    bestFont = fntHeader;
                }
            }
        }
        else
        {
            // Skip resources of other types
            rsrcTable += typeInfo->Count() * sizeof(NeResourceNameInfo);
        }
    }

    if (!bestFont)
    {
        std::free(fileData);
        throw InvalidDataException("No font resources found in file.");
    }

    // Parse the selected font
    FontData* data = new FontData();
    data->pointSize = Int32(bestFont->Points());
    data->pixelHeight = Int32(bestFont->PixHeight());
    data->ascent = Int32(bestFont->Ascent());
    data->firstChar = Int32(bestFont->FirstChar());
    data->lastChar = Int32(bestFont->LastChar());

    // Use requested style (allows fake bold/italic even if font doesn't have it)
    // Combine with any inherent style from the font file
    data->style = style;
    if (bestFont->Weight() >= 700)
    {
        data->style = data->style | FontStyle::Bold;
    }
    if (bestFont->Italic())
    {
        data->style = data->style | FontStyle::Italic;
    }

    // Get face name
    UInt32 faceOffset = UInt32(bestFont->dfFace);
    const unsigned char* fontBase = reinterpret_cast<const unsigned char*>(bestFont);
    if (static_cast<unsigned int>(faceOffset) > 0 && static_cast<unsigned int>(faceOffset) < 0x10000)
    {
        const char* faceName = reinterpret_cast<const char*>(fontBase + static_cast<unsigned int>(faceOffset));
        data->name = String(faceName);
    }
    else
    {
        data->name = String("Unknown");
    }

    // Calculate character widths and offsets
    Boolean isV3 = Boolean(bestFont->Version() >= 0x0300);
    Int32 numChars = Int32(static_cast<int>(data->lastChar) - static_cast<int>(data->firstChar) + 1);

    // Character table follows FntHeader (per FreeType winfnt.c line 1026)
    // V2.0: 118 bytes
    // V3.0: 148 bytes
    Int32 headerSize = static_cast<bool>(isV3) ? Int32(148) : Int32(118);
    const unsigned char* charTable = fontBase + static_cast<int>(headerSize);

    if (static_cast<bool>(isV3))
    {
        // V3.0 format: 6-byte entries (2-byte width + 4-byte offset)
        const FntCharEntryV3* entries = reinterpret_cast<const FntCharEntryV3*>(charTable);

        for (Int32 i = Int32(0); static_cast<int>(i) <= static_cast<int>(numChars); i += 1)
        {  // +1 for sentinel
            Int32 charCode = Int32(static_cast<int>(data->firstChar) + static_cast<int>(i));
            if (static_cast<int>(charCode) >= 0 && static_cast<int>(charCode) < 256 && static_cast<int>(i) < static_cast<int>(numChars))
            {
                data->charWidths[static_cast<int>(charCode)] = entries[static_cast<int>(i)].width;
                data->charOffsets[static_cast<int>(charCode)] = entries[static_cast<int>(i)].offset;
            }
        }
    }
    else
    {
        // V2.0 format: 4-byte entries (2-byte width + 2-byte offset)
        const FntCharEntryV2* entries = reinterpret_cast<const FntCharEntryV2*>(charTable);

        for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(numChars); i += 1)
        {
            Int32 charCode = Int32(static_cast<int>(data->firstChar) + static_cast<int>(i));
            if (static_cast<int>(charCode) >= 0 && static_cast<int>(charCode) < 256)
            {
                data->charWidths[static_cast<int>(charCode)] = entries[static_cast<int>(i)].width;
                data->charOffsets[static_cast<int>(charCode)] = entries[static_cast<int>(i)].offset;
            }
        }
    }

    // Copy bitmap data
    // Each glyph bitmap size is: pitch * height (where pitch = ceil(width/8))
    UInt32 bitmapStart = UInt32(0);
    UInt32 bitmapEnd = UInt32(0);
    Int32 height = data->pixelHeight;

    if (static_cast<bool>(isV3))
    {
        // In V3, offsets are from start of FNT resource
        bitmapStart = UInt32(0xFFFFFFFF);
        for (Int32 i = data->firstChar; static_cast<int>(i) <= static_cast<int>(data->lastChar); i += 1)
        {
            if (data->charOffsets[static_cast<int>(i)] > 0 && data->charOffsets[static_cast<int>(i)] < static_cast<unsigned int>(bitmapStart))
            {
                bitmapStart = UInt32(data->charOffsets[static_cast<int>(i)]);
            }
            Int32 bytesPerRow = Int32((data->charWidths[static_cast<int>(i)] + 7) / 8);
            UInt32 charEnd = UInt32(data->charOffsets[static_cast<int>(i)] + static_cast<int>(bytesPerRow) * static_cast<int>(height));
            if (static_cast<unsigned int>(charEnd) > static_cast<unsigned int>(bitmapEnd))
            {
                bitmapEnd = charEnd;
            }
        }
    }
    else
    {
        // In V2, offsets are from start of FNT resource
        bitmapStart = UInt32(0xFFFFFFFF);
        for (Int32 i = data->firstChar; static_cast<int>(i) <= static_cast<int>(data->lastChar); i += 1)
        {
            if (data->charOffsets[static_cast<int>(i)] > 0 && data->charOffsets[static_cast<int>(i)] < static_cast<unsigned int>(bitmapStart))
            {
                bitmapStart = UInt32(data->charOffsets[static_cast<int>(i)]);
            }
            Int32 bytesPerRow = Int32((data->charWidths[static_cast<int>(i)] + 7) / 8);
            UInt32 charEnd = UInt32(data->charOffsets[static_cast<int>(i)] + static_cast<int>(bytesPerRow) * static_cast<int>(height));
            if (static_cast<unsigned int>(charEnd) > static_cast<unsigned int>(bitmapEnd))
            {
                bitmapEnd = charEnd;
            }
        }
    }

    if (static_cast<unsigned int>(bitmapEnd) > static_cast<unsigned int>(bitmapStart))
    {
        data->bitmapSize = static_cast<unsigned int>(bitmapEnd);  // Store entire font data to simplify offsets
        data->bitmapData = static_cast<unsigned char*>(std::malloc(data->bitmapSize));
        if (data->bitmapData)
        {
            std::memcpy(data->bitmapData, fontBase, data->bitmapSize);
        }
    }

    std::free(fileData);
    return Font(data);
}

Font Font::SystemFont()
{
    try {
        return FromFile("MSSANS.fon", 8);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::SystemFontBold()
{
    try {
        return FromFile("MSSANS.fon", 8, FontStyle::Bold);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FixedFont()
{
    try {
        return FromFile("FIXEDSYS.fon", 8);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FromTrueType(const char* path, Int32 pixelHeight, FontStyle style)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetHeight = pixelHeight;
    if (static_cast<int>(targetHeight) <= 0)
    {
        throw ArgumentException("pixelHeight must be positive.");
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < 12)
    {
        throw InvalidDataException("File is too small to be a valid TTF file.");
    }

    // Allocate and copy file data (stb_truetype requires persistent data)
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Initialize stb_truetype
    FontData* data = new FontData();
    data->bitmapData = fileData;
    data->bitmapSize = static_cast<int>(fileSize);
    data->isTrueType = Boolean(true);
    data->style = style;

    // Get font offset (handles font collections and validates TTF header)
    Int32 fontOffset = Int32(stbtt_GetFontOffsetForIndex(fileData, 0));
    if (static_cast<int>(fontOffset) < 0)
    {
        delete data;
        throw InvalidDataException("Invalid TTF file or font index.");
    }

    if (!stbtt_InitFont(&data->ttfInfo, fileData, static_cast<int>(fontOffset)))
    {
        delete data;
        throw InvalidDataException("Failed to parse TTF file.");
    }

    // Calculate scale for desired pixel height
    data->ttfScale = Float32(stbtt_ScaleForPixelHeight(&data->ttfInfo, static_cast<float>(targetHeight)));

    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&data->ttfInfo, &ascent, &descent, &lineGap);

    data->pixelHeight = targetHeight;
    data->ascent = Int32(static_cast<int>(ascent * static_cast<float>(data->ttfScale)));
    data->pointSize = targetHeight;  // Approximate
    data->firstChar = Int32(32);   // Space
    data->lastChar = Int32(126);   // Tilde

    // Pre-calculate character widths (round instead of truncate)
    for (Int32 ch = Int32(0); static_cast<int>(ch) < 256; ch += 1)
    {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&data->ttfInfo, static_cast<int>(ch), &advanceWidth, &leftSideBearing);
        // Add 0.5f for proper rounding to avoid accumulated spacing errors
        data->charWidths[static_cast<int>(ch)] = static_cast<unsigned short>(advanceWidth * static_cast<float>(data->ttfScale) + 0.5f);
    }

    // Get font name from TTF name table (simplified - just use filename)
    const char* nameStart = path;
    const char* p = path;
    while (*p)
    {
        if (*p == '/' || *p == '\\') nameStart = p + 1;
        p++;
    }
    // Remove extension
    char nameBuf[64];
    Int32 nameLen = Int32(0);
    for (const char* q = nameStart; *q && *q != '.' && static_cast<int>(nameLen) < 63; q++)
    {
        nameBuf[static_cast<int>(nameLen)] = *q;
        nameLen += 1;
    }
    nameBuf[static_cast<int>(nameLen)] = '\0';
    data->name = String(nameBuf);

    return Font(data);
}

String Font::Name() const {
    return _data ? _data->name : String();
}

Int32 Font::Size() const {
    return _data ? _data->pointSize : Int32(0);
}

Int32 Font::Height() const {
    return _data ? _data->pixelHeight : Int32(0);
}

Int32 Font::Ascent() const {
    return _data ? _data->ascent : Int32(0);
}

FontStyle Font::Style() const {
    return _data ? _data->style : FontStyle::Regular;
}

Boolean Font::IsValid() const {
    return Boolean(_data != nullptr && static_cast<int>(_data->pixelHeight) > 0);
}

Boolean Font::IsTrueType() const {
    return Boolean(_data != nullptr && static_cast<bool>(_data->isTrueType));
}

void* Font::GetTTFInfo() const {
    if (!_data || !static_cast<bool>(_data->isTrueType)) return nullptr;
    return const_cast<stbtt_fontinfo*>(&_data->ttfInfo);
}

float Font::GetTTFScale() const {
    if (!_data || !static_cast<bool>(_data->isTrueType)) return 0.0f;
    return static_cast<float>(_data->ttfScale);
}

Int32 Font::GetCharWidth(Char c) const {
    if (!_data) return Int32(0);
    Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(c)));
    return Int32(_data->charWidths[static_cast<int>(ch)]);
}

Drawing::Size Font::MeasureString(const String& text) const {
    return MeasureString(text.GetRawString());
}

Drawing::Size Font::MeasureString(const char* text) const {
    if (!_data || !text) return Drawing::Size(Int32(0), Int32(0));

    // Check if font style includes bold (adds 1 pixel per character)
    Boolean isBold = Boolean((static_cast<unsigned char>(_data->style) &
                   static_cast<unsigned char>(FontStyle::Bold)) != 0);

    Int32 maxWidth = Int32(0);
    Int32 currentWidth = Int32(0);
    Int32 lines = Int32(1);
    Int32 charsOnLine = Int32(0);

    for (const char* p = text; *p; p++)
    {
        if (*p == '\n')
        {
            // Add extra pixels for bold characters
            if (static_cast<bool>(isBold) && static_cast<int>(charsOnLine) > 0)
            {
                currentWidth = Int32(static_cast<int>(currentWidth) + static_cast<int>(charsOnLine));
            }
            if (static_cast<int>(currentWidth) > static_cast<int>(maxWidth)) maxWidth = currentWidth;
            currentWidth = Int32(0);
            charsOnLine = Int32(0);
            lines += 1;
        }
        else
        {
            Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(*p)));
            currentWidth = Int32(static_cast<int>(currentWidth) + _data->charWidths[static_cast<int>(ch)]);
            charsOnLine += 1;
        }
    }

    // Add extra pixels for bold characters on last line
    if (static_cast<bool>(isBold) && static_cast<int>(charsOnLine) > 0)
    {
        currentWidth = Int32(static_cast<int>(currentWidth) + static_cast<int>(charsOnLine));
    }
    if (static_cast<int>(currentWidth) > static_cast<int>(maxWidth)) maxWidth = currentWidth;
    return Drawing::Size(maxWidth, Int32(static_cast<int>(lines) * static_cast<int>(_data->pixelHeight)));
}

const Image& Font::GetGlyph(Char c) const {
    static Image emptyGlyph(Int32(1), Int32(1), Color::Transparent);
    if (!_data) return emptyGlyph;

    Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(c)));
    if (!static_cast<bool>(_data->glyphCached[static_cast<int>(ch)]))
    {
        _data->RenderGlyph(ch);
    }
    return _data->glyphCache[static_cast<int>(ch)];
}

/******************************************************************************/
/*    Fast fill for rectangles (32-bit pixels)                                */
/******************************************************************************/

static void FastFillRect32(unsigned int* data, Int32 stride, Int32 x, Int32 y,
                           Int32 width, Int32 height, UInt32 color)
                           {
    for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(height); row += 1)
    {
        unsigned int* rowStart = data + (static_cast<int>(y) + static_cast<int>(row)) * static_cast<int>(stride) + static_cast<int>(x);
        for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(width); col += 1)
        {
            rowStart[static_cast<int>(col)] = static_cast<unsigned int>(color);
        }
    }
}

/******************************************************************************/
/*    Buffer writers                                                          */
/******************************************************************************/

// Global state for dirty rectangle tracking
static Rectangle g_dirtyRect = Rectangle::Empty;
static Boolean g_hasDirtyRect = Boolean(false);
static Int32 g_screenWidth = Int32(0);
static Int32 g_screenHeight = Int32(0);
static UInt8 g_videoMode = UInt8(0);

// Mark a region as dirty (needs redraw)
void MarkDirty(Int32 x, Int32 y, Int32 width, Int32 height)
{
    if (!static_cast<bool>(g_hasDirtyRect))
    {
        g_dirtyRect = Rectangle(x, y, width, height);
        g_hasDirtyRect = Boolean(true);
    }
    else
    {
        // Expand dirty rect to include new region
        Int32 gx = Int32(static_cast<int>(g_dirtyRect.x));
        Int32 gy = Int32(static_cast<int>(g_dirtyRect.y));
        Int32 gw = Int32(static_cast<int>(g_dirtyRect.width));
        Int32 gh = Int32(static_cast<int>(g_dirtyRect.height));
        Int32 left = Int32(static_cast<int>(gx) < static_cast<int>(x) ? static_cast<int>(gx) : static_cast<int>(x));
        Int32 top = Int32(static_cast<int>(gy) < static_cast<int>(y) ? static_cast<int>(gy) : static_cast<int>(y));
        Int32 right1 = Int32(static_cast<int>(gx) + static_cast<int>(gw));
        Int32 right2 = Int32(static_cast<int>(x) + static_cast<int>(width));
        Int32 right = Int32(static_cast<int>(right1) > static_cast<int>(right2) ? static_cast<int>(right1) : static_cast<int>(right2));
        Int32 bottom1 = Int32(static_cast<int>(gy) + static_cast<int>(gh));
        Int32 bottom2 = Int32(static_cast<int>(y) + static_cast<int>(height));
        Int32 bottom = Int32(static_cast<int>(bottom1) > static_cast<int>(bottom2) ? static_cast<int>(bottom1) : static_cast<int>(bottom2));
        g_dirtyRect = Rectangle(left, top, Int32(static_cast<int>(right) - static_cast<int>(left)), Int32(static_cast<int>(bottom) - static_cast<int>(top)));
    }
}

void ClearDirty()
{
    g_hasDirtyRect = Boolean(false);
    g_dirtyRect = Rectangle::Empty;
}

// Writes to frame buffer (for double buffering)
static void FrameBufferWriter(const GraphicsBuffer& buffer)
{
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    const Rectangle& bounds = buffer.Bounds();
    Int32 bx = Int32(static_cast<int>(bounds.x));
    Int32 by = Int32(static_cast<int>(bounds.y));
    Int32 bw = Int32(static_cast<int>(bounds.width));
    Int32 bh = Int32(static_cast<int>(bounds.height));
    fb->GetImage().CopyFrom(buffer.GetImage(), bounds.x, bounds.y);
    MarkDirty(bx, by, bw, bh);
}

// OPTIMIZED: Planar buffer writer with dithering - writes only dirty region
// Converts 32-bit ARGB pixels to 4-bit VGA palette using Bayer dithering
static void PlanarBufferWriterFast(const Image& img, const Rectangle& region)
{
    InitC2PTable();

    Int32 screenWidth = g_screenWidth;
    Int32 screenWidthBytes = Int32(static_cast<int>(screenWidth) / 8);

    Int32 rx = Int32(static_cast<int>(region.x));
    Int32 ry = Int32(static_cast<int>(region.y));
    Int32 rw = Int32(static_cast<int>(region.width));
    Int32 rh = Int32(static_cast<int>(region.height));

    // Align region to 8-pixel boundaries for planar mode
    Int32 x1 = Int32((static_cast<int>(rx) / 8) * 8);
    Int32 x2 = Int32(((static_cast<int>(rx) + static_cast<int>(rw) + 7) / 8) * 8);
    Int32 y1 = ry;
    Int32 y2 = Int32(static_cast<int>(ry) + static_cast<int>(rh));

    // Clamp to screen bounds
    if (static_cast<int>(x1) < 0) x1 = Int32(0);
    if (static_cast<int>(y1) < 0) y1 = Int32(0);
    if (static_cast<int>(x2) > static_cast<int>(screenWidth)) x2 = screenWidth;
    if (static_cast<int>(y2) > static_cast<int>(g_screenHeight)) y2 = g_screenHeight;

    Int32 regionWidthBytes = Int32((static_cast<int>(x2) - static_cast<int>(x1)) / 8);
    Int32 regionHeight = Int32(static_cast<int>(y2) - static_cast<int>(y1));

    if (static_cast<int>(regionWidthBytes) <= 0 || static_cast<int>(regionHeight) <= 0) return;

    // Allocate plane buffers for this region only
    Int32 regionPlaneSize = Int32(static_cast<int>(regionWidthBytes) * static_cast<int>(regionHeight));
    unsigned char* planes = static_cast<unsigned char*>(std::malloc(static_cast<int>(regionPlaneSize) * 4));
    if (!planes) return;

    std::memset(planes, 0, static_cast<int>(regionPlaneSize) * 4);

    const unsigned int* pixels = img.Data();
    Int32 imgWidth = Int32(static_cast<int>(img.Width()));

    // Convert region using lookup table - process 2 pixels at a time
    // Dither from 32-bit ARGB to 4-bit VGA palette
    for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(regionHeight); row += 1)
    {
        Int32 srcY = Int32(static_cast<int>(y1) + static_cast<int>(row));
        const unsigned int* srcRow = pixels + static_cast<int>(srcY) * static_cast<int>(imgWidth) + static_cast<int>(x1);
        Int32 dstByteOffset = Int32(static_cast<int>(row) * static_cast<int>(regionWidthBytes));

        for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(regionWidthBytes); col += 1)
        {
            Int32 srcX = Int32(static_cast<int>(col) * 8);
            unsigned char planeByte[4] = {0, 0, 0, 0};

            // Process 8 pixels (4 pairs) using lookup table
            for (Int32 pair = Int32(0); static_cast<int>(pair) < 4; pair += 1)
            {
                // Get 32-bit ARGB pixels and dither to VGA indices
                UInt32 pix0 = UInt32(srcRow[static_cast<int>(srcX) + static_cast<int>(pair) * 2]);
                UInt32 pix1 = UInt32(srcRow[static_cast<int>(srcX) + static_cast<int>(pair) * 2 + 1]);

                unsigned char p0 = DitherToVga(Int32(static_cast<int>(x1) + static_cast<int>(srcX) + static_cast<int>(pair) * 2), srcY,
                    (static_cast<unsigned int>(pix0) >> 16) & 0xFF, (static_cast<unsigned int>(pix0) >> 8) & 0xFF, static_cast<unsigned int>(pix0) & 0xFF);
                unsigned char p1 = DitherToVga(Int32(static_cast<int>(x1) + static_cast<int>(srcX) + static_cast<int>(pair) * 2 + 1), srcY,
                    (static_cast<unsigned int>(pix1) >> 16) & 0xFF, (static_cast<unsigned int>(pix1) >> 8) & 0xFF, static_cast<unsigned int>(pix1) & 0xFF);

                Int32 idx = Int32(((p0 & 0x0F) << 4) | (p1 & 0x0F));
                Int32 shift = Int32(6 - static_cast<int>(pair) * 2);

                planeByte[0] |= g_c2p_table[static_cast<int>(idx)][0] << static_cast<int>(shift);
                planeByte[1] |= g_c2p_table[static_cast<int>(idx)][1] << static_cast<int>(shift);
                planeByte[2] |= g_c2p_table[static_cast<int>(idx)][2] << static_cast<int>(shift);
                planeByte[3] |= g_c2p_table[static_cast<int>(idx)][3] << static_cast<int>(shift);
            }

            planes[0 * static_cast<int>(regionPlaneSize) + static_cast<int>(dstByteOffset) + static_cast<int>(col)] = planeByte[0];
            planes[1 * static_cast<int>(regionPlaneSize) + static_cast<int>(dstByteOffset) + static_cast<int>(col)] = planeByte[1];
            planes[2 * static_cast<int>(regionPlaneSize) + static_cast<int>(dstByteOffset) + static_cast<int>(col)] = planeByte[2];
            planes[3 * static_cast<int>(regionPlaneSize) + static_cast<int>(dstByteOffset) + static_cast<int>(col)] = planeByte[3];
        }
    }

    // Write each plane to VGA memory - only the dirty region
    Int32 startOffset = Int32(static_cast<int>(y1) * static_cast<int>(screenWidthBytes) + (static_cast<int>(x1) / 8));

    for (Int32 plane = Int32(0); static_cast<int>(plane) < 4; plane += 1)
    {
        System::IO::Devices::Display::SelectPlane(static_cast<int>(plane));

        // Copy row by row to handle stride difference
        for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(regionHeight); row += 1)
        {
            Int32 vgaOffset = Int32(static_cast<int>(startOffset) + static_cast<int>(row) * static_cast<int>(screenWidthBytes));
            System::IO::Devices::Display::CopyToVGA(
                planes + static_cast<int>(plane) * static_cast<int>(regionPlaneSize) + static_cast<int>(row) * static_cast<int>(regionWidthBytes),
                static_cast<int>(vgaOffset),
                static_cast<int>(regionWidthBytes)
            );
        }
    }

    // Reset to all planes enabled
    System::IO::Devices::Display::OutPort(0x3C4, 0x02);
    System::IO::Devices::Display::OutPort(0x3C5, 0x0F);

    std::free(planes);
}

// Full screen planar writer (for initial draw)
static void PlanarBufferWriter(const GraphicsBuffer& buffer)
{
    const Image& img = buffer.GetImage();
    Rectangle fullScreen(0, 0, img.Width(), img.Height());
    PlanarBufferWriterFast(img, fullScreen);
}

// Writes directly to VGA memory in mode 0x13 (320x200x8bpp linear)
// Dithers 32-bit ARGB to 8-bit VGA palette
static void LinearBufferWriter(const GraphicsBuffer& buffer)
{
    const Image& img = buffer.GetImage();
    Int32 width = Int32(static_cast<int>(img.Width()));
    Int32 height = Int32(static_cast<int>(img.Height()));
    const unsigned int* pixels = img.Data();

    // Allocate temporary 8-bit buffer
    unsigned char* vgaBuffer = static_cast<unsigned char*>(std::malloc(static_cast<int>(width) * static_cast<int>(height)));
    if (!vgaBuffer) return;

    // Dither each pixel
    for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
    {
        for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
        {
            UInt32 pixel = UInt32(pixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)]);
            vgaBuffer[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = DitherToVga(x, y,
                (static_cast<unsigned int>(pixel) >> 16) & 0xFF, (static_cast<unsigned int>(pixel) >> 8) & 0xFF, static_cast<unsigned int>(pixel) & 0xFF);
        }
    }

    System::IO::Devices::Display::CopyToVGA(vgaBuffer, 0, static_cast<int>(width) * static_cast<int>(height));
    std::free(vgaBuffer);
}

// Writes 32-bit image to linear framebuffer (for VBE modes)
// Uses LDT selector for proper protected mode LFB access
// Handles both 24bpp and 32bpp display modes
static void Linear32BufferWriter(const GraphicsBuffer& buffer)
{
    Int32 selector = Int32(System::IO::Devices::Display::GetLfbSelector());
    if (static_cast<int>(selector) <= 0) return;

    UInt32 pitch = UInt32(static_cast<unsigned int>(buffer.LfbPitch()));
    Int32 width = Int32(static_cast<int>(buffer.Bounds().width));
    Int32 height = Int32(static_cast<int>(buffer.Bounds().height));
    UInt8 bpp = UInt8(static_cast<unsigned char>(buffer.Bpp()));

    const Image& img = buffer.GetImage();
    const unsigned int* pixels = img.Data();

    // Static row buffer for conversion
    static unsigned char rowBuffer[4096 * 4];  // Max 4096 pixels wide, 4 bytes each
    Int32 bytesPerPixel = Int32((static_cast<unsigned char>(bpp) == 32) ? 4 : 3);

    for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
    {
        UInt32 dstOffset = UInt32(static_cast<unsigned int>(y) * static_cast<unsigned int>(pitch));

        for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
        {
            UInt32 pixel = UInt32(pixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)]);
            unsigned char r = (static_cast<unsigned int>(pixel) >> 16) & 0xFF;
            unsigned char g = (static_cast<unsigned int>(pixel) >> 8) & 0xFF;
            unsigned char b = static_cast<unsigned int>(pixel) & 0xFF;

            if (static_cast<unsigned char>(bpp) == 32)
            {
                rowBuffer[static_cast<int>(x) * 4 + 0] = b;
                rowBuffer[static_cast<int>(x) * 4 + 1] = g;
                rowBuffer[static_cast<int>(x) * 4 + 2] = r;
                rowBuffer[static_cast<int>(x) * 4 + 3] = 0xFF;
            }
            else
            {
                rowBuffer[static_cast<int>(x) * 3 + 0] = b;
                rowBuffer[static_cast<int>(x) * 3 + 1] = g;
                rowBuffer[static_cast<int>(x) * 3 + 2] = r;
            }
        }

        UInt32 rowBytes = UInt32(static_cast<unsigned int>(width) * static_cast<int>(bytesPerPixel));
        UInt32 srcOffset = UInt32(static_cast<unsigned int>(
            reinterpret_cast<unsigned long>(rowBuffer) & 0xFFFFFFFF));
        movedata(_my_ds(), static_cast<unsigned int>(srcOffset), static_cast<int>(selector), static_cast<unsigned int>(dstOffset), static_cast<unsigned int>(rowBytes));
    }
}

/******************************************************************************/
/*    GraphicsBuffer implementation                                           */
/******************************************************************************/

GraphicsBuffer* GraphicsBuffer::_frameBuffer = nullptr;
void* GraphicsBuffer::_lfbAddress = nullptr;
unsigned int GraphicsBuffer::_lfbSize = 0;

GraphicsBuffer::GraphicsBuffer(BufferWriter writer, const Rectangle& bounds,
                               unsigned char bpp, unsigned char videoMode)
    : _writer(writer), _bounds(bounds), _image(bounds.width, bounds.height)
    , _lfbPitch(0), _bpp(bpp), _videoMode(videoMode)
    {
}

GraphicsBuffer::~GraphicsBuffer()
{
}

void GraphicsBuffer::Invalidate()
{
    if (_writer)
    {
        _writer(*this);
    }
}

void GraphicsBuffer::CreateFrameBuffer(Int32 width, Int32 height, UInt8 videoMode)
{
    DestroyFrameBuffer();

    g_screenWidth = width;
    g_screenHeight = height;
    g_videoMode = videoMode;

    Rectangle bounds(0, 0, width, height);
    BufferWriter writer = nullptr;
    UInt8 bpp = UInt8(4);  // Default for VGA

    switch (static_cast<unsigned char>(g_videoMode))
    {
        case 0x12:  // 640x480x4bpp planar
            writer = PlanarBufferWriter;
            bpp = UInt8(4);
            break;
        case 0x13:  // 320x200x8bpp linear
            writer = LinearBufferWriter;
            bpp = UInt8(8);
            break;
        default:
            return;
    }

    _frameBuffer = new GraphicsBuffer(writer, bounds, static_cast<unsigned char>(bpp), static_cast<unsigned char>(g_videoMode));
    _frameBuffer->GetImage().Clear(Color::Black);

    // Initialize lookup table
    InitC2PTable();
}

void GraphicsBuffer::CreateFrameBuffer32(Int32 width, Int32 height, UInt16 /*vbeMode*/,
                                          void* /*lfbAddr*/, UInt32 pitch, UInt8 bpp)
                                          {
    DestroyFrameBuffer();

    g_screenWidth = width;
    g_screenHeight = height;
    g_videoMode = UInt8(0);  // Not a standard VGA mode

    // LFB access is now via selector, not direct address
    _lfbAddress = nullptr;
    _lfbSize = static_cast<unsigned int>(pitch) * static_cast<unsigned int>(height);

    Rectangle bounds(0, 0, width, height);
    _frameBuffer = new GraphicsBuffer(Linear32BufferWriter, bounds,
                                       static_cast<unsigned char>(bpp), 0);
    _frameBuffer->_lfbPitch = static_cast<unsigned int>(pitch);

    // Clear to black
    _frameBuffer->GetImage().Clear(Color::Black);
}

void GraphicsBuffer::DestroyFrameBuffer()
{
    if (_frameBuffer)
    {
        delete _frameBuffer;
        _frameBuffer = nullptr;
    }
    _lfbAddress = nullptr;
    _lfbSize = 0;
    ClearDirty();
}

void GraphicsBuffer::FlushFrameBuffer()
{
    if (_frameBuffer)
    {
        // Use dirty rectangle optimization for mode 0x12
        if (static_cast<unsigned char>(g_videoMode) == 0x12 && static_cast<bool>(g_hasDirtyRect))
        {
            PlanarBufferWriterFast(_frameBuffer->GetImage(), g_dirtyRect);
            ClearDirty();
        }
        else
        {
            _frameBuffer->Invalidate();
        }
    }
}

GraphicsBuffer* GraphicsBuffer::GetFrameBuffer()
{
    return _frameBuffer;
}

GraphicsBuffer* GraphicsBuffer::Create(BufferMode mode, const Rectangle& bounds)
{
    if (mode == BufferMode::Single)
    {
        return _frameBuffer;
    }
    else
    {
        return new GraphicsBuffer(FrameBufferWriter, bounds, 32, 0);
    }
}

/******************************************************************************/
/*    Graphics implementation                                                 */
/******************************************************************************/

Graphics::Graphics(BufferMode mode, const Rectangle& bounds)
    : _buffer(nullptr), _bounds(bounds), _ownsBuffer(Boolean(false))
    {
    _buffer = GraphicsBuffer::Create(mode, bounds);
    _ownsBuffer = Boolean(mode == BufferMode::Double);
}

Graphics::Graphics(BufferMode mode, Int32 x, Int32 y, Int32 width, Int32 height)
    : Graphics(mode, Rectangle(x, y, width, height))
    {
}

Graphics::~Graphics()
{
    if (static_cast<bool>(_ownsBuffer) && _buffer)
    {
        delete _buffer;
    }
}

void Graphics::Clear(const Color& color)
{
    if (!_buffer) return;
    _buffer->GetImage().Clear(color);
    MarkDirty(Int32(0), Int32(0), Int32(static_cast<int>(_bounds.width)), Int32(static_cast<int>(_bounds.height)));
}

void Graphics::DrawPixel(Int32 x, Int32 y, const Color& color)
{
    Int32 xi = x;
    Int32 yi = y;
    Int32 bw = Int32(static_cast<int>(_bounds.width));
    Int32 bh = Int32(static_cast<int>(_bounds.height));

    if (color == Color::Transparent) return;
    if (static_cast<int>(xi) < 0 || static_cast<int>(yi) < 0 || static_cast<int>(xi) >= static_cast<int>(bw) || static_cast<int>(yi) >= static_cast<int>(bh)) return;
    if (!_buffer) return;

    Image& img = _buffer->GetImage();
    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        Int32 bx = Int32(static_cast<int>(_bounds.x));
        Int32 by = Int32(static_cast<int>(_bounds.y));
        img.SetPixel(Int32(static_cast<int>(bx) + static_cast<int>(xi)), Int32(static_cast<int>(by) + static_cast<int>(yi)), color);
        MarkDirty(Int32(static_cast<int>(bx) + static_cast<int>(xi)), Int32(static_cast<int>(by) + static_cast<int>(yi)), Int32(1), Int32(1));
    }
    else
    {
        img.SetPixel(xi, yi, color);
    }
}

void Graphics::DrawPixel(const Point& pt, const Color& color)
{
    DrawPixel(pt.x, pt.y, color);
}

void Graphics::DrawLine(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const Color& color)
{
    if (color == Color::Transparent) return;

    Int32 xi1 = x1;
    Int32 yi1 = y1;
    Int32 xi2 = x2;
    Int32 yi2 = y2;

    // Bresenham's line algorithm
    Int32 dx = Int32(static_cast<int>(xi2) > static_cast<int>(xi1) ? static_cast<int>(xi2) - static_cast<int>(xi1) : static_cast<int>(xi1) - static_cast<int>(xi2));
    Int32 dy = Int32(static_cast<int>(yi2) > static_cast<int>(yi1) ? static_cast<int>(yi2) - static_cast<int>(yi1) : static_cast<int>(yi1) - static_cast<int>(yi2));
    Int32 sx = Int32(static_cast<int>(xi1) < static_cast<int>(xi2) ? 1 : -1);
    Int32 sy = Int32(static_cast<int>(yi1) < static_cast<int>(yi2) ? 1 : -1);
    Int32 err = Int32(static_cast<int>(dx) - static_cast<int>(dy));

    Int32 x = xi1;
    Int32 y = yi1;
    while (true)
    {
        DrawPixel(x, y, color);
        if (static_cast<int>(x) == static_cast<int>(xi2) && static_cast<int>(y) == static_cast<int>(yi2)) break;

        Int32 e2 = Int32(static_cast<int>(err) * 2);
        if (static_cast<int>(e2) > -static_cast<int>(dy))
        {
            err = Int32(static_cast<int>(err) - static_cast<int>(dy));
            x = Int32(static_cast<int>(x) + static_cast<int>(sx));
        }
        if (static_cast<int>(e2) < static_cast<int>(dx))
        {
            err = Int32(static_cast<int>(err) + static_cast<int>(dx));
            y = Int32(static_cast<int>(y) + static_cast<int>(sy));
        }
    }
}

void Graphics::DrawLine(const Point& p1, const Point& p2, const Color& color)
{
    DrawLine(p1.x, p1.y, p2.x, p2.y, color);
}

void Graphics::DrawRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color)
{
    if (color == Color::Transparent) return;

    Int32 xi = x;
    Int32 yi = y;
    Int32 wi = width;
    Int32 hi = height;

    Int32 x2 = Int32(static_cast<int>(xi) + static_cast<int>(wi) - 1);
    Int32 y2 = Int32(static_cast<int>(yi) + static_cast<int>(hi) - 1);

    DrawLine(xi, yi, x2, yi, color);      // Top
    DrawLine(xi, y2, x2, y2, color);    // Bottom
    DrawLine(xi, yi, xi, y2, color);      // Left
    DrawLine(x2, yi, x2, y2, color);    // Right
}

void Graphics::DrawRectangle(const Rectangle& rect, const Color& color)
{
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, color);
}

void Graphics::FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color)
{
    if (color == Color::Transparent) return;
    if (!_buffer) return;

    Int32 xi = x;
    Int32 yi = y;
    Int32 wi = width;
    Int32 hi = height;
    Int32 bw = Int32(static_cast<int>(_bounds.width));
    Int32 bh = Int32(static_cast<int>(_bounds.height));

    // Clip to bounds
    Int32 x1 = Int32(static_cast<int>(xi) < 0 ? 0 : static_cast<int>(xi));
    Int32 y1 = Int32(static_cast<int>(yi) < 0 ? 0 : static_cast<int>(yi));
    Int32 x2 = Int32(static_cast<int>(xi) + static_cast<int>(wi) > static_cast<int>(bw) ? static_cast<int>(bw) : static_cast<int>(xi) + static_cast<int>(wi));
    Int32 y2 = Int32(static_cast<int>(yi) + static_cast<int>(hi) > static_cast<int>(bh) ? static_cast<int>(bh) : static_cast<int>(yi) + static_cast<int>(hi));

    if (static_cast<int>(x1) >= static_cast<int>(x2) || static_cast<int>(y1) >= static_cast<int>(y2)) return;

    Image& img = _buffer->GetImage();
    Int32 actualX = x1;
    Int32 actualY = y1;

    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        actualX = Int32(static_cast<int>(actualX) + static_cast<int>(_bounds.x));
        actualY = Int32(static_cast<int>(actualY) + static_cast<int>(_bounds.y));
    }

    // Use fast 32-bit fill
    FastFillRect32(img.Data(), Int32(static_cast<int>(img.Width())), actualX, actualY,
                   Int32(static_cast<int>(x2) - static_cast<int>(x1)), Int32(static_cast<int>(y2) - static_cast<int>(y1)), UInt32(static_cast<unsigned int>(color)));

    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        MarkDirty(actualX, actualY, Int32(static_cast<int>(x2) - static_cast<int>(x1)), Int32(static_cast<int>(y2) - static_cast<int>(y1)));
    }
}

void Graphics::FillRectangle(const Rectangle& rect, const Color& color)
{
    FillRectangle(rect.x, rect.y, rect.width, rect.height, color);
}

void Graphics::FillRectangle(const Rectangle& rect, BorderStyle style)
{
    Int32 x = Int32(static_cast<int>(rect.x));
    Int32 y = Int32(static_cast<int>(rect.y));
    Int32 w = Int32(static_cast<int>(rect.width));
    Int32 h = Int32(static_cast<int>(rect.height));

    switch (style)
    {
        case BorderStyle::None:
            // No border, just fill with gray
            FillRectangle(x, y, w, h, Color::Gray);
            break;

        case BorderStyle::Flat:
            // Flat gray with black outline
            FillRectangle(x, y, w, h, Color::Gray);
            DrawRectangle(x, y, w, h, Color::Black);
            break;

        case BorderStyle::Raised:
            // 3D raised: white top/left, dark gray bottom/right
            FillRectangle(x, y, w, h, Color::Gray);
            DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::White);           // Top
            DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);           // Left
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::DarkGray);  // Right
            DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::DarkGray);  // Bottom
            break;

        case BorderStyle::Sunken:
            // 3D sunken: dark gray top/left, white bottom/right
            FillRectangle(x, y, w, h, Color::Gray);
            DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::DarkGray);        // Top
            DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::DarkGray);        // Left
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);  // Right
            DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);  // Bottom
            break;

        case BorderStyle::RaisedDouble:
            // Double 3D raised (button released state)
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border: White top/left, Black bottom/right
            DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::White);           // Top outer
            DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);           // Left outer
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);   // Right outer
            DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);   // Bottom outer
            // Inner border: Gray top/left, DarkGray bottom/right
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Color::Gray);        // Top inner
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::Gray);        // Left inner
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray);  // Right inner
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray);  // Bottom inner
            break;

        case BorderStyle::SunkenDouble:
            // Double 3D sunken (button pressed state)
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border: Black top/left, White bottom/right
            DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::Black);           // Top outer
            DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);           // Left outer
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);   // Right outer
            DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);   // Bottom outer
            // Inner border: DarkGray top/left, Gray bottom/right
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Color::DarkGray);    // Top inner
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray);    // Left inner
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::Gray);     // Right inner
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::Gray);     // Bottom inner
            break;

        case BorderStyle::Window:
            // Window frame style - thick 3D raised border like Windows 95
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border (row 0): White top/left, Black bottom/right
            DrawLine(x, y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Color::White);                   // Top outer
            DrawLine(x, y, x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::White);                   // Left outer
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 1), y, Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);   // Right outer
            DrawLine(x, Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 1), Color::Black);   // Bottom outer
            // Second border (row 1): White top/left, DarkGray bottom/right
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Color::White);       // Top row 1
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::White);       // Left row 1
            DrawLine(Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + 1), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray); // Right row 1
            DrawLine(Int32(static_cast<int>(x) + 1), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Int32(static_cast<int>(x) + static_cast<int>(w) - 2), Int32(static_cast<int>(y) + static_cast<int>(h) - 2), Color::DarkGray); // Bottom row 1
            break;
    }
}

void Graphics::FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                              const HatchStyle& hatch, const Color& foreColor, const Color& backColor)
{
    if (!_buffer) return;

    Int32 xi = x;
    Int32 yi = y;
    Int32 wi = width;
    Int32 hi = height;
    Int32 bw = Int32(static_cast<int>(_bounds.width));
    Int32 bh = Int32(static_cast<int>(_bounds.height));

    // Clip to bounds
    Int32 x1 = Int32(static_cast<int>(xi) < 0 ? 0 : static_cast<int>(xi));
    Int32 y1 = Int32(static_cast<int>(yi) < 0 ? 0 : static_cast<int>(yi));
    Int32 x2 = Int32(static_cast<int>(xi) + static_cast<int>(wi) > static_cast<int>(bw) ? static_cast<int>(bw) : static_cast<int>(xi) + static_cast<int>(wi));
    Int32 y2 = Int32(static_cast<int>(yi) + static_cast<int>(hi) > static_cast<int>(bh) ? static_cast<int>(bh) : static_cast<int>(yi) + static_cast<int>(hi));

    if (static_cast<int>(x1) >= static_cast<int>(x2) || static_cast<int>(y1) >= static_cast<int>(y2)) return;

    Image& img = _buffer->GetImage();
    Int32 actualX = x1;
    Int32 actualY = y1;

    // If we're drawing to a framebuffer, offset by bounds position
    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        actualX = Int32(static_cast<int>(actualX) + static_cast<int>(_bounds.x));
        actualY = Int32(static_cast<int>(actualY) + static_cast<int>(_bounds.y));
    }

    // Fill with pattern
    for (Int32 py = y1; static_cast<int>(py) < static_cast<int>(y2); py += 1)
    {
        for (Int32 px = x1; static_cast<int>(px) < static_cast<int>(x2); px += 1)
        {
            // Determine if this pixel should be foreground or background
            // Pattern repeats every 8 pixels
            Boolean isForeground = Boolean(hatch.GetBit(static_cast<int>(px), static_cast<int>(py)));

            if (static_cast<bool>(isForeground))
            {
                if (foreColor != Color::Transparent)
                {
                    Int32 destX = Int32(static_cast<int>(actualX) + (static_cast<int>(px) - static_cast<int>(x1)));
                    Int32 destY = Int32(static_cast<int>(actualY) + (static_cast<int>(py) - static_cast<int>(y1)));
                    img.SetPixel(destX, destY, foreColor);
                }
            }
            else
            {
                if (backColor != Color::Transparent)
                {
                    Int32 destX = Int32(static_cast<int>(actualX) + (static_cast<int>(px) - static_cast<int>(x1)));
                    Int32 destY = Int32(static_cast<int>(actualY) + (static_cast<int>(py) - static_cast<int>(y1)));
                    img.SetPixel(destX, destY, backColor);
                }
            }
        }
    }
}

void Graphics::FillRectangle(const Rectangle& rect,
                              const HatchStyle& hatch, const Color& foreColor, const Color& backColor)
                              {
    FillRectangle(rect.x, rect.y, rect.width, rect.height, hatch, foreColor, backColor);
}

void Graphics::DrawImage(const Image& image, Int32 x, Int32 y)
{
    if (!_buffer) return;

    Int32 xi = x;
    Int32 yi = y;

    Image& img = _buffer->GetImage();
    Int32 actualX = xi;
    Int32 actualY = yi;

    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        actualX = Int32(static_cast<int>(actualX) + static_cast<int>(_bounds.x));
        actualY = Int32(static_cast<int>(actualY) + static_cast<int>(_bounds.y));
        img.CopyFrom(image, actualX, actualY);
        MarkDirty(actualX, actualY, Int32(static_cast<int>(image.Width())),
                  Int32(static_cast<int>(image.Height())));
    }
    else
    {
        img.CopyFrom(image, xi, yi);
    }
}

void Graphics::DrawImage(const Image& image, const Point& location)
{
    DrawImage(image, location.x, location.y);
}

void Graphics::Invalidate(Boolean flushFrameBuffer)
{
    if (_buffer)
    {
        _buffer->Invalidate();
    }
    if (static_cast<bool>(flushFrameBuffer))
    {
        GraphicsBuffer::FlushFrameBuffer();
    }
}

/******************************************************************************/
/*    Graphics text rendering                                                  */
/******************************************************************************/

void Graphics::DrawString(const String& text, const Font& font, const Color& color, Int32 x, Int32 y)
{
    DrawString(text.GetRawString(), font, color, x, y);
}

void Graphics::DrawString(const char* text, const Font& font, const Color& color, Int32 x, Int32 y)
{
    if (!text || !_buffer || !font.IsValid()) return;
    if (color == Color::Transparent) return;

    Int32 curX = x;
    Int32 curY = y;
    Int32 startX = curX;
    Int32 fontHeight = font.Height();
    Int32 fontAscent = font.Ascent();

    // Check if font style includes bold (for fake bold rendering)
    Boolean isBold = Boolean((static_cast<unsigned char>(font.Style()) &
                   static_cast<unsigned char>(FontStyle::Bold)) != 0);

    Image& targetImg = _buffer->GetImage();
    Int32 offsetX = Int32(0);
    Int32 offsetY = Int32(0);

    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        offsetX = Int32(static_cast<int>(_bounds.x));
        offsetY = Int32(static_cast<int>(_bounds.y));
    }

    Int32 boundW = Int32(static_cast<int>(_bounds.width));
    Int32 boundH = Int32(static_cast<int>(_bounds.height));
    Int32 imgWidth = Int32(static_cast<int>(targetImg.Width()));
    Int32 imgHeight = Int32(static_cast<int>(targetImg.Height()));

    // Check if TTF font - use direct rendering like the working example
    Boolean isTTF = font.IsTrueType();
    stbtt_fontinfo* ttfInfo = nullptr;
    Float32 ttfScale = Float32(0.0f);
    if (static_cast<bool>(isTTF))
    {
        ttfInfo = static_cast<stbtt_fontinfo*>(font.GetTTFInfo());
        ttfScale = Float32(font.GetTTFScale());
    }

    for (const char* p = text; *p; p++)
    {
        char ch = *p;

        if (ch == '\n')
        {
            curX = startX;
            curY += fontHeight;
            continue;
        }

        if (isTTF && ttfInfo)
        {
            // Direct TTF rendering (like the working example)
            int advanceWidth, lsb;
            stbtt_GetCodepointHMetrics(ttfInfo, ch, &advanceWidth, &lsb);

            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(ttfInfo, ch, static_cast<float>(ttfScale), static_cast<float>(ttfScale),
                                        &c_x1, &c_y1, &c_x2, &c_y2);

            Int32 glyphW = Int32(c_x2 - c_x1);
            Int32 glyphH = Int32(c_y2 - c_y1);

            if (glyphW > Int32(0) && glyphH > Int32(0))
            {
                // Allocate temporary bitmap
                unsigned char* bitmap = static_cast<unsigned char*>(
                    std::malloc(static_cast<int>(glyphW) * static_cast<int>(glyphH)));
                if (bitmap)
                {
                    stbtt_MakeCodepointBitmap(ttfInfo, bitmap, static_cast<int>(glyphW), static_cast<int>(glyphH),
                                               static_cast<int>(glyphW), static_cast<float>(ttfScale), static_cast<float>(ttfScale), ch);

                    // Position: x + lsb*scale, y + ascent + c_y1
                    Int32 glyphX = curX + Int32(static_cast<int>(lsb * static_cast<float>(ttfScale) + 0.5f));
                    Int32 glyphY = curY + fontAscent + Int32(c_y1);

                    // Render bitmap
                    for (Int32 row = Int32(0); row < glyphH; row += 1)
                    {
                        Int32 destY = glyphY + row;
                        if (destY < Int32(0) || destY >= boundH) continue;

                        for (Int32 col = Int32(0); col < glyphW; col += 1)
                        {
                            Int32 destX = glyphX + col;
                            if (destX < Int32(0) || destX >= boundW) continue;

                            UInt8 gray = UInt8(bitmap[static_cast<int>(row) * static_cast<int>(glyphW) + static_cast<int>(col)]);
                            // Sharp threshold rendering - no anti-aliasing blur
                            // Use 128 threshold for crisp edges
                            if (gray > UInt8(128))
                            {
                                Int32 finalX = offsetX + destX;
                                Int32 finalY = offsetY + destY;
                                if (finalX >= Int32(0) && finalX < imgWidth && finalY >= Int32(0) && finalY < imgHeight)
                                {
                                    targetImg.SetPixel(static_cast<int>(finalX), static_cast<int>(finalY), color);
                                }
                            }
                        }
                    }
                    std::free(bitmap);
                }
            }

            // Advance cursor
            curX += Int32(static_cast<int>(advanceWidth * static_cast<float>(ttfScale) + 0.5f));
        }
        else
        {
            // FON font - use glyph cache
            const Image& glyph = font.GetGlyph(Char(ch));
            Int32 glyphW = glyph.Width();
            Int32 glyphH = glyph.Height();

            // Clip to bounds (account for extra pixel if bold)
            Int32 effectiveW = static_cast<bool>(isBold) ? glyphW + Int32(1) : glyphW;
            if (curX + effectiveW > Int32(0) && curX < boundW && curY + glyphH > Int32(0) && curY < boundH)
            {
                // Blit glyph: white pixels become text color
                for (Int32 gy = Int32(0); gy < glyphH; gy += 1)
                {
                    Int32 destY = curY + gy;
                    if (destY < Int32(0) || destY >= boundH) continue;

                    for (Int32 gx = Int32(0); gx < glyphW; gx += 1)
                    {
                        Color pixel = glyph.GetPixel(static_cast<int>(gx), static_cast<int>(gy));
                        UInt8 glyphAlpha = pixel.A();
                        if (glyphAlpha > UInt8(0))
                        {
                            // Draw at normal position
                            Int32 destX = curX + gx;
                            if (destX >= Int32(0) && destX < boundW)
                            {
                                Int32 finalX = offsetX + destX;
                                Int32 finalY = offsetY + destY;
                                if (finalX >= Int32(0) && finalX < imgWidth && finalY >= Int32(0) && finalY < imgHeight)
                                {
                                    if (glyphAlpha >= UInt8(255))
                                    {
                                        targetImg.SetPixel(static_cast<int>(finalX), static_cast<int>(finalY), color);
                                    }
                                    else
                                    {
                                        Color bg = targetImg.GetPixel(static_cast<int>(finalX), static_cast<int>(finalY));
                                        UInt8 invAlpha = UInt8(255) - glyphAlpha;
                                        UInt8 r = UInt8((static_cast<unsigned char>(color.R()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.R()) * static_cast<unsigned char>(invAlpha)) / 255);
                                        UInt8 g = UInt8((static_cast<unsigned char>(color.G()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.G()) * static_cast<unsigned char>(invAlpha)) / 255);
                                        UInt8 b = UInt8((static_cast<unsigned char>(color.B()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.B()) * static_cast<unsigned char>(invAlpha)) / 255);
                                        targetImg.SetPixel(static_cast<int>(finalX), static_cast<int>(finalY), Color(r, g, b));
                                    }
                                }
                            }
                            // For fake bold, draw again at x+1
                            if (static_cast<bool>(isBold))
                            {
                                destX = curX + gx + Int32(1);
                                if (destX >= Int32(0) && destX < boundW)
                                {
                                    Int32 finalX = offsetX + destX;
                                    Int32 finalY = offsetY + destY;
                                    if (finalX >= Int32(0) && finalX < imgWidth && finalY >= Int32(0) && finalY < imgHeight)
                                    {
                                        if (glyphAlpha >= UInt8(255))
                                        {
                                            targetImg.SetPixel(static_cast<int>(finalX), static_cast<int>(finalY), color);
                                        }
                                        else
                                        {
                                            Color bg = targetImg.GetPixel(static_cast<int>(finalX), static_cast<int>(finalY));
                                            UInt8 invAlpha = UInt8(255) - glyphAlpha;
                                            UInt8 r = UInt8((static_cast<unsigned char>(color.R()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.R()) * static_cast<unsigned char>(invAlpha)) / 255);
                                            UInt8 g = UInt8((static_cast<unsigned char>(color.G()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.G()) * static_cast<unsigned char>(invAlpha)) / 255);
                                            UInt8 b = UInt8((static_cast<unsigned char>(color.B()) * static_cast<unsigned char>(glyphAlpha) + static_cast<unsigned char>(bg.B()) * static_cast<unsigned char>(invAlpha)) / 255);
                                            targetImg.SetPixel(static_cast<int>(finalX), static_cast<int>(finalY), Color(r, g, b));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Advance cursor (extra pixel for bold)
            curX += font.GetCharWidth(Char(ch));
            if (static_cast<bool>(isBold)) curX += Int32(1);
        }
    }

    // Mark dirty region
    if (_buffer == GraphicsBuffer::GetFrameBuffer())
    {
        Drawing::Size textSize = font.MeasureString(text);
        MarkDirty(offsetX + static_cast<int>(x), offsetY + static_cast<int>(y),
                  static_cast<int>(textSize.width), static_cast<int>(textSize.height));
    }
}

void Graphics::DrawString(const String& text, const Font& font, const Color& color,
                          const Rectangle& rect, StringAlignment hAlign, StringAlignment vAlign)
{
    if (!font.IsValid()) return;

    Drawing::Size textSize = font.MeasureString(text);
    Int32 textW = textSize.width;
    Int32 textH = textSize.height;
    Int32 rectX = rect.x;
    Int32 rectY = rect.y;
    Int32 rectW = rect.width;
    Int32 rectH = rect.height;

    // Calculate X position based on horizontal alignment
    Int32 x = rectX;
    switch (hAlign)
    {
        case StringAlignment::Near:
            x = rectX;
            break;
        case StringAlignment::Center:
            x = rectX + (rectW - textW) / Int32(2);
            break;
        case StringAlignment::Far:
            x = rectX + rectW - textW;
            break;
    }

    // Calculate Y position based on vertical alignment
    Int32 y = rectY;
    switch (vAlign)
    {
        case StringAlignment::Near:
            y = rectY;
            break;
        case StringAlignment::Center:
            y = rectY + (rectH - textH) / Int32(2);
            break;
        case StringAlignment::Far:
            y = rectY + rectH - textH;
            break;
    }

    DrawString(text, font, color, x, y);
}

Drawing::Size Graphics::MeasureString(const String& text, const Font& font) const
{
    return font.MeasureString(text);
}

Drawing::Size Graphics::MeasureString(const char* text, const Font& font) const
{
    return font.MeasureString(text);
}

} // namespace System::Drawing
