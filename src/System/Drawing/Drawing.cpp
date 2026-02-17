#include "Drawing.hpp"
#include "../Exception.hpp"
#include "../IO/IO.hpp"
#include "../../Platform/DOS/Graphics.hpp"
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

}} // namespace System::Drawing
