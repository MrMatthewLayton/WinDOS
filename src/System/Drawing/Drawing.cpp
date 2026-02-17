#include "Drawing.hpp"
#include "../Exception.hpp"
#include "../IO/IO.hpp"
#include "../../Platform/DOS/Graphics.hpp"
#include "../../ThirdParty/stb_truetype.h"
#include <cstdlib>
#include <cstring>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <dpmi.h>

namespace System { namespace Drawing {

/******************************************************************************/
/*    VGA 16-color palette RGB values (used for dithering to 4bpp)            */
/******************************************************************************/

static const struct { unsigned char r, g, b; } g_vgaPalette[16] = {
    { 0x00, 0x00, 0x00 },  // 0: Black
    { 0x00, 0x00, 0xAA },  // 1: DarkBlue
    { 0x00, 0xAA, 0x00 },  // 2: DarkGreen
    { 0x00, 0xAA, 0xAA },  // 3: DarkCyan
    { 0xAA, 0x00, 0x00 },  // 4: DarkRed
    { 0xAA, 0x00, 0xAA },  // 5: DarkMagenta
    { 0xAA, 0x55, 0x00 },  // 6: DarkYellow (brown)
    { 0xAA, 0xAA, 0xAA },  // 7: Gray
    { 0x55, 0x55, 0x55 },  // 8: DarkGray
    { 0x55, 0x55, 0xFF },  // 9: Blue
    { 0x55, 0xFF, 0x55 },  // 10: Green
    { 0x55, 0xFF, 0xFF },  // 11: Cyan
    { 0xFF, 0x55, 0x55 },  // 12: Red
    { 0xFF, 0x55, 0xFF },  // 13: Magenta
    { 0xFF, 0xFF, 0x55 },  // 14: Yellow
    { 0xFF, 0xFF, 0xFF }   // 15: White
};

/******************************************************************************/
/*    Color constants (unified 32-bit ARGB)                                   */
/******************************************************************************/

const Color Color::Black(0xFF000000);
const Color Color::White(0xFFFFFFFF);
const Color Color::Red(0xFFFF5555);       // Match VGA bright red
const Color Color::Green(0xFF55FF55);     // Match VGA bright green
const Color Color::Blue(0xFF5555FF);      // Match VGA bright blue
const Color Color::Cyan(0xFF55FFFF);      // Match VGA bright cyan
const Color Color::Magenta(0xFFFF55FF);   // Match VGA bright magenta
const Color Color::Yellow(0xFFFFFF55);    // Match VGA bright yellow
const Color Color::Gray(0xFFAAAAAA);      // Match VGA gray
const Color Color::DarkGray(0xFF555555);  // Match VGA dark gray
const Color Color::DarkBlue(0xFF0000AA);
const Color Color::DarkGreen(0xFF00AA00);
const Color Color::DarkCyan(0xFF00AAAA);
const Color Color::DarkRed(0xFFAA0000);
const Color Color::DarkMagenta(0xFFAA00AA);
const Color Color::DarkYellow(0xFFAA5500);// Brown
const Color Color::Transparent(0x00000000);

/******************************************************************************/
/*    HatchStyle patterns (8x8 bitmaps, 1=foreground, 0=background)           */
/******************************************************************************/

// Solid patterns
const HatchStyle HatchStyle::Solid(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
const HatchStyle HatchStyle::Empty(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// Horizontal/Vertical lines
const HatchStyle HatchStyle::Horizontal(0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00);
const HatchStyle HatchStyle::Vertical(0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88);
const HatchStyle HatchStyle::Cross(0xFF, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88);

// Diagonal lines
const HatchStyle HatchStyle::ForwardDiagonal(0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80);
const HatchStyle HatchStyle::BackwardDiagonal(0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01);
const HatchStyle HatchStyle::DiagonalCross(0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81);

// Dot/percentage patterns
const HatchStyle HatchStyle::Percent05(0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x80);
const HatchStyle HatchStyle::Percent10(0x00, 0x22, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00);
const HatchStyle HatchStyle::Percent20(0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88);
const HatchStyle HatchStyle::Percent25(0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00);
const HatchStyle HatchStyle::Percent30(0x22, 0x88, 0x22, 0x00, 0x88, 0x22, 0x88, 0x00);
const HatchStyle HatchStyle::Percent40(0x55, 0x22, 0x55, 0x88, 0x55, 0x22, 0x55, 0x88);
const HatchStyle HatchStyle::Percent50(0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55);
const HatchStyle HatchStyle::Percent60(0xAA, 0xDD, 0xAA, 0x77, 0xAA, 0xDD, 0xAA, 0x77);
const HatchStyle HatchStyle::Percent70(0xDD, 0x77, 0xDD, 0xFF, 0x77, 0xDD, 0x77, 0xFF);
const HatchStyle HatchStyle::Percent75(0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF);
const HatchStyle HatchStyle::Percent80(0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF, 0x77);
const HatchStyle HatchStyle::Percent90(0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xFF, 0xFF, 0x7F);

// Light/Dark line patterns
const HatchStyle HatchStyle::LightHorizontal(0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00);
const HatchStyle HatchStyle::LightVertical(0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08);
const HatchStyle HatchStyle::DarkHorizontal(0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00);
const HatchStyle HatchStyle::DarkVertical(0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC);

// Dashed patterns
const HatchStyle HatchStyle::DashedHorizontal(0x00, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x00);
const HatchStyle HatchStyle::DashedVertical(0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00);

// Grid patterns
const HatchStyle HatchStyle::SmallGrid(0xFF, 0x11, 0x11, 0x11, 0xFF, 0x11, 0x11, 0x11);
const HatchStyle HatchStyle::LargeGrid(0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
const HatchStyle HatchStyle::DottedGrid(0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00);
const HatchStyle HatchStyle::DottedDiamond(0x08, 0x00, 0x80, 0x00, 0x08, 0x00, 0x80, 0x00);

// Special patterns
const HatchStyle HatchStyle::Brick(0xFF, 0x08, 0x08, 0x08, 0xFF, 0x80, 0x80, 0x80);
const HatchStyle HatchStyle::Weave(0x88, 0x54, 0x22, 0x45, 0x88, 0x15, 0x22, 0x51);
const HatchStyle HatchStyle::Trellis(0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11);
const HatchStyle HatchStyle::Sphere(0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18);
const HatchStyle HatchStyle::Wave(0x18, 0x24, 0x42, 0x81, 0x18, 0x24, 0x42, 0x81);
const HatchStyle HatchStyle::ZigZag(0x01, 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02);
const HatchStyle HatchStyle::Shingle(0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x81, 0x81);
const HatchStyle HatchStyle::Plaid(0xFF, 0x55, 0xFF, 0x55, 0x33, 0x55, 0x33, 0x55);

/******************************************************************************/
/*    Color methods                                                           */
/******************************************************************************/

Color Color::Lerp(const Color& c1, const Color& c2, float t) {
    if (t <= 0.0f) return c1;
    if (t >= 1.0f) return c2;

    float oneMinusT = 1.0f - t;
    unsigned char a = static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.A())) * oneMinusT +
        static_cast<float>(static_cast<unsigned char>(c2.A())) * t);
    unsigned char r = static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.R())) * oneMinusT +
        static_cast<float>(static_cast<unsigned char>(c2.R())) * t);
    unsigned char g = static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.G())) * oneMinusT +
        static_cast<float>(static_cast<unsigned char>(c2.G())) * t);
    unsigned char b = static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.B())) * oneMinusT +
        static_cast<float>(static_cast<unsigned char>(c2.B())) * t);

    return Color(UInt8(a), UInt8(r), UInt8(g), UInt8(b));
}

UInt8 Color::ToVgaIndex() const {
    return RgbToVgaIndex(R(), G(), B());
}

UInt8 Color::RgbToVgaIndex(UInt8 r, UInt8 g, UInt8 b) {
    unsigned int bestDist = 0xFFFFFFFF;
    unsigned char bestIndex = 0;

    unsigned char rv = static_cast<unsigned char>(r);
    unsigned char gv = static_cast<unsigned char>(g);
    unsigned char bv = static_cast<unsigned char>(b);

    for (unsigned char i = 0; i < 16; ++i) {
        int dr = static_cast<int>(rv) - g_vgaPalette[i].r;
        int dg = static_cast<int>(gv) - g_vgaPalette[i].g;
        int db = static_cast<int>(bv) - g_vgaPalette[i].b;
        unsigned int dist = dr * dr + dg * dg + db * db;

        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }
    return UInt8(bestIndex);
}

void Color::BuildVgaRemap(const unsigned char* paletteData, UInt32 paletteCount, unsigned char remap[16]) {
    unsigned int count = static_cast<unsigned int>(paletteCount);
    // For each palette entry, find the closest VGA color
    for (unsigned int i = 0; i < count && i < 16; ++i) {
        // BMP palette is BGRA format (4 bytes per entry)
        unsigned char b = paletteData[i * 4 + 0];
        unsigned char g = paletteData[i * 4 + 1];
        unsigned char r = paletteData[i * 4 + 2];
        remap[i] = static_cast<unsigned char>(RgbToVgaIndex(r, g, b));
    }
    // Fill remaining entries with black
    for (unsigned int i = count; i < 16; ++i) {
        remap[i] = 0;
    }
}

/******************************************************************************/
/*    Point constants                                                         */
/******************************************************************************/

const Point Point::Empty(0, 0);

/******************************************************************************/
/*    Size constants                                                          */
/******************************************************************************/

const Size Size::Empty(0, 0);
const Size Size::IconSmall(16, 16);
const Size Size::IconMedium(32, 32);
const Size Size::IconLarge(48, 48);
const Size Size::IconCursor(24, 24);

/******************************************************************************/
/*    Rectangle constants                                                     */
/******************************************************************************/

const Rectangle Rectangle::Empty(0, 0, 0, 0);

/******************************************************************************/
/*    Planar conversion lookup table                                          */
/*    Pre-computed table for fast chunky-to-planar conversion                 */
/*    Index: 2 pixels packed (p0<<4 | p1) = 256 entries                       */
/*    Each entry: 4 bytes (one per plane), 2 bits set per byte                */
/******************************************************************************/

static unsigned char g_c2p_table[256][4];
static bool g_c2p_initialized = false;

static void InitC2PTable() {
    if (g_c2p_initialized) return;

    for (int p0 = 0; p0 < 16; p0++) {
        for (int p1 = 0; p1 < 16; p1++) {
            int idx = (p0 << 4) | p1;
            for (int plane = 0; plane < 4; plane++) {
                g_c2p_table[idx][plane] =
                    (((p0 >> plane) & 1) << 1) |
                    ((p1 >> plane) & 1);
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
static unsigned char DitherToVga(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    // Get threshold from Bayer matrix (0-15, scaled to color range)
    int threshold = (g_bayerMatrix[y & 3][x & 3] - 8) * 8;  // -64 to +56

    // Apply threshold to each channel
    int rq = static_cast<int>(r) + threshold;
    int gq = static_cast<int>(g) + threshold;
    int bq = static_cast<int>(b) + threshold;

    // Clamp to valid range
    if (rq < 0) rq = 0;
    if (rq > 255) rq = 255;
    if (gq < 0) gq = 0;
    if (gq > 255) gq = 255;
    if (bq < 0) bq = 0;
    if (bq > 255) bq = 255;

    // Find closest VGA color
    return static_cast<unsigned char>(Color::RgbToVgaIndex(rq, gq, bq));
}

/******************************************************************************/
/*    Image implementation (unified 32-bit ARGB)                              */
/******************************************************************************/

void Image::_allocate(int w, int h, unsigned int fill) {
    _width = w;
    _height = h;
    int size = w * h;
    if (size > 0) {
        _data = static_cast<unsigned int*>(std::malloc(size * sizeof(unsigned int)));
        if (_data) {
            for (int i = 0; i < size; ++i) {
                _data[i] = fill;
            }
        }
    } else {
        _data = nullptr;
    }
}

void Image::_free() {
    if (_data) {
        std::free(_data);
        _data = nullptr;
    }
    _width = 0;
    _height = 0;
}

void Image::_copy(const Image& other) {
    _allocate(other._width, other._height, 0);
    if (_data && other._data) {
        std::memcpy(_data, other._data, _width * _height * sizeof(unsigned int));
    }
}

Image::Image() : _data(nullptr), _width(0), _height(0) {}

Image::Image(Int32 width, Int32 height, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0) {
    _allocate(static_cast<int>(width), static_cast<int>(height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Size& size, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0) {
    _allocate(static_cast<int>(size.width), static_cast<int>(size.height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Image& other) : _data(nullptr), _width(0), _height(0) {
    _copy(other);
}

Image::Image(Image&& other) noexcept
    : _data(other._data), _width(other._width), _height(other._height) {
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
}

Image::~Image() {
    _free();
}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        _free();
        _copy(other);
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
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

Color Image::GetPixel(Int32 x, Int32 y) const {
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    if (xi < 0 || yi < 0 || xi >= _width || yi >= _height || !_data) {
        return Color::Transparent;
    }
    return Color(_data[yi * _width + xi]);
}

void Image::SetPixel(Int32 x, Int32 y, const Color& color) {
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    if (xi < 0 || yi < 0 || xi >= _width || yi >= _height || !_data) {
        return;
    }
    _data[yi * _width + xi] = static_cast<unsigned int>(color);
}

void Image::SetPixel(const Point& pt, const Color& color) {
    SetPixel(pt.x, pt.y, color);
}

void Image::Clear(const Color& color) {
    if (_data && _width > 0 && _height > 0) {
        unsigned int val = static_cast<unsigned int>(color);
        int size = _width * _height;
        for (int i = 0; i < size; ++i) {
            _data[i] = val;
        }
    }
}

void Image::CopyFrom(const Image& src, Int32 destX, Int32 destY) {
    if (!_data || !src._data) return;

    int dstX = static_cast<int>(destX);
    int dstY = static_cast<int>(destY);

    for (int sy = 0; sy < src._height; sy++) {
        int dy = dstY + sy;
        if (dy < 0 || dy >= _height) continue;

        int srcStartX = 0;
        int dstStartX = dstX;
        int copyWidth = src._width;

        if (dstStartX < 0) {
            srcStartX = -dstStartX;
            copyWidth += dstStartX;
            dstStartX = 0;
        }
        if (dstStartX + copyWidth > _width) {
            copyWidth = _width - dstStartX;
        }
        if (copyWidth <= 0) continue;

        unsigned int* dstRow = _data + dy * _width + dstStartX;
        const unsigned int* srcRow = src._data + sy * src._width + srcStartX;
        std::memcpy(dstRow, srcRow, copyWidth * sizeof(unsigned int));
    }
}

void Image::CopyFrom(const Image& src, const Point& dest) {
    CopyFrom(src, dest.x, dest.y);
}

void Image::CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY) {
    if (!_data || !src._data) return;

    int dstX = static_cast<int>(destX);
    int dstY = static_cast<int>(destY);

    for (int sy = 0; sy < src._height; sy++) {
        int dy = dstY + sy;
        if (dy < 0 || dy >= _height) continue;

        for (int sx = 0; sx < src._width; sx++) {
            int dx = dstX + sx;
            if (dx < 0 || dx >= _width) continue;

            unsigned int pixel = src._data[sy * src._width + sx];
            // Only copy if alpha >= 128 (semi-opaque or opaque)
            if ((pixel >> 24) >= 128) {
                _data[dy * _width + dx] = pixel;
            }
        }
    }
}

Image Image::GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const {
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int wi = static_cast<int>(width);
    int hi = static_cast<int>(height);

    Image result(width, height, Color::Transparent);
    if (!_data || !result._data) return result;

    for (int dy = 0; dy < hi; dy++) {
        int sy = yi + dy;
        if (sy < 0 || sy >= _height) continue;

        int srcStartX = xi;
        int dstStartX = 0;
        int copyWidth = wi;

        if (srcStartX < 0) {
            dstStartX = -srcStartX;
            copyWidth += srcStartX;
            srcStartX = 0;
        }
        if (srcStartX + copyWidth > _width) {
            copyWidth = _width - srcStartX;
        }
        if (copyWidth <= 0) continue;

        unsigned int* dstRow = result._data + dy * wi + dstStartX;
        const unsigned int* srcRow = _data + sy * _width + srcStartX;
        std::memcpy(dstRow, srcRow, copyWidth * sizeof(unsigned int));
    }
    return result;
}

Image Image::GetRegion(const Rectangle& rect) const {
    return GetRegion(rect.x, rect.y, rect.width, rect.height);
}

Image Image::FromBitmap(const char* path) {
    const unsigned short BMP_SIGNATURE = 0x4D42;  // 'BM'
    const int FILE_HEADER_SIZE = sizeof(BitmapFileHeader);
    const int INFO_HEADER_SIZE = sizeof(BitmapInfoHeader);

    // Validate path
    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    if (fileSize < FILE_HEADER_SIZE + INFO_HEADER_SIZE) {
        throw InvalidDataException("File is too small to be a valid BMP.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory for file data.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Parse headers
    const BitmapFileHeader* fileHeader = reinterpret_cast<const BitmapFileHeader*>(fileData);
    if (static_cast<unsigned short>(fileHeader->Type()) != BMP_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("File is not a valid BMP (invalid signature).");
    }

    const BitmapInfoHeader* infoHeader = reinterpret_cast<const BitmapInfoHeader*>(fileData + FILE_HEADER_SIZE);
    int bitCount = static_cast<int>(infoHeader->BitCount());

    if (static_cast<unsigned int>(infoHeader->Compression()) != 0) {
        std::free(fileData);
        throw InvalidDataException("Compressed BMP files are not supported.");
    }

    int width = static_cast<int>(infoHeader->Width());
    int height = static_cast<int>(infoHeader->Height());

    if (width <= 0 || height <= 0) {
        std::free(fileData);
        throw InvalidDataException("BMP has invalid dimensions.");
    }

    // Get pixel data
    const unsigned char* pixelData = fileData + static_cast<unsigned int>(fileHeader->Offset());

    // Create output image (32-bit ARGB)
    Image result(width, height);
    unsigned int* outPixels = result.Data();

    // Handle different bit depths
    if (bitCount == 4) {
        // 4bpp - need palette
        unsigned int paletteCount = static_cast<unsigned int>(infoHeader->UsedColors());
        if (paletteCount == 0) paletteCount = 16;
        const unsigned char* paletteData = fileData + FILE_HEADER_SIZE +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        int bytesPerLine = (((width + 1) / 2) + 3) & ~3;

        for (int y = 0; y < height; ++y) {
            const unsigned char* row = pixelData + (height - 1 - y) * bytesPerLine;
            for (int x = 0; x < width; ++x) {
                unsigned char byteVal = row[x / 2];
                unsigned char index = ((x & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                if (index < paletteCount) {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (bitCount == 8) {
        // 8bpp - need palette
        unsigned int paletteCount = static_cast<unsigned int>(infoHeader->UsedColors());
        if (paletteCount == 0) paletteCount = 256;
        const unsigned char* paletteData = fileData + FILE_HEADER_SIZE +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        int bytesPerLine = (width + 3) & ~3;

        for (int y = 0; y < height; ++y) {
            const unsigned char* row = pixelData + (height - 1 - y) * bytesPerLine;
            for (int x = 0; x < width; ++x) {
                unsigned char index = row[x];
                if (index < paletteCount) {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (bitCount == 24) {
        // 24bpp - direct RGB
        int bytesPerLine = ((width * 3) + 3) & ~3;

        for (int y = 0; y < height; ++y) {
            const unsigned char* row = pixelData + (height - 1 - y) * bytesPerLine;
            for (int x = 0; x < width; ++x) {
                unsigned char b = row[x * 3 + 0];
                unsigned char g = row[x * 3 + 1];
                unsigned char r = row[x * 3 + 2];
                outPixels[y * width + x] = 0xFF000000 |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    } else if (bitCount == 32) {
        // 32bpp - direct BGRA
        int bytesPerLine = width * 4;

        for (int y = 0; y < height; ++y) {
            const unsigned char* row = pixelData + (height - 1 - y) * bytesPerLine;
            for (int x = 0; x < width; ++x) {
                unsigned char b = row[x * 4 + 0];
                unsigned char g = row[x * 4 + 1];
                unsigned char r = row[x * 4 + 2];
                unsigned char a = row[x * 4 + 3];
                outPixels[y * width + x] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    } else {
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

static void DecodeIconDIB(const unsigned char* iconData, int targetSize, Image& result) {
    const BitmapInfoHeader* bmpHeader = reinterpret_cast<const BitmapInfoHeader*>(iconData);
    int width = static_cast<int>(bmpHeader->Width());
    int height = static_cast<int>(bmpHeader->Height()) / 2;  // DIB height includes mask
    int bitCount = static_cast<int>(bmpHeader->BitCount());

    if (width != targetSize || height != targetSize) {
        throw InvalidDataException("Icon DIB dimensions don't match expected size.");
    }

    // Get palette (if any)
    unsigned int paletteCount = static_cast<unsigned int>(bmpHeader->UsedColors());
    if (paletteCount == 0 && bitCount <= 8) {
        paletteCount = 1u << bitCount;
    }

    const unsigned char* paletteData = iconData + static_cast<unsigned int>(bmpHeader->HeaderSize());
    const unsigned char* xorMask = paletteData + paletteCount * 4;

    // Calculate strides
    int xorStride = ((bitCount * width + 31) / 32) * 4;
    int andStride = ((width + 31) / 32) * 4;
    const unsigned char* andMask = xorMask + xorStride * height;

    unsigned int* outPixels = result.Data();

    // Decode based on bit depth
    if (bitCount == 32) {
        // 32bpp BGRA with alpha
        for (int y = 0; y < height; ++y) {
            const unsigned char* row = xorMask + (height - 1 - y) * xorStride;
            for (int x = 0; x < width; ++x) {
                unsigned char b = row[x * 4 + 0];
                unsigned char g = row[x * 4 + 1];
                unsigned char r = row[x * 4 + 2];
                unsigned char a = row[x * 4 + 3];
                outPixels[y * width + x] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    } else if (bitCount == 24) {
        // 24bpp RGB, use AND mask for transparency
        for (int y = 0; y < height; ++y) {
            const unsigned char* row = xorMask + (height - 1 - y) * xorStride;
            const unsigned char* maskRow = andMask + (height - 1 - y) * andStride;
            for (int x = 0; x < width; ++x) {
                bool transparent = ((maskRow[x / 8] >> (7 - (x & 7))) & 1) != 0;
                if (transparent) {
                    outPixels[y * width + x] = 0x00000000;
                } else {
                    unsigned char b = row[x * 3 + 0];
                    unsigned char g = row[x * 3 + 1];
                    unsigned char r = row[x * 3 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (bitCount == 8) {
        // 8bpp indexed
        for (int y = 0; y < height; ++y) {
            const unsigned char* row = xorMask + (height - 1 - y) * xorStride;
            const unsigned char* maskRow = andMask + (height - 1 - y) * andStride;
            for (int x = 0; x < width; ++x) {
                bool transparent = ((maskRow[x / 8] >> (7 - (x & 7))) & 1) != 0;
                if (transparent) {
                    outPixels[y * width + x] = 0x00000000;
                } else {
                    unsigned char index = row[x];
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (bitCount == 4) {
        // 4bpp indexed
        for (int y = 0; y < height; ++y) {
            const unsigned char* row = xorMask + (height - 1 - y) * xorStride;
            const unsigned char* maskRow = andMask + (height - 1 - y) * andStride;
            for (int x = 0; x < width; ++x) {
                bool transparent = ((maskRow[x / 8] >> (7 - (x & 7))) & 1) != 0;
                if (transparent) {
                    outPixels[y * width + x] = 0x00000000;
                } else {
                    unsigned char byteVal = row[x / 2];
                    unsigned char index = ((x & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (bitCount == 1) {
        // 1bpp monochrome
        for (int y = 0; y < height; ++y) {
            const unsigned char* row = xorMask + (height - 1 - y) * xorStride;
            const unsigned char* maskRow = andMask + (height - 1 - y) * andStride;
            for (int x = 0; x < width; ++x) {
                bool transparent = ((maskRow[x / 8] >> (7 - (x & 7))) & 1) != 0;
                if (transparent) {
                    outPixels[y * width + x] = 0x00000000;
                } else {
                    bool pixel = ((row[x / 8] >> (7 - (x & 7))) & 1) != 0;
                    unsigned char index = pixel ? 1 : 0;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[y * width + x] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else {
        throw InvalidDataException("Unsupported icon bit depth.");
    }
}

/******************************************************************************/
/*    Image::FromIcon - Load icon from standalone .ico file                    */
/******************************************************************************/

Image Image::FromIcon(const char* path, const Size& size) {
    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    int targetSize = static_cast<int>(size.width);
    if (targetSize != 16 && targetSize != 24 && targetSize != 32 && targetSize != 48) {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    if (fileSize < static_cast<int>(sizeof(IconDirectory))) {
        throw InvalidDataException("File is too small to be a valid ICO.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    const IconDirectory* dir = reinterpret_cast<const IconDirectory*>(fileData);
    if (dir->Type() != 1 || dir->Count() == 0) {
        std::free(fileData);
        throw InvalidDataException("Invalid ICO file format.");
    }

    const IconDirectoryEntry* entries = reinterpret_cast<const IconDirectoryEntry*>(
        fileData + sizeof(IconDirectory));

    // Find matching size
    const IconDirectoryEntry* chosen = nullptr;
    for (unsigned short i = 0; i < dir->Count(); ++i) {
        if (entries[i].Width() == targetSize && entries[i].Height() == targetSize) {
            chosen = &entries[i];
            break;
        }
    }

    if (!chosen) {
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

Image Image::FromIconLibrary(const char* path, Int32 iconIndex, const Size& size) {
    const unsigned short MZ_SIGNATURE = 0x5A4D;
    const unsigned int PE_SIGNATURE = 0x00004550;
    const unsigned int RT_ICON = 3;
    const unsigned int RT_GROUP_ICON = 14;

    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    int targetSize = static_cast<int>(size.width);
    if (targetSize != 16 && targetSize != 24 && targetSize != 32 && targetSize != 48) {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != MZ_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != PE_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0) {
        std::free(fileData);
        throw InvalidDataException("No resource section in file.");
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (int i = 0; i < peHeaders->GetFileHeader().SectionCount(); ++i) {
        if (rsrcDir.VirtualAddress() >= sections[i].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[i].VirtualAddress() + sections[i].VirtualSize()) {
            rsrcSection = &sections[i];
            break;
        }
    }

    if (!rsrcSection) {
        std::free(fileData);
        throw InvalidDataException("Resource section not found.");
    }

    unsigned int rsrcRva = rsrcSection->VirtualAddress();
    unsigned int rsrcOffset = rsrcSection->RawDataPointer();
    const unsigned char* rsrcBase = fileData + rsrcOffset + (rsrcDir.VirtualAddress() - rsrcRva);

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDirectoryEntry* groupIconEntry = nullptr;
    const PortableExecutableResourceDirectoryEntry* iconEntry = nullptr;

    int totalRootEntries = rootDir->TotalEntries();
    for (int i = 0; i < totalRootEntries; ++i) {
        if (!rootEntries[i].IsNamed()) {
            if (rootEntries[i].GetId() == RT_GROUP_ICON) {
                groupIconEntry = &rootEntries[i];
            } else if (rootEntries[i].GetId() == RT_ICON) {
                iconEntry = &rootEntries[i];
            }
        }
    }

    if (!groupIconEntry || !iconEntry) {
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

    int idx = static_cast<int>(iconIndex);
    if (idx < 0 || idx >= groupIconDir->TotalEntries()) {
        std::free(fileData);
        throw ArgumentException("Icon index out of range.");
    }

    // Get the specific icon group
    const PortableExecutableResourceDirectoryEntry* chosenGroup = &groupIconEntries[idx];
    if (!chosenGroup->IsDirectory()) {
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

    if (langEntry->IsDirectory()) {
        std::free(fileData);
        throw InvalidDataException("Invalid icon resource structure.");
    }

    // Get the data entry
    const PortableExecutableResourceDataEntry* dataEntry =
        reinterpret_cast<const PortableExecutableResourceDataEntry*>(
            rsrcBase + langEntry->GetOffsetToData());

    const unsigned char* groupData = fileData + rsrcOffset + (dataEntry->DataRva() - rsrcRva);

    // Parse the GROUP_ICON directory
    const IconDirectory* iconDir = reinterpret_cast<const IconDirectory*>(groupData);
    if (iconDir->Type() != 1 || iconDir->Count() == 0) {
        std::free(fileData);
        throw InvalidDataException("Invalid GROUP_ICON format.");
    }

    const GroupIconDirectoryEntry* groupEntries =
        reinterpret_cast<const GroupIconDirectoryEntry*>(groupData + sizeof(IconDirectory));

    // Find matching size
    const GroupIconDirectoryEntry* chosenIcon = nullptr;
    for (int i = 0; i < iconDir->Count(); ++i) {
        if (groupEntries[i].Width() == targetSize && groupEntries[i].Height() == targetSize) {
            chosenIcon = &groupEntries[i];
            break;
        }
    }

    if (!chosenIcon) {
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
    int totalIconEntries = iconTypeDir->TotalEntries();
    for (int i = 0; i < totalIconEntries; ++i) {
        if (!iconTypeEntries[i].IsNamed() && iconTypeEntries[i].GetId() == chosenIcon->Identifier()) {
            matchingIcon = &iconTypeEntries[i];
            break;
        }
    }

    if (!matchingIcon) {
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

    const unsigned char* iconData = fileData + rsrcOffset + (iconDataEntry->DataRva() - rsrcRva);

    Image result(targetSize, targetSize);
    DecodeIconDIB(iconData, targetSize, result);

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    Image::GetIconLibraryCount - Get number of icon groups in library        */
/******************************************************************************/

Int32 Image::GetIconLibraryCount(const char* path) {
    const unsigned short MZ_SIGNATURE = 0x5A4D;
    const unsigned int PE_SIGNATURE = 0x00004550;
    const unsigned int RT_GROUP_ICON = 14;

    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != MZ_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != PE_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0) {
        std::free(fileData);
        return Int32(0);  // No resources
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (int i = 0; i < peHeaders->GetFileHeader().SectionCount(); ++i) {
        if (rsrcDir.VirtualAddress() >= sections[i].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[i].VirtualAddress() + sections[i].VirtualSize()) {
            rsrcSection = &sections[i];
            break;
        }
    }

    if (!rsrcSection) {
        std::free(fileData);
        return Int32(0);
    }

    unsigned int rsrcRva = rsrcSection->VirtualAddress();
    unsigned int rsrcOffset = rsrcSection->RawDataPointer();
    const unsigned char* rsrcBase = fileData + rsrcOffset + (rsrcDir.VirtualAddress() - rsrcRva);

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    int totalRootEntries = rootDir->TotalEntries();
    for (int i = 0; i < totalRootEntries; ++i) {
        if (!rootEntries[i].IsNamed() && rootEntries[i].GetId() == RT_GROUP_ICON) {
            // Found RT_GROUP_ICON, count entries
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[i].GetOffsetToData());
            int count = groupIconDir->TotalEntries();
            std::free(fileData);
            return Int32(count);
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
static String ReadResourceName(const unsigned char* rsrcBase, unsigned int nameOffset) {
    // Name format: WORD length (characters), followed by UTF-16LE chars (no null)
    const unsigned char* namePtr = rsrcBase + nameOffset;
    unsigned short charCount = *reinterpret_cast<const unsigned short*>(namePtr);

    if (charCount == 0 || charCount > 256) {
        return String();  // Empty or suspiciously long
    }

    // Convert UTF-16LE to ASCII (simple conversion, ignore high bytes)
    char* asciiName = static_cast<char*>(std::malloc(charCount + 1));
    if (!asciiName) return String();

    const unsigned short* utf16Chars = reinterpret_cast<const unsigned short*>(namePtr + 2);
    for (unsigned short i = 0; i < charCount; i++) {
        // Simple conversion - just take low byte (works for ASCII names)
        asciiName[i] = static_cast<char>(utf16Chars[i] & 0xFF);
    }
    asciiName[charCount] = '\0';

    String result(asciiName);
    std::free(asciiName);
    return result;
}

Array<String> Image::GetIconLibraryNames(const char* path) {
    const unsigned short MZ_SIGNATURE = 0x5A4D;
    const unsigned int PE_SIGNATURE = 0x00004550;
    const unsigned int RT_GROUP_ICON = 14;

    if (!path || path[0] == '\0') {
        return Array<String>(0);
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) return Array<String>(0);

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Parse DOS/PE headers
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != MZ_SIGNATURE) {
        std::free(fileData);
        return Array<String>(0);
    }

    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != PE_SIGNATURE) {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0) {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (int i = 0; i < peHeaders->GetFileHeader().SectionCount(); ++i) {
        if (rsrcDir.VirtualAddress() >= sections[i].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[i].VirtualAddress() + sections[i].VirtualSize()) {
            rsrcSection = &sections[i];
            break;
        }
    }

    if (!rsrcSection) {
        std::free(fileData);
        return Array<String>(0);
    }

    unsigned int rsrcOffset = rsrcSection->RawDataPointer();
    const unsigned char* rsrcBase = fileData + rsrcOffset + (rsrcDir.VirtualAddress() - rsrcSection->VirtualAddress());

    // Find RT_GROUP_ICON directory
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    int totalRootEntries = rootDir->TotalEntries();
    for (int i = 0; i < totalRootEntries; ++i) {
        if (!rootEntries[i].IsNamed() && rootEntries[i].GetId() == RT_GROUP_ICON) {
            // Found RT_GROUP_ICON, get icon names
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[i].GetOffsetToData());
            const PortableExecutableResourceDirectoryEntry* iconEntries =
                reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
                    reinterpret_cast<const unsigned char*>(groupIconDir) + sizeof(PortableExecutableResourceDirectory));

            int count = groupIconDir->TotalEntries();
            Array<String> names(count);

            for (int j = 0; j < count; j++) {
                if (iconEntries[j].IsNamed()) {
                    names[j] = ReadResourceName(rsrcBase, iconEntries[j].GetNameOffset());
                } else {
                    // ID-based entry - return empty string or the ID as string
                    names[j] = String();
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

Int32 Image::GetIconLibraryIndex(const char* path, const char* iconName) {
    if (!path || !iconName || iconName[0] == '\0') {
        return Int32(-1);
    }

    Array<String> names = GetIconLibraryNames(path);
    String target(iconName);

    for (int i = 0; i < names.Length(); i++) {
        if (names[i].EqualsIgnoreCase(target)) {
            return Int32(i);
        }
    }

    return Int32(-1);
}

/******************************************************************************/
/*    Image::FromIconLibrary (by name) - Load icon by name                     */
/******************************************************************************/

Image Image::FromIconLibrary(const char* path, const char* iconName, const Size& size) {
    if (!iconName || iconName[0] == '\0') {
        throw ArgumentNullException("iconName");
    }

    Int32 index = GetIconLibraryIndex(path, iconName);
    if (static_cast<int>(index) < 0) {
        throw ArgumentException("Icon not found in library.");
    }

    return FromIconLibrary(path, index, size);
}

/******************************************************************************/
/*    Font::FontData - Internal font data storage                              */
/******************************************************************************/

struct Font::FontData {
    String name;                    // Font face name
    int pointSize;                  // Nominal point size
    int pixelHeight;                // Actual pixel height
    int ascent;                     // Pixels above baseline
    FontStyle style;                // Font style flags
    int firstChar;                  // First character code
    int lastChar;                   // Last character code
    bool isTrueType;                // True if TTF, false if FON

    // Character widths (256 entries, 0 for non-existent chars)
    unsigned short charWidths[256];

    // FON: Glyph offsets into bitmap data
    unsigned int charOffsets[256];

    // Raw font file data (FON bitmap or TTF file)
    unsigned char* bitmapData;
    unsigned int bitmapSize;

    // TTF: stbtt font info
    stbtt_fontinfo ttfInfo;
    float ttfScale;                 // Scale factor for pixel height

    // Glyph cache (lazily populated)
    mutable Image glyphCache[256];
    mutable bool glyphCached[256];

    FontData()
        : pointSize(0), pixelHeight(0), ascent(0), style(FontStyle::Regular)
        , firstChar(0), lastChar(0), isTrueType(false)
        , bitmapData(nullptr), bitmapSize(0), ttfScale(0.0f) {
        std::memset(charWidths, 0, sizeof(charWidths));
        std::memset(charOffsets, 0, sizeof(charOffsets));
        std::memset(glyphCached, 0, sizeof(glyphCached));
        std::memset(&ttfInfo, 0, sizeof(ttfInfo));
    }

    ~FontData() {
        if (bitmapData) {
            std::free(bitmapData);
            bitmapData = nullptr;
        }
    }

    // Render a glyph to the cache
    void RenderGlyph(int ch) const {
        if (ch < 0 || ch > 255 || glyphCached[ch]) return;

        if (isTrueType) {
            RenderTrueTypeGlyph(ch);
        } else {
            RenderFonGlyph(ch);
        }
    }

    // Render FON (bitmap) glyph
    void RenderFonGlyph(int ch) const {
        if (ch < firstChar || ch > lastChar) {
            // Character not in font - create empty glyph
            glyphCache[ch] = Image(1, pixelHeight, Color::Transparent);
            glyphCached[ch] = true;
            return;
        }

        int width = charWidths[ch];
        int height = pixelHeight;
        if (width <= 0) {
            glyphCache[ch] = Image(1, height, Color::Transparent);
            glyphCached[ch] = true;
            return;
        }

        // Create transparent glyph image
        glyphCache[ch] = Image(width, height, Color::Transparent);

        // FON bitmap format: row-major, MSB first
        // Each row is ceil(width/8) bytes
        // Rows are stored top-to-bottom
        int bytesPerRow = (width + 7) / 8;
        const unsigned char* src = bitmapData + charOffsets[ch];

        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                int byteIndex = col / 8;
                int bitIndex = 7 - (col % 8);  // MSB is leftmost pixel
                unsigned char byte = src[row * bytesPerRow + byteIndex];
                bool pixel = ((byte >> bitIndex) & 1) != 0;
                if (pixel) {
                    glyphCache[ch].SetPixel(col, row, Color::White);
                }
            }
        }

        glyphCached[ch] = true;
    }

    // Render TrueType glyph using stb_truetype
    void RenderTrueTypeGlyph(int ch) const {
        // Get horizontal metrics (advance and left side bearing)
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&ttfInfo, ch, &advanceWidth, &lsb);

        // Get bitmap bounding box
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&ttfInfo, ch, ttfScale, ttfScale, &x0, &y0, &x1, &y1);

        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        // Scale lsb to pixels
        int lsbPixels = static_cast<int>(lsb * ttfScale + 0.5f);

        // Use advance width for image width (for proper character spacing)
        int imageWidth = charWidths[ch];
        if (imageWidth <= 0) imageWidth = 1;
        int imageHeight = pixelHeight;

        // Create glyph image
        glyphCache[ch] = Image(imageWidth, imageHeight, Color::Transparent);

        if (glyphWidth <= 0 || glyphHeight <= 0) {
            // Empty glyph (e.g., space) - just return the transparent image
            glyphCached[ch] = true;
            return;
        }

        // Rasterize the glyph (8-bit grayscale)
        unsigned char* bitmap = static_cast<unsigned char*>(
            std::malloc(glyphWidth * glyphHeight));
        if (!bitmap) {
            glyphCached[ch] = true;
            return;
        }

        stbtt_MakeCodepointBitmap(&ttfInfo, bitmap, glyphWidth, glyphHeight,
                                   glyphWidth, ttfScale, ttfScale, ch);

        // Position glyph in image using the working example's approach:
        // - Horizontally: use lsb (left side bearing) scaled to pixels
        // - Vertically: use ascent + y0 for baseline alignment

        // Copy bitmap to image with anti-aliasing (store grayscale as alpha)
        for (int row = 0; row < glyphHeight; row++) {
            int destY = ascent + y0 + row;
            if (destY < 0 || destY >= imageHeight) continue;

            for (int col = 0; col < glyphWidth; col++) {
                int destX = lsbPixels + col;
                if (destX < 0 || destX >= imageWidth) continue;

                unsigned char gray = bitmap[row * glyphWidth + col];
                if (gray > 0) {
                    // Store grayscale as alpha for anti-aliasing
                    // White color with variable alpha
                    glyphCache[ch].SetPixel(destX, destY, Color(255, 255, 255, gray));
                }
            }
        }

        std::free(bitmap);
        glyphCached[ch] = true;
    }
};

/******************************************************************************/
/*    Font implementation                                                      */
/******************************************************************************/

Font::Font() : _data(nullptr) {}

Font::Font(FontData* data) : _data(data) {}

Font::Font(const Font& other) : _data(nullptr) {
    if (other._data) {
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

        if (other._data->bitmapData && other._data->bitmapSize > 0) {
            _data->bitmapData = static_cast<unsigned char*>(std::malloc(other._data->bitmapSize));
            if (_data->bitmapData) {
                std::memcpy(_data->bitmapData, other._data->bitmapData, other._data->bitmapSize);
                _data->bitmapSize = other._data->bitmapSize;

                // For TTF fonts, re-initialize ttfInfo to point to the new bitmapData
                if (_data->isTrueType) {
                    int fontOffset = stbtt_GetFontOffsetForIndex(_data->bitmapData, 0);
                    stbtt_InitFont(&_data->ttfInfo, _data->bitmapData, fontOffset);
                }
            }
        }

        // Copy cached glyphs
        for (int i = 0; i < 256; i++) {
            if (other._data->glyphCached[i]) {
                _data->glyphCache[i] = other._data->glyphCache[i];
                _data->glyphCached[i] = true;
            }
        }
    }
}

Font::Font(Font&& other) noexcept : _data(other._data) {
    other._data = nullptr;
}

Font::~Font() {
    if (_data) {
        delete _data;
        _data = nullptr;
    }
}

Font& Font::operator=(const Font& other) {
    if (this != &other) {
        if (_data) {
            delete _data;
            _data = nullptr;
        }
        if (other._data) {
            Font temp(other);
            _data = temp._data;
            temp._data = nullptr;
        }
    }
    return *this;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        if (_data) {
            delete _data;
        }
        _data = other._data;
        other._data = nullptr;
    }
    return *this;
}

Font Font::FromFile(const char* path, Int32 size, FontStyle style) {
    const unsigned short MZ_SIGNATURE = 0x5A4D;
    const unsigned short NE_SIGNATURE = 0x454E;
    const unsigned short RT_FONT = 0x8008;  // NE resource type for fonts

    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    int targetSize = static_cast<int>(size);

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    if (fileSize < 64) {
        throw InvalidDataException("File is too small to be a valid FON file.");
    }

    // Copy to raw buffer
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Parse MZ header to find NE header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != MZ_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    unsigned int neOffset = dosHeader->NewHeaderOffset();
    if (neOffset >= static_cast<unsigned int>(fileSize) - sizeof(NeHeader)) {
        std::free(fileData);
        throw InvalidDataException("Invalid NE header offset.");
    }

    const NeHeader* neHeader = reinterpret_cast<const NeHeader*>(fileData + neOffset);
    if (neHeader->Signature() != NE_SIGNATURE) {
        std::free(fileData);
        throw InvalidDataException("Invalid NE signature (not a FON file).");
    }

    // Parse resource table
    unsigned int rsrcTableOffset = neOffset + neHeader->ResourceTableOffset();
    if (rsrcTableOffset >= static_cast<unsigned int>(fileSize)) {
        std::free(fileData);
        throw InvalidDataException("Invalid resource table offset.");
    }

    // Resource table starts with alignment shift count
    const unsigned char* rsrcTable = fileData + rsrcTableOffset;
    unsigned short alignShift = *reinterpret_cast<const unsigned short*>(rsrcTable);
    rsrcTable += 2;

    // Find RT_FONT resources
    const FntHeader* bestFont = nullptr;
    int bestMatch = 0x7FFFFFFF;
    bool isBold = (static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Bold)) != 0;
    bool isItalic = (static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Italic)) != 0;

    while (true) {
        const NeResourceTypeInfo* typeInfo = reinterpret_cast<const NeResourceTypeInfo*>(rsrcTable);
        if (typeInfo->TypeId() == 0) break;  // End of resource table

        rsrcTable += sizeof(NeResourceTypeInfo);

        if (typeInfo->TypeId() == RT_FONT) {
            // Found font resources
            for (int i = 0; i < typeInfo->Count(); i++) {
                const NeResourceNameInfo* nameInfo = reinterpret_cast<const NeResourceNameInfo*>(rsrcTable);
                rsrcTable += sizeof(NeResourceNameInfo);

                // Calculate actual offset
                unsigned int fontOffset = static_cast<unsigned int>(nameInfo->Offset()) << alignShift;
                if (fontOffset >= static_cast<unsigned int>(fileSize)) continue;

                const FntHeader* fntHeader = reinterpret_cast<const FntHeader*>(fileData + fontOffset);

                // Check if this font matches our criteria
                int fontPoints = fntHeader->Points();
                bool fontBold = fntHeader->Weight() >= 700;
                bool fontItalic = fntHeader->Italic() != 0;

                // Calculate match score (lower is better)
                int sizeDiff = fontPoints > targetSize ? fontPoints - targetSize : targetSize - fontPoints;
                int styleMatch = 0;
                if (fontBold != isBold) styleMatch += 100;
                if (fontItalic != isItalic) styleMatch += 100;

                int matchScore = sizeDiff + styleMatch;
                if (matchScore < bestMatch) {
                    bestMatch = matchScore;
                    bestFont = fntHeader;
                }
            }
        } else {
            // Skip resources of other types
            rsrcTable += typeInfo->Count() * sizeof(NeResourceNameInfo);
        }
    }

    if (!bestFont) {
        std::free(fileData);
        throw InvalidDataException("No font resources found in file.");
    }

    // Parse the selected font
    FontData* data = new FontData();
    data->pointSize = bestFont->Points();
    data->pixelHeight = bestFont->PixHeight();
    data->ascent = bestFont->Ascent();
    data->firstChar = bestFont->FirstChar();
    data->lastChar = bestFont->LastChar();

    // Use requested style (allows fake bold/italic even if font doesn't have it)
    // Combine with any inherent style from the font file
    data->style = style;
    if (bestFont->Weight() >= 700) {
        data->style = data->style | FontStyle::Bold;
    }
    if (bestFont->Italic()) {
        data->style = data->style | FontStyle::Italic;
    }

    // Get face name
    unsigned int faceOffset = bestFont->dfFace;
    const unsigned char* fontBase = reinterpret_cast<const unsigned char*>(bestFont);
    if (faceOffset > 0 && faceOffset < 0x10000) {
        const char* faceName = reinterpret_cast<const char*>(fontBase + faceOffset);
        data->name = String(faceName);
    } else {
        data->name = String("Unknown");
    }

    // Calculate character widths and offsets
    bool isV3 = bestFont->Version() >= 0x0300;
    int numChars = data->lastChar - data->firstChar + 1;

    // Character table follows FntHeader (118 bytes for V2/V3)
    const unsigned char* charTable = fontBase + 118;

    if (isV3) {
        // V3.0 format: 6-byte entries (2-byte width + 4-byte offset)
        const FntCharEntryV3* entries = reinterpret_cast<const FntCharEntryV3*>(charTable);

        for (int i = 0; i <= numChars; i++) {  // +1 for sentinel
            int charCode = data->firstChar + i;
            if (charCode >= 0 && charCode < 256 && i < numChars) {
                data->charWidths[charCode] = entries[i].width;
                data->charOffsets[charCode] = entries[i].offset;
            }
        }
    } else {
        // V2.0 format: 4-byte entries (2-byte width + 2-byte offset)
        const FntCharEntryV2* entries = reinterpret_cast<const FntCharEntryV2*>(charTable);

        for (int i = 0; i < numChars; i++) {
            int charCode = data->firstChar + i;
            if (charCode >= 0 && charCode < 256) {
                data->charWidths[charCode] = entries[i].width;
                data->charOffsets[charCode] = entries[i].offset;
            }
        }
    }

    // Copy bitmap data
    // Each glyph bitmap size is: ceil(width/8) * height (row-major format)
    unsigned int bitmapStart = 0;
    unsigned int bitmapEnd = 0;
    int height = data->pixelHeight;

    if (isV3) {
        // In V3, offsets are from start of FNT resource
        bitmapStart = 0xFFFFFFFF;
        for (int i = data->firstChar; i <= data->lastChar; i++) {
            if (data->charOffsets[i] > 0 && data->charOffsets[i] < bitmapStart) {
                bitmapStart = data->charOffsets[i];
            }
            int bytesPerRow = (data->charWidths[i] + 7) / 8;
            unsigned int charEnd = data->charOffsets[i] + bytesPerRow * height;
            if (charEnd > bitmapEnd) {
                bitmapEnd = charEnd;
            }
        }
    } else {
        // In V2, offsets are from start of FNT resource
        bitmapStart = 0xFFFFFFFF;
        for (int i = data->firstChar; i <= data->lastChar; i++) {
            if (data->charOffsets[i] > 0 && data->charOffsets[i] < bitmapStart) {
                bitmapStart = data->charOffsets[i];
            }
            int bytesPerRow = (data->charWidths[i] + 7) / 8;
            unsigned int charEnd = data->charOffsets[i] + bytesPerRow * height;
            if (charEnd > bitmapEnd) {
                bitmapEnd = charEnd;
            }
        }
    }

    if (bitmapEnd > bitmapStart) {
        data->bitmapSize = bitmapEnd;  // Store entire font data to simplify offsets
        data->bitmapData = static_cast<unsigned char*>(std::malloc(data->bitmapSize));
        if (data->bitmapData) {
            std::memcpy(data->bitmapData, fontBase, data->bitmapSize);
        }
    }

    std::free(fileData);
    return Font(data);
}

Font Font::SystemFont() {
    try {
        return FromFile("MSSANS.fon", 8);
    } catch (...) {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::SystemFontBold() {
    try {
        return FromFile("MSSANS.fon", 8, FontStyle::Bold);
    } catch (...) {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FixedFont() {
    try {
        return FromFile("FIXEDSYS.fon", 8);
    } catch (...) {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FromTrueType(const char* path, Int32 pixelHeight, FontStyle style) {
    if (!path || path[0] == '\0') {
        throw ArgumentNullException("path");
    }

    int targetHeight = static_cast<int>(pixelHeight);
    if (targetHeight <= 0) {
        throw ArgumentException("pixelHeight must be positive.");
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    int fileSize = fileBytes.Length();

    if (fileSize < 12) {
        throw InvalidDataException("File is too small to be a valid TTF file.");
    }

    // Allocate and copy file data (stb_truetype requires persistent data)
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!fileData) {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (int i = 0; i < fileSize; i++) {
        fileData[i] = static_cast<unsigned char>(fileBytes[i]);
    }

    // Initialize stb_truetype
    FontData* data = new FontData();
    data->bitmapData = fileData;
    data->bitmapSize = fileSize;
    data->isTrueType = true;
    data->style = style;

    // Get font offset (handles font collections and validates TTF header)
    int fontOffset = stbtt_GetFontOffsetForIndex(fileData, 0);
    if (fontOffset < 0) {
        delete data;
        throw InvalidDataException("Invalid TTF file or font index.");
    }

    if (!stbtt_InitFont(&data->ttfInfo, fileData, fontOffset)) {
        delete data;
        throw InvalidDataException("Failed to parse TTF file.");
    }

    // Calculate scale for desired pixel height
    data->ttfScale = stbtt_ScaleForPixelHeight(&data->ttfInfo, static_cast<float>(targetHeight));

    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&data->ttfInfo, &ascent, &descent, &lineGap);

    data->pixelHeight = targetHeight;
    data->ascent = static_cast<int>(ascent * data->ttfScale);
    data->pointSize = targetHeight;  // Approximate
    data->firstChar = 32;   // Space
    data->lastChar = 126;   // Tilde

    // Pre-calculate character widths (round instead of truncate)
    for (int ch = 0; ch < 256; ch++) {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&data->ttfInfo, ch, &advanceWidth, &leftSideBearing);
        // Add 0.5f for proper rounding to avoid accumulated spacing errors
        data->charWidths[ch] = static_cast<unsigned short>(advanceWidth * data->ttfScale + 0.5f);
    }

    // Get font name from TTF name table (simplified - just use filename)
    const char* nameStart = path;
    const char* p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') nameStart = p + 1;
        p++;
    }
    // Remove extension
    char nameBuf[64];
    int nameLen = 0;
    for (const char* q = nameStart; *q && *q != '.' && nameLen < 63; q++) {
        nameBuf[nameLen++] = *q;
    }
    nameBuf[nameLen] = '\0';
    data->name = String(nameBuf);

    return Font(data);
}

String Font::Name() const {
    return _data ? _data->name : String();
}

Int32 Font::Size() const {
    return _data ? Int32(_data->pointSize) : Int32(0);
}

Int32 Font::Height() const {
    return _data ? Int32(_data->pixelHeight) : Int32(0);
}

Int32 Font::Ascent() const {
    return _data ? Int32(_data->ascent) : Int32(0);
}

FontStyle Font::Style() const {
    return _data ? _data->style : FontStyle::Regular;
}

Boolean Font::IsValid() const {
    return Boolean(_data != nullptr && _data->pixelHeight > 0);
}

Boolean Font::IsTrueType() const {
    return Boolean(_data != nullptr && _data->isTrueType);
}

void* Font::GetTTFInfo() const {
    if (!_data || !_data->isTrueType) return nullptr;
    return const_cast<stbtt_fontinfo*>(&_data->ttfInfo);
}

float Font::GetTTFScale() const {
    if (!_data || !_data->isTrueType) return 0.0f;
    return _data->ttfScale;
}

Int32 Font::GetCharWidth(Char c) const {
    if (!_data) return Int32(0);
    int ch = static_cast<int>(static_cast<unsigned char>(c));
    return Int32(_data->charWidths[ch]);
}

Drawing::Size Font::MeasureString(const String& text) const {
    return MeasureString(text.CStr());
}

Drawing::Size Font::MeasureString(const char* text) const {
    if (!_data || !text) return Drawing::Size(Int32(0), Int32(0));

    // Check if font style includes bold (adds 1 pixel per character)
    bool isBold = (static_cast<unsigned char>(_data->style) &
                   static_cast<unsigned char>(FontStyle::Bold)) != 0;

    int maxWidth = 0;
    int currentWidth = 0;
    int lines = 1;
    int charsOnLine = 0;

    for (const char* p = text; *p; p++) {
        if (*p == '\n') {
            // Add extra pixels for bold characters
            if (isBold && charsOnLine > 0) {
                currentWidth += charsOnLine;
            }
            if (currentWidth > maxWidth) maxWidth = currentWidth;
            currentWidth = 0;
            charsOnLine = 0;
            lines++;
        } else {
            int ch = static_cast<int>(static_cast<unsigned char>(*p));
            currentWidth += _data->charWidths[ch];
            charsOnLine++;
        }
    }

    // Add extra pixels for bold characters on last line
    if (isBold && charsOnLine > 0) {
        currentWidth += charsOnLine;
    }
    if (currentWidth > maxWidth) maxWidth = currentWidth;
    return Drawing::Size(Int32(maxWidth), Int32(lines * _data->pixelHeight));
}

const Image& Font::GetGlyph(Char c) const {
    static Image emptyGlyph(1, 1, Color::Transparent);
    if (!_data) return emptyGlyph;

    int ch = static_cast<int>(static_cast<unsigned char>(c));
    if (!_data->glyphCached[ch]) {
        _data->RenderGlyph(ch);
    }
    return _data->glyphCache[ch];
}

/******************************************************************************/
/*    Fast fill for rectangles (32-bit pixels)                                */
/******************************************************************************/

static void FastFillRect32(unsigned int* data, int stride, int x, int y,
                           int width, int height, unsigned int color) {
    for (int row = 0; row < height; row++) {
        unsigned int* rowStart = data + (y + row) * stride + x;
        for (int col = 0; col < width; col++) {
            rowStart[col] = color;
        }
    }
}

/******************************************************************************/
/*    Buffer writers                                                          */
/******************************************************************************/

// Global state for dirty rectangle tracking
static Rectangle g_dirtyRect = Rectangle::Empty;
static bool g_hasDirtyRect = false;
static int g_screenWidth = 0;
static int g_screenHeight = 0;
static unsigned char g_videoMode = 0;

// Mark a region as dirty (needs redraw)
void MarkDirty(int x, int y, int width, int height) {
    if (!g_hasDirtyRect) {
        g_dirtyRect = Rectangle(x, y, width, height);
        g_hasDirtyRect = true;
    } else {
        // Expand dirty rect to include new region
        int gx = static_cast<int>(g_dirtyRect.x);
        int gy = static_cast<int>(g_dirtyRect.y);
        int gw = static_cast<int>(g_dirtyRect.width);
        int gh = static_cast<int>(g_dirtyRect.height);
        int left = gx < x ? gx : x;
        int top = gy < y ? gy : y;
        int right1 = gx + gw;
        int right2 = x + width;
        int right = right1 > right2 ? right1 : right2;
        int bottom1 = gy + gh;
        int bottom2 = y + height;
        int bottom = bottom1 > bottom2 ? bottom1 : bottom2;
        g_dirtyRect = Rectangle(left, top, right - left, bottom - top);
    }
}

void ClearDirty() {
    g_hasDirtyRect = false;
    g_dirtyRect = Rectangle::Empty;
}

// Writes to frame buffer (for double buffering)
static void FrameBufferWriter(const GraphicsBuffer& buffer) {
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return;

    const Rectangle& bounds = buffer.Bounds();
    int bx = static_cast<int>(bounds.x);
    int by = static_cast<int>(bounds.y);
    int bw = static_cast<int>(bounds.width);
    int bh = static_cast<int>(bounds.height);
    fb->GetImage().CopyFrom(buffer.GetImage(), bounds.x, bounds.y);
    MarkDirty(bx, by, bw, bh);
}

// OPTIMIZED: Planar buffer writer with dithering - writes only dirty region
// Converts 32-bit ARGB pixels to 4-bit VGA palette using Bayer dithering
static void PlanarBufferWriterFast(const Image& img, const Rectangle& region) {
    InitC2PTable();

    int screenWidth = g_screenWidth;
    int screenWidthBytes = screenWidth / 8;

    int rx = static_cast<int>(region.x);
    int ry = static_cast<int>(region.y);
    int rw = static_cast<int>(region.width);
    int rh = static_cast<int>(region.height);

    // Align region to 8-pixel boundaries for planar mode
    int x1 = (rx / 8) * 8;
    int x2 = ((rx + rw + 7) / 8) * 8;
    int y1 = ry;
    int y2 = ry + rh;

    // Clamp to screen bounds
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > screenWidth) x2 = screenWidth;
    if (y2 > g_screenHeight) y2 = g_screenHeight;

    int regionWidthBytes = (x2 - x1) / 8;
    int regionHeight = y2 - y1;

    if (regionWidthBytes <= 0 || regionHeight <= 0) return;

    // Allocate plane buffers for this region only
    int regionPlaneSize = regionWidthBytes * regionHeight;
    unsigned char* planes = static_cast<unsigned char*>(std::malloc(regionPlaneSize * 4));
    if (!planes) return;

    std::memset(planes, 0, regionPlaneSize * 4);

    const unsigned int* pixels = img.Data();
    int imgWidth = static_cast<int>(img.Width());

    // Convert region using lookup table - process 2 pixels at a time
    // Dither from 32-bit ARGB to 4-bit VGA palette
    for (int row = 0; row < regionHeight; row++) {
        int srcY = y1 + row;
        const unsigned int* srcRow = pixels + srcY * imgWidth + x1;
        int dstByteOffset = row * regionWidthBytes;

        for (int col = 0; col < regionWidthBytes; col++) {
            int srcX = col * 8;
            unsigned char planeByte[4] = {0, 0, 0, 0};

            // Process 8 pixels (4 pairs) using lookup table
            for (int pair = 0; pair < 4; pair++) {
                // Get 32-bit ARGB pixels and dither to VGA indices
                unsigned int pix0 = srcRow[srcX + pair * 2];
                unsigned int pix1 = srcRow[srcX + pair * 2 + 1];

                unsigned char p0 = DitherToVga(x1 + srcX + pair * 2, srcY,
                    (pix0 >> 16) & 0xFF, (pix0 >> 8) & 0xFF, pix0 & 0xFF);
                unsigned char p1 = DitherToVga(x1 + srcX + pair * 2 + 1, srcY,
                    (pix1 >> 16) & 0xFF, (pix1 >> 8) & 0xFF, pix1 & 0xFF);

                int idx = ((p0 & 0x0F) << 4) | (p1 & 0x0F);
                int shift = 6 - pair * 2;

                planeByte[0] |= g_c2p_table[idx][0] << shift;
                planeByte[1] |= g_c2p_table[idx][1] << shift;
                planeByte[2] |= g_c2p_table[idx][2] << shift;
                planeByte[3] |= g_c2p_table[idx][3] << shift;
            }

            planes[0 * regionPlaneSize + dstByteOffset + col] = planeByte[0];
            planes[1 * regionPlaneSize + dstByteOffset + col] = planeByte[1];
            planes[2 * regionPlaneSize + dstByteOffset + col] = planeByte[2];
            planes[3 * regionPlaneSize + dstByteOffset + col] = planeByte[3];
        }
    }

    // Write each plane to VGA memory - only the dirty region
    int startOffset = y1 * screenWidthBytes + (x1 / 8);

    for (int plane = 0; plane < 4; plane++) {
        Platform::DOS::Graphics::SelectPlane(plane);

        // Copy row by row to handle stride difference
        for (int row = 0; row < regionHeight; row++) {
            int vgaOffset = startOffset + row * screenWidthBytes;
            Platform::DOS::Graphics::CopyToVGA(
                planes + plane * regionPlaneSize + row * regionWidthBytes,
                vgaOffset,
                regionWidthBytes
            );
        }
    }

    // Reset to all planes enabled
    Platform::DOS::Graphics::OutPort(0x3C4, 0x02);
    Platform::DOS::Graphics::OutPort(0x3C5, 0x0F);

    std::free(planes);
}

// Full screen planar writer (for initial draw)
static void PlanarBufferWriter(const GraphicsBuffer& buffer) {
    const Image& img = buffer.GetImage();
    Rectangle fullScreen(0, 0, img.Width(), img.Height());
    PlanarBufferWriterFast(img, fullScreen);
}

// Writes directly to VGA memory in mode 0x13 (320x200x8bpp linear)
// Dithers 32-bit ARGB to 8-bit VGA palette
static void LinearBufferWriter(const GraphicsBuffer& buffer) {
    const Image& img = buffer.GetImage();
    int width = static_cast<int>(img.Width());
    int height = static_cast<int>(img.Height());
    const unsigned int* pixels = img.Data();

    // Allocate temporary 8-bit buffer
    unsigned char* vgaBuffer = static_cast<unsigned char*>(std::malloc(width * height));
    if (!vgaBuffer) return;

    // Dither each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned int pixel = pixels[y * width + x];
            vgaBuffer[y * width + x] = DitherToVga(x, y,
                (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF);
        }
    }

    Platform::DOS::Graphics::CopyToVGA(vgaBuffer, 0, width * height);
    std::free(vgaBuffer);
}

// Writes 32-bit image to linear framebuffer (for VBE modes)
// Uses LDT selector for proper protected mode LFB access
// Handles both 24bpp and 32bpp display modes
static void Linear32BufferWriter(const GraphicsBuffer& buffer) {
    int selector = Platform::DOS::Graphics::GetLfbSelector();
    if (selector <= 0) return;

    unsigned int pitch = static_cast<unsigned int>(buffer.LfbPitch());
    int width = static_cast<int>(buffer.Bounds().width);
    int height = static_cast<int>(buffer.Bounds().height);
    unsigned char bpp = static_cast<unsigned char>(buffer.Bpp());

    const Image& img = buffer.GetImage();
    const unsigned int* pixels = img.Data();

    // Static row buffer for conversion
    static unsigned char rowBuffer[4096 * 4];  // Max 4096 pixels wide, 4 bytes each
    int bytesPerPixel = (bpp == 32) ? 4 : 3;

    for (int y = 0; y < height; y++) {
        unsigned int dstOffset = static_cast<unsigned int>(y) * pitch;

        for (int x = 0; x < width; x++) {
            unsigned int pixel = pixels[y * width + x];
            unsigned char r = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >> 8) & 0xFF;
            unsigned char b = pixel & 0xFF;

            if (bpp == 32) {
                rowBuffer[x * 4 + 0] = b;
                rowBuffer[x * 4 + 1] = g;
                rowBuffer[x * 4 + 2] = r;
                rowBuffer[x * 4 + 3] = 0xFF;
            } else {
                rowBuffer[x * 3 + 0] = b;
                rowBuffer[x * 3 + 1] = g;
                rowBuffer[x * 3 + 2] = r;
            }
        }

        unsigned int rowBytes = static_cast<unsigned int>(width) * bytesPerPixel;
        unsigned int srcOffset = static_cast<unsigned int>(
            reinterpret_cast<unsigned long>(rowBuffer) & 0xFFFFFFFF);
        movedata(_my_ds(), srcOffset, selector, dstOffset, rowBytes);
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
    , _lfbPitch(0), _bpp(bpp), _videoMode(videoMode) {
}

GraphicsBuffer::~GraphicsBuffer() {
}

void GraphicsBuffer::Invalidate() {
    if (_writer) {
        _writer(*this);
    }
}

void GraphicsBuffer::CreateFrameBuffer(Int32 width, Int32 height, UInt8 videoMode) {
    DestroyFrameBuffer();

    g_screenWidth = static_cast<int>(width);
    g_screenHeight = static_cast<int>(height);
    g_videoMode = static_cast<unsigned char>(videoMode);

    Rectangle bounds(0, 0, width, height);
    BufferWriter writer = nullptr;
    unsigned char bpp = 4;  // Default for VGA

    switch (g_videoMode) {
        case 0x12:  // 640x480x4bpp planar
            writer = PlanarBufferWriter;
            bpp = 4;
            break;
        case 0x13:  // 320x200x8bpp linear
            writer = LinearBufferWriter;
            bpp = 8;
            break;
        default:
            return;
    }

    _frameBuffer = new GraphicsBuffer(writer, bounds, bpp, g_videoMode);
    _frameBuffer->GetImage().Clear(Color::Black);

    // Initialize lookup table
    InitC2PTable();
}

void GraphicsBuffer::CreateFrameBuffer32(Int32 width, Int32 height, UInt16 /*vbeMode*/,
                                          void* /*lfbAddr*/, UInt32 pitch, UInt8 bpp) {
    DestroyFrameBuffer();

    g_screenWidth = static_cast<int>(width);
    g_screenHeight = static_cast<int>(height);
    g_videoMode = 0;  // Not a standard VGA mode

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

void GraphicsBuffer::DestroyFrameBuffer() {
    if (_frameBuffer) {
        delete _frameBuffer;
        _frameBuffer = nullptr;
    }
    _lfbAddress = nullptr;
    _lfbSize = 0;
    ClearDirty();
}

void GraphicsBuffer::FlushFrameBuffer() {
    if (_frameBuffer) {
        // Use dirty rectangle optimization for mode 0x12
        if (g_videoMode == 0x12 && g_hasDirtyRect) {
            PlanarBufferWriterFast(_frameBuffer->GetImage(), g_dirtyRect);
            ClearDirty();
        } else {
            _frameBuffer->Invalidate();
        }
    }
}

GraphicsBuffer* GraphicsBuffer::GetFrameBuffer() {
    return _frameBuffer;
}

GraphicsBuffer* GraphicsBuffer::Create(BufferMode mode, const Rectangle& bounds) {
    if (mode == BufferMode::Single) {
        return _frameBuffer;
    } else {
        return new GraphicsBuffer(FrameBufferWriter, bounds, 32, 0);
    }
}

/******************************************************************************/
/*    Graphics implementation                                                 */
/******************************************************************************/

Graphics::Graphics(BufferMode mode, const Rectangle& bounds)
    : _buffer(nullptr), _bounds(bounds), _ownsBuffer(false) {
    _buffer = GraphicsBuffer::Create(mode, bounds);
    _ownsBuffer = (mode == BufferMode::Double);
}

Graphics::Graphics(BufferMode mode, Int32 x, Int32 y, Int32 width, Int32 height)
    : Graphics(mode, Rectangle(x, y, width, height)) {
}

Graphics::~Graphics() {
    if (_ownsBuffer && _buffer) {
        delete _buffer;
    }
}

void Graphics::Clear(const Color& color) {
    if (!_buffer) return;
    _buffer->GetImage().Clear(color);
    MarkDirty(0, 0, static_cast<int>(_bounds.width), static_cast<int>(_bounds.height));
}

void Graphics::DrawPixel(Int32 x, Int32 y, const Color& color) {
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int bw = static_cast<int>(_bounds.width);
    int bh = static_cast<int>(_bounds.height);

    if (color == Color::Transparent) return;
    if (xi < 0 || yi < 0 || xi >= bw || yi >= bh) return;
    if (!_buffer) return;

    Image& img = _buffer->GetImage();
    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        int bx = static_cast<int>(_bounds.x);
        int by = static_cast<int>(_bounds.y);
        img.SetPixel(bx + xi, by + yi, color);
        MarkDirty(bx + xi, by + yi, 1, 1);
    } else {
        img.SetPixel(xi, yi, color);
    }
}

void Graphics::DrawPixel(const Point& pt, const Color& color) {
    DrawPixel(pt.x, pt.y, color);
}

void Graphics::DrawLine(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const Color& color) {
    if (color == Color::Transparent) return;

    int xi1 = static_cast<int>(x1);
    int yi1 = static_cast<int>(y1);
    int xi2 = static_cast<int>(x2);
    int yi2 = static_cast<int>(y2);

    // Bresenham's line algorithm
    int dx = xi2 > xi1 ? xi2 - xi1 : xi1 - xi2;
    int dy = yi2 > yi1 ? yi2 - yi1 : yi1 - yi2;
    int sx = xi1 < xi2 ? 1 : -1;
    int sy = yi1 < yi2 ? 1 : -1;
    int err = dx - dy;

    int x = xi1, y = yi1;
    while (true) {
        DrawPixel(x, y, color);
        if (x == xi2 && y == yi2) break;

        int e2 = err * 2;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

void Graphics::DrawLine(const Point& p1, const Point& p2, const Color& color) {
    DrawLine(p1.x, p1.y, p2.x, p2.y, color);
}

void Graphics::DrawRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color) {
    if (color == Color::Transparent) return;

    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int wi = static_cast<int>(width);
    int hi = static_cast<int>(height);

    int x2 = xi + wi - 1;
    int y2 = yi + hi - 1;

    DrawLine(xi, yi, x2, yi, color);      // Top
    DrawLine(xi, y2, x2, y2, color);    // Bottom
    DrawLine(xi, yi, xi, y2, color);      // Left
    DrawLine(x2, yi, x2, y2, color);    // Right
}

void Graphics::DrawRectangle(const Rectangle& rect, const Color& color) {
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, color);
}

void Graphics::FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color) {
    if (color == Color::Transparent) return;
    if (!_buffer) return;

    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int wi = static_cast<int>(width);
    int hi = static_cast<int>(height);
    int bw = static_cast<int>(_bounds.width);
    int bh = static_cast<int>(_bounds.height);

    // Clip to bounds
    int x1 = xi < 0 ? 0 : xi;
    int y1 = yi < 0 ? 0 : yi;
    int x2 = xi + wi > bw ? bw : xi + wi;
    int y2 = yi + hi > bh ? bh : yi + hi;

    if (x1 >= x2 || y1 >= y2) return;

    Image& img = _buffer->GetImage();
    int actualX = x1;
    int actualY = y1;

    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        actualX += static_cast<int>(_bounds.x);
        actualY += static_cast<int>(_bounds.y);
    }

    // Use fast 32-bit fill
    FastFillRect32(img.Data(), static_cast<int>(img.Width()), actualX, actualY,
                   x2 - x1, y2 - y1, static_cast<unsigned int>(color));

    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        MarkDirty(actualX, actualY, x2 - x1, y2 - y1);
    }
}

void Graphics::FillRectangle(const Rectangle& rect, const Color& color) {
    FillRectangle(rect.x, rect.y, rect.width, rect.height, color);
}

void Graphics::FillRectangle(const Rectangle& rect, BorderStyle style) {
    int x = static_cast<int>(rect.x);
    int y = static_cast<int>(rect.y);
    int w = static_cast<int>(rect.width);
    int h = static_cast<int>(rect.height);

    switch (style) {
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
            DrawLine(x, y, x + w - 1, y, Color::White);           // Top
            DrawLine(x, y, x, y + h - 1, Color::White);           // Left
            DrawLine(x + w - 1, y, x + w - 1, y + h - 1, Color::DarkGray);  // Right
            DrawLine(x, y + h - 1, x + w - 1, y + h - 1, Color::DarkGray);  // Bottom
            break;

        case BorderStyle::Sunken:
            // 3D sunken: dark gray top/left, white bottom/right
            FillRectangle(x, y, w, h, Color::Gray);
            DrawLine(x, y, x + w - 1, y, Color::DarkGray);        // Top
            DrawLine(x, y, x, y + h - 1, Color::DarkGray);        // Left
            DrawLine(x + w - 1, y, x + w - 1, y + h - 1, Color::White);  // Right
            DrawLine(x, y + h - 1, x + w - 1, y + h - 1, Color::White);  // Bottom
            break;

        case BorderStyle::RaisedDouble:
            // Double 3D raised (button released state)
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border: White top/left, Black bottom/right
            DrawLine(x, y, x + w - 1, y, Color::White);           // Top outer
            DrawLine(x, y, x, y + h - 1, Color::White);           // Left outer
            DrawLine(x + w - 1, y, x + w - 1, y + h - 1, Color::Black);   // Right outer
            DrawLine(x, y + h - 1, x + w - 1, y + h - 1, Color::Black);   // Bottom outer
            // Inner border: Gray top/left, DarkGray bottom/right
            DrawLine(x + 1, y + 1, x + w - 2, y + 1, Color::Gray);        // Top inner
            DrawLine(x + 1, y + 1, x + 1, y + h - 2, Color::Gray);        // Left inner
            DrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2, Color::DarkGray);  // Right inner
            DrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2, Color::DarkGray);  // Bottom inner
            break;

        case BorderStyle::SunkenDouble:
            // Double 3D sunken (button pressed state)
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border: Black top/left, White bottom/right
            DrawLine(x, y, x + w - 1, y, Color::Black);           // Top outer
            DrawLine(x, y, x, y + h - 1, Color::Black);           // Left outer
            DrawLine(x + w - 1, y, x + w - 1, y + h - 1, Color::White);   // Right outer
            DrawLine(x, y + h - 1, x + w - 1, y + h - 1, Color::White);   // Bottom outer
            // Inner border: DarkGray top/left, Gray bottom/right
            DrawLine(x + 1, y + 1, x + w - 2, y + 1, Color::DarkGray);    // Top inner
            DrawLine(x + 1, y + 1, x + 1, y + h - 2, Color::DarkGray);    // Left inner
            DrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2, Color::Gray);     // Right inner
            DrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2, Color::Gray);     // Bottom inner
            break;

        case BorderStyle::Window:
            // Window frame style - thick 3D raised border like Windows 95
            FillRectangle(x, y, w, h, Color::Gray);
            // Outer border (row 0): White top/left, Black bottom/right
            DrawLine(x, y, x + w - 1, y, Color::White);                   // Top outer
            DrawLine(x, y, x, y + h - 1, Color::White);                   // Left outer
            DrawLine(x + w - 1, y, x + w - 1, y + h - 1, Color::Black);   // Right outer
            DrawLine(x, y + h - 1, x + w - 1, y + h - 1, Color::Black);   // Bottom outer
            // Second border (row 1): White top/left, DarkGray bottom/right
            DrawLine(x + 1, y + 1, x + w - 2, y + 1, Color::White);       // Top row 1
            DrawLine(x + 1, y + 1, x + 1, y + h - 2, Color::White);       // Left row 1
            DrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2, Color::DarkGray); // Right row 1
            DrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2, Color::DarkGray); // Bottom row 1
            break;
    }
}

void Graphics::FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                              const HatchStyle& hatch, const Color& foreColor, const Color& backColor) {
    if (!_buffer) return;

    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int wi = static_cast<int>(width);
    int hi = static_cast<int>(height);
    int bw = static_cast<int>(_bounds.width);
    int bh = static_cast<int>(_bounds.height);

    // Clip to bounds
    int x1 = xi < 0 ? 0 : xi;
    int y1 = yi < 0 ? 0 : yi;
    int x2 = xi + wi > bw ? bw : xi + wi;
    int y2 = yi + hi > bh ? bh : yi + hi;

    if (x1 >= x2 || y1 >= y2) return;

    Image& img = _buffer->GetImage();
    int actualX = x1;
    int actualY = y1;

    // If we're drawing to a framebuffer, offset by bounds position
    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        actualX += static_cast<int>(_bounds.x);
        actualY += static_cast<int>(_bounds.y);
    }

    // Fill with pattern
    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            // Determine if this pixel should be foreground or background
            // Pattern repeats every 8 pixels
            bool isForeground = hatch.GetBit(px, py);

            if (isForeground) {
                if (foreColor != Color::Transparent) {
                    int destX = actualX + (px - x1);
                    int destY = actualY + (py - y1);
                    img.SetPixel(destX, destY, foreColor);
                }
            } else {
                if (backColor != Color::Transparent) {
                    int destX = actualX + (px - x1);
                    int destY = actualY + (py - y1);
                    img.SetPixel(destX, destY, backColor);
                }
            }
        }
    }
}

void Graphics::FillRectangle(const Rectangle& rect,
                              const HatchStyle& hatch, const Color& foreColor, const Color& backColor) {
    FillRectangle(rect.x, rect.y, rect.width, rect.height, hatch, foreColor, backColor);
}

void Graphics::DrawImage(const Image& image, Int32 x, Int32 y) {
    if (!_buffer) return;

    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);

    Image& img = _buffer->GetImage();
    int actualX = xi;
    int actualY = yi;

    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        actualX += static_cast<int>(_bounds.x);
        actualY += static_cast<int>(_bounds.y);
        img.CopyFrom(image, actualX, actualY);
        MarkDirty(actualX, actualY, static_cast<int>(image.Width()),
                  static_cast<int>(image.Height()));
    } else {
        img.CopyFrom(image, xi, yi);
    }
}

void Graphics::DrawImage(const Image& image, const Point& location) {
    DrawImage(image, location.x, location.y);
}

void Graphics::Invalidate(Boolean flushFrameBuffer) {
    if (_buffer) {
        _buffer->Invalidate();
    }
    if (static_cast<bool>(flushFrameBuffer)) {
        GraphicsBuffer::FlushFrameBuffer();
    }
}

/******************************************************************************/
/*    Graphics text rendering                                                  */
/******************************************************************************/

void Graphics::DrawString(const String& text, const Font& font, const Color& color, Int32 x, Int32 y) {
    DrawString(text.CStr(), font, color, x, y);
}

void Graphics::DrawString(const char* text, const Font& font, const Color& color, Int32 x, Int32 y) {
    if (!text || !_buffer || !font.IsValid()) return;
    if (color == Color::Transparent) return;

    int curX = static_cast<int>(x);
    int curY = static_cast<int>(y);
    int startX = curX;
    int fontHeight = static_cast<int>(font.Height());
    int fontAscent = static_cast<int>(font.Ascent());

    // Check if font style includes bold (for fake bold rendering)
    bool isBold = (static_cast<unsigned char>(font.Style()) &
                   static_cast<unsigned char>(FontStyle::Bold)) != 0;

    Image& targetImg = _buffer->GetImage();
    int offsetX = 0;
    int offsetY = 0;

    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        offsetX = static_cast<int>(_bounds.x);
        offsetY = static_cast<int>(_bounds.y);
    }

    int boundW = static_cast<int>(_bounds.width);
    int boundH = static_cast<int>(_bounds.height);
    int imgWidth = static_cast<int>(targetImg.Width());
    int imgHeight = static_cast<int>(targetImg.Height());

    // Check if TTF font - use direct rendering like the working example
    bool isTTF = static_cast<bool>(font.IsTrueType());
    stbtt_fontinfo* ttfInfo = nullptr;
    float ttfScale = 0.0f;
    if (isTTF) {
        ttfInfo = static_cast<stbtt_fontinfo*>(font.GetTTFInfo());
        ttfScale = font.GetTTFScale();
    }

    for (const char* p = text; *p; p++) {
        char ch = *p;

        if (ch == '\n') {
            curX = startX;
            curY += fontHeight;
            continue;
        }

        if (isTTF && ttfInfo) {
            // Direct TTF rendering (like the working example)
            int advanceWidth, lsb;
            stbtt_GetCodepointHMetrics(ttfInfo, ch, &advanceWidth, &lsb);

            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(ttfInfo, ch, ttfScale, ttfScale,
                                        &c_x1, &c_y1, &c_x2, &c_y2);

            int glyphW = c_x2 - c_x1;
            int glyphH = c_y2 - c_y1;

            if (glyphW > 0 && glyphH > 0) {
                // Allocate temporary bitmap
                unsigned char* bitmap = static_cast<unsigned char*>(
                    std::malloc(glyphW * glyphH));
                if (bitmap) {
                    stbtt_MakeCodepointBitmap(ttfInfo, bitmap, glyphW, glyphH,
                                               glyphW, ttfScale, ttfScale, ch);

                    // Position: x + lsb*scale, y + ascent + c_y1
                    int glyphX = curX + static_cast<int>(lsb * ttfScale + 0.5f);
                    int glyphY = curY + fontAscent + c_y1;

                    // Render bitmap
                    for (int row = 0; row < glyphH; row++) {
                        int destY = glyphY + row;
                        if (destY < 0 || destY >= boundH) continue;

                        for (int col = 0; col < glyphW; col++) {
                            int destX = glyphX + col;
                            if (destX < 0 || destX >= boundW) continue;

                            unsigned char gray = bitmap[row * glyphW + col];
                            // Sharp threshold rendering - no anti-aliasing blur
                            // Use 128 threshold for crisp edges
                            if (gray > 128) {
                                int finalX = offsetX + destX;
                                int finalY = offsetY + destY;
                                if (finalX >= 0 && finalX < imgWidth && finalY >= 0 && finalY < imgHeight) {
                                    targetImg.SetPixel(finalX, finalY, color);
                                }
                            }
                        }
                    }
                    std::free(bitmap);
                }
            }

            // Advance cursor
            curX += static_cast<int>(advanceWidth * ttfScale + 0.5f);
        } else {
            // FON font - use glyph cache
            const Image& glyph = font.GetGlyph(Char(ch));
            int glyphW = static_cast<int>(glyph.Width());
            int glyphH = static_cast<int>(glyph.Height());

            // Clip to bounds (account for extra pixel if bold)
            int effectiveW = isBold ? glyphW + 1 : glyphW;
            if (curX + effectiveW > 0 && curX < boundW && curY + glyphH > 0 && curY < boundH) {
                // Blit glyph: white pixels become text color
                for (int gy = 0; gy < glyphH; gy++) {
                    int destY = curY + gy;
                    if (destY < 0 || destY >= boundH) continue;

                    for (int gx = 0; gx < glyphW; gx++) {
                        Color pixel = glyph.GetPixel(gx, gy);
                        unsigned char glyphAlpha = static_cast<unsigned char>(pixel.A());
                        if (glyphAlpha > 0) {
                            // Draw at normal position
                            int destX = curX + gx;
                            if (destX >= 0 && destX < boundW) {
                                int finalX = offsetX + destX;
                                int finalY = offsetY + destY;
                                if (finalX >= 0 && finalX < imgWidth && finalY >= 0 && finalY < imgHeight) {
                                    if (glyphAlpha >= 255) {
                                        targetImg.SetPixel(finalX, finalY, color);
                                    } else {
                                        Color bg = targetImg.GetPixel(finalX, finalY);
                                        unsigned char invAlpha = 255 - glyphAlpha;
                                        unsigned char r = static_cast<unsigned char>((static_cast<unsigned char>(color.R()) * glyphAlpha + static_cast<unsigned char>(bg.R()) * invAlpha) / 255);
                                        unsigned char g = static_cast<unsigned char>((static_cast<unsigned char>(color.G()) * glyphAlpha + static_cast<unsigned char>(bg.G()) * invAlpha) / 255);
                                        unsigned char b = static_cast<unsigned char>((static_cast<unsigned char>(color.B()) * glyphAlpha + static_cast<unsigned char>(bg.B()) * invAlpha) / 255);
                                        targetImg.SetPixel(finalX, finalY, Color(r, g, b));
                                    }
                                }
                            }
                            // For fake bold, draw again at x+1
                            if (isBold) {
                                destX = curX + gx + 1;
                                if (destX >= 0 && destX < boundW) {
                                    int finalX = offsetX + destX;
                                    int finalY = offsetY + destY;
                                    if (finalX >= 0 && finalX < imgWidth && finalY >= 0 && finalY < imgHeight) {
                                        if (glyphAlpha >= 255) {
                                            targetImg.SetPixel(finalX, finalY, color);
                                        } else {
                                            Color bg = targetImg.GetPixel(finalX, finalY);
                                            unsigned char invAlpha = 255 - glyphAlpha;
                                            unsigned char r = static_cast<unsigned char>((static_cast<unsigned char>(color.R()) * glyphAlpha + static_cast<unsigned char>(bg.R()) * invAlpha) / 255);
                                            unsigned char g = static_cast<unsigned char>((static_cast<unsigned char>(color.G()) * glyphAlpha + static_cast<unsigned char>(bg.G()) * invAlpha) / 255);
                                            unsigned char b = static_cast<unsigned char>((static_cast<unsigned char>(color.B()) * glyphAlpha + static_cast<unsigned char>(bg.B()) * invAlpha) / 255);
                                            targetImg.SetPixel(finalX, finalY, Color(r, g, b));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Advance cursor (extra pixel for bold)
            curX += static_cast<int>(font.GetCharWidth(Char(ch)));
            if (isBold) curX += 1;
        }
    }

    // Mark dirty region
    if (_buffer == GraphicsBuffer::GetFrameBuffer()) {
        Drawing::Size textSize = font.MeasureString(text);
        MarkDirty(offsetX + static_cast<int>(x), offsetY + static_cast<int>(y),
                  static_cast<int>(textSize.width), static_cast<int>(textSize.height));
    }
}

void Graphics::DrawString(const String& text, const Font& font, const Color& color,
                          const Rectangle& rect, StringAlignment hAlign, StringAlignment vAlign) {
    if (!font.IsValid()) return;

    Drawing::Size textSize = font.MeasureString(text);
    int textW = static_cast<int>(textSize.width);
    int textH = static_cast<int>(textSize.height);
    int rectX = static_cast<int>(rect.x);
    int rectY = static_cast<int>(rect.y);
    int rectW = static_cast<int>(rect.width);
    int rectH = static_cast<int>(rect.height);

    // Calculate X position based on horizontal alignment
    int x = rectX;
    switch (hAlign) {
        case StringAlignment::Near:
            x = rectX;
            break;
        case StringAlignment::Center:
            x = rectX + (rectW - textW) / 2;
            break;
        case StringAlignment::Far:
            x = rectX + rectW - textW;
            break;
    }

    // Calculate Y position based on vertical alignment
    int y = rectY;
    switch (vAlign) {
        case StringAlignment::Near:
            y = rectY;
            break;
        case StringAlignment::Center:
            y = rectY + (rectH - textH) / 2;
            break;
        case StringAlignment::Far:
            y = rectY + rectH - textH;
            break;
    }

    DrawString(text, font, color, Int32(x), Int32(y));
}

Drawing::Size Graphics::MeasureString(const String& text, const Font& font) const {
    return font.MeasureString(text);
}

Drawing::Size Graphics::MeasureString(const char* text, const Font& font) const {
    return font.MeasureString(text);
}

}} // namespace System::Drawing
