#ifndef SYSTEM_DRAWING_HPP
#define SYSTEM_DRAWING_HPP

#include "../Types.hpp"
#include "../Array.hpp"
#include "../Exception.hpp"

namespace System { namespace Drawing {

/******************************************************************************/
/*    BMP File Structures (packed for binary compatibility)                   */
/******************************************************************************/

#pragma pack(push, 1)

struct BitmapFileHeader {
    unsigned short type;        // 'BM' = 0x4D42
    unsigned int   size;        // File size
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int   offset;      // Offset to pixel data

    UInt16 Type() const { return UInt16(type); }
    UInt32 Size() const { return UInt32(size); }
    UInt32 Offset() const { return UInt32(offset); }
};

struct BitmapInfoHeader {
    unsigned int   headerSize;
    int            width;
    int            height;
    unsigned short planes;
    unsigned short bitCount;
    unsigned int   compression;
    unsigned int   imageSize;
    int            xPixelsPerMeter;
    int            yPixelsPerMeter;
    unsigned int   usedColors;
    unsigned int   importantColors;

    UInt32 HeaderSize() const { return UInt32(headerSize); }
    Int32  Width() const { return Int32(width); }
    Int32  Height() const { return Int32(height); }
    UInt16 Planes() const { return UInt16(planes); }
    UInt16 BitCount() const { return UInt16(bitCount); }
    UInt32 Compression() const { return UInt32(compression); }
    UInt32 ImageSize() const { return UInt32(imageSize); }
    UInt32 UsedColors() const { return UInt32(usedColors); }
};

#pragma pack(pop)

// Forward declarations
class Graphics;
class GraphicsBuffer;
class Image;

/******************************************************************************/
/*    System::Drawing::Color                                                  */
/*                                                                            */
/*    Unified 32-bit ARGB color class. All colors are represented internally  */
/*    as 32-bit ARGB (0xAARRGGBB). For low-color display modes (4bpp/8bpp),   */
/*    colors are dithered at render time.                                     */
/******************************************************************************/

class Color {
private:
    unsigned int _value;  // ARGB format: 0xAARRGGBB

public:
    // Constructors
    Color() : _value(0xFF000000) {}  // Opaque black
    Color(unsigned int argb) : _value(argb) {}
    Color(UInt8 r, UInt8 g, UInt8 b)
        : _value(0xFF000000 |
                 (static_cast<unsigned int>(static_cast<unsigned char>(r)) << 16) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(g)) << 8) |
                 static_cast<unsigned int>(static_cast<unsigned char>(b))) {}
    Color(UInt8 a, UInt8 r, UInt8 g, UInt8 b)
        : _value((static_cast<unsigned int>(static_cast<unsigned char>(a)) << 24) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(r)) << 16) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(g)) << 8) |
                 static_cast<unsigned int>(static_cast<unsigned char>(b))) {}
    Color(const Color& other) : _value(other._value) {}

    Color& operator=(const Color& other) {
        _value = other._value;
        return *this;
    }

    // Component accessors
    UInt8 A() const { return UInt8((_value >> 24) & 0xFF); }
    UInt8 R() const { return UInt8((_value >> 16) & 0xFF); }
    UInt8 G() const { return UInt8((_value >> 8) & 0xFF); }
    UInt8 B() const { return UInt8(_value & 0xFF); }

    UInt32 ToArgb() const { return UInt32(_value); }
    operator unsigned int() const { return _value; }

    Boolean operator==(const Color& other) const { return Boolean(_value == other._value); }
    Boolean operator!=(const Color& other) const { return Boolean(_value != other._value); }

    // Linear interpolation between two colors
    static Color Lerp(const Color& c1, const Color& c2, float t);

    // Find closest VGA palette index (0-15) for this color
    UInt8 ToVgaIndex() const;

    // Find closest VGA palette index for given RGB values
    static UInt8 RgbToVgaIndex(UInt8 r, UInt8 g, UInt8 b);

    // Build a remap table from BMP palette to VGA palette
    static void BuildVgaRemap(const unsigned char* paletteData, UInt32 paletteCount, unsigned char remap[16]);

    // Standard colors (all opaque, 32-bit ARGB)
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Yellow;
    static const Color Gray;
    static const Color DarkGray;
    static const Color DarkBlue;
    static const Color DarkGreen;
    static const Color DarkCyan;
    static const Color DarkRed;
    static const Color DarkMagenta;
    static const Color DarkYellow;
    static const Color Transparent;
};

// Backwards compatibility alias (deprecated - will be removed)
typedef Color Color32;

/******************************************************************************/
/*    System::Drawing::Point                                                  */
/******************************************************************************/

class Point {
public:
    Int32 x;
    Int32 y;

    Point() : x(0), y(0) {}
    Point(Int32 x, Int32 y) : x(x), y(y) {}
    Point(const Point& other) : x(other.x), y(other.y) {}

    Point& operator=(const Point& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    Boolean operator==(const Point& other) const {
        return Boolean(static_cast<int>(x) == static_cast<int>(other.x) &&
                       static_cast<int>(y) == static_cast<int>(other.y));
    }

    Boolean operator!=(const Point& other) const {
        return !(*this == other);
    }

    Point Offset(Int32 dx, Int32 dy) const {
        return Point(Int32(static_cast<int>(x) + static_cast<int>(dx)),
                     Int32(static_cast<int>(y) + static_cast<int>(dy)));
    }

    static const Point Empty;
};

/******************************************************************************/
/*    System::Drawing::Size                                                   */
/******************************************************************************/

class Size {
public:
    Int32 width;
    Int32 height;

    Size() : width(0), height(0) {}
    Size(Int32 width, Int32 height) : width(width), height(height) {}
    Size(const Size& other) : width(other.width), height(other.height) {}

    Size& operator=(const Size& other) {
        width = other.width;
        height = other.height;
        return *this;
    }

    Boolean operator==(const Size& other) const {
        return Boolean(static_cast<int>(width) == static_cast<int>(other.width) &&
                       static_cast<int>(height) == static_cast<int>(other.height));
    }

    Boolean operator!=(const Size& other) const {
        return !(*this == other);
    }

    Boolean IsEmpty() const {
        return Boolean(static_cast<int>(width) == 0 || static_cast<int>(height) == 0);
    }

    static const Size Empty;

    // Standard icon sizes
    static const Size IconSmall;   // 16x16
    static const Size IconMedium;  // 32x32
    static const Size IconLarge;   // 48x48
    static const Size IconCursor;  // 24x24 (for cursors)
};

/******************************************************************************/
/*    System::Drawing::Rectangle                                              */
/******************************************************************************/

class Rectangle {
public:
    Int32 x;
    Int32 y;
    Int32 width;
    Int32 height;

    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(Int32 x, Int32 y, Int32 width, Int32 height)
        : x(x), y(y), width(width), height(height) {}
    Rectangle(const Point& location, const Size& size)
        : x(location.x), y(location.y), width(size.width), height(size.height) {}
    Rectangle(const Rectangle& other)
        : x(other.x), y(other.y), width(other.width), height(other.height) {}

    Rectangle& operator=(const Rectangle& other) {
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        return *this;
    }

    Point Location() const { return Point(x, y); }
    Size GetSize() const { return Size(width, height); }

    Int32 Left() const { return x; }
    Int32 Top() const { return y; }
    Int32 Right() const { return Int32(static_cast<int>(x) + static_cast<int>(width)); }
    Int32 Bottom() const { return Int32(static_cast<int>(y) + static_cast<int>(height)); }

    Boolean Contains(Int32 px, Int32 py) const {
        int pxi = static_cast<int>(px);
        int pyi = static_cast<int>(py);
        int xi = static_cast<int>(x);
        int yi = static_cast<int>(y);
        int wi = static_cast<int>(width);
        int hi = static_cast<int>(height);
        return Boolean(pxi >= xi && pxi < xi + wi && pyi >= yi && pyi < yi + hi);
    }

    Boolean Contains(const Point& pt) const {
        return Contains(pt.x, pt.y);
    }

    Rectangle Offset(Int32 dx, Int32 dy) const {
        return Rectangle(Int32(static_cast<int>(x) + static_cast<int>(dx)),
                         Int32(static_cast<int>(y) + static_cast<int>(dy)),
                         width, height);
    }

    Rectangle Inflate(Int32 dx, Int32 dy) const {
        int dxi = static_cast<int>(dx);
        int dyi = static_cast<int>(dy);
        return Rectangle(Int32(static_cast<int>(x) - dxi),
                         Int32(static_cast<int>(y) - dyi),
                         Int32(static_cast<int>(width) + dxi * 2),
                         Int32(static_cast<int>(height) + dyi * 2));
    }

    static const Rectangle Empty;
};

/******************************************************************************/
/*    System::Drawing::Image                                                  */
/*                                                                            */
/*    Unified 32-bit image class. All pixels are stored as 32-bit ARGB.       */
/*    For low-color display modes, images are dithered at render time.        */
/******************************************************************************/

class Image {
private:
    unsigned int* _data;  // ARGB pixels (4 bytes per pixel)
    int _width;
    int _height;

    void _allocate(int w, int h, unsigned int fill);
    void _free();
    void _copy(const Image& other);

public:
    Image();
    Image(Int32 width, Int32 height, const Color& fillColor = Color::Black);
    Image(const Size& size, const Color& fillColor = Color::Black);
    Image(const Image& other);
    Image(Image&& other) noexcept;
    ~Image();

    Image& operator=(const Image& other);
    Image& operator=(Image&& other) noexcept;

    Int32 Width() const { return Int32(_width); }
    Int32 Height() const { return Int32(_height); }
    Size GetSize() const { return Size(Int32(_width), Int32(_height)); }
    Int32 Length() const { return Int32(_width * _height); }
    Int32 ByteLength() const { return Int32(_width * _height * 4); }

    unsigned int* Data() { return _data; }
    const unsigned int* Data() const { return _data; }

    Color GetPixel(Int32 x, Int32 y) const;
    void SetPixel(Int32 x, Int32 y, const Color& color);
    void SetPixel(const Point& pt, const Color& color);

    void Clear(const Color& color);
    void CopyFrom(const Image& src, Int32 destX, Int32 destY);
    void CopyFrom(const Image& src, const Point& dest);
    // Copy with transparency - pixels with alpha < 128 are not copied
    void CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY);
    Image GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const;
    Image GetRegion(const Rectangle& rect) const;

    // Load a BMP file from disk (supports 4bpp, 8bpp, 24bpp, 32bpp)
    static Image FromBitmap(const char* path);

    // Load an icon from a standalone .ico file
    // Supported sizes: 16x16, 32x32, 48x48 (use Size::IconSmall/Medium/Large)
    static Image FromIcon(const char* path, const Size& size);

    // Load an icon from a PE-based icon library (.icl, .dll, .exe)
    // iconIndex: 0-based index of the icon group in the library
    static Image FromIconLibrary(const char* path, Int32 iconIndex, const Size& size);

    // Get the number of icon groups in a PE-based icon library
    static Int32 GetIconLibraryCount(const char* path);
};

// Backwards compatibility alias (deprecated - will be removed)
typedef Image Image32;

/******************************************************************************/
/*    System::Drawing::BufferMode                                             */
/******************************************************************************/

enum class BufferMode {
    Single,
    Double
};

/******************************************************************************/
/*    System::Drawing::HatchStyle                                             */
/*                                                                            */
/*    Defines fill patterns for hatched brushes. Each pattern is an 8x8       */
/*    bitmap where 1 bits are drawn in the foreground color and 0 bits        */
/*    are drawn in the background color.                                      */
/******************************************************************************/

class HatchStyle {
private:
    unsigned char _pattern[8];

    HatchStyle(const unsigned char p0, const unsigned char p1,
               const unsigned char p2, const unsigned char p3,
               const unsigned char p4, const unsigned char p5,
               const unsigned char p6, const unsigned char p7) {
        _pattern[0] = p0; _pattern[1] = p1; _pattern[2] = p2; _pattern[3] = p3;
        _pattern[4] = p4; _pattern[5] = p5; _pattern[6] = p6; _pattern[7] = p7;
    }

public:
    // Get bit at position (x % 8, y % 8) - returns true for foreground
    bool GetBit(int x, int y) const {
        return (_pattern[y & 7] >> (7 - (x & 7))) & 1;
    }

    // Access pattern bytes directly
    const unsigned char* Pattern() const { return _pattern; }

    // Solid patterns
    static const HatchStyle Solid;           // All foreground
    static const HatchStyle Empty;           // All background

    // Horizontal/Vertical lines
    static const HatchStyle Horizontal;      // Horizontal lines
    static const HatchStyle Vertical;        // Vertical lines
    static const HatchStyle Cross;           // Grid (horizontal + vertical)

    // Diagonal lines (forward = top-left to bottom-right)
    static const HatchStyle ForwardDiagonal; // /// pattern
    static const HatchStyle BackwardDiagonal;// \\\ pattern
    static const HatchStyle DiagonalCross;   // X pattern

    // Dot patterns
    static const HatchStyle Percent05;       // 5% dots
    static const HatchStyle Percent10;       // 10% dots
    static const HatchStyle Percent20;       // 20% dots
    static const HatchStyle Percent25;       // 25% dots
    static const HatchStyle Percent30;       // 30% dots
    static const HatchStyle Percent40;       // 40% dots
    static const HatchStyle Percent50;       // 50% checkerboard
    static const HatchStyle Percent60;       // 60% dots
    static const HatchStyle Percent70;       // 70% dots
    static const HatchStyle Percent75;       // 75% dots
    static const HatchStyle Percent80;       // 80% dots
    static const HatchStyle Percent90;       // 90% dots

    // Special patterns
    static const HatchStyle LightHorizontal; // Thin horizontal lines
    static const HatchStyle LightVertical;   // Thin vertical lines
    static const HatchStyle DarkHorizontal;  // Thick horizontal lines
    static const HatchStyle DarkVertical;    // Thick vertical lines
    static const HatchStyle DashedHorizontal;// Dashed horizontal
    static const HatchStyle DashedVertical;  // Dashed vertical
    static const HatchStyle SmallGrid;       // Small grid
    static const HatchStyle LargeGrid;       // Large grid
    static const HatchStyle DottedGrid;      // Dotted grid
    static const HatchStyle DottedDiamond;   // Dotted diamond
    static const HatchStyle Brick;           // Brick pattern
    static const HatchStyle Weave;           // Weave pattern
    static const HatchStyle Trellis;         // Trellis pattern
    static const HatchStyle Sphere;          // Sphere/circle pattern
    static const HatchStyle Wave;            // Wave pattern
    static const HatchStyle ZigZag;          // Zig-zag pattern
    static const HatchStyle Shingle;         // Shingle pattern
    static const HatchStyle Plaid;           // Plaid pattern
};

/******************************************************************************/
/*    System::Drawing::BorderStyle                                            */
/******************************************************************************/

enum class BorderStyle {
    None,
    Flat,
    Raised,
    Sunken,
    RaisedDouble,
    SunkenDouble,
    Window
};

/******************************************************************************/
/*    System::Drawing::BufferWriter (function pointer type)                   */
/******************************************************************************/

typedef void (*BufferWriter)(const GraphicsBuffer& buffer);

/******************************************************************************/
/*    System::Drawing::GraphicsBuffer                                         */
/*                                                                            */
/*    Unified graphics buffer. All rendering is done to a 32-bit Image.       */
/*    For low-color display modes, content is dithered when flushed.          */
/******************************************************************************/

class GraphicsBuffer {
private:
    BufferWriter _writer;
    Rectangle _bounds;
    Image _image;            // 32-bit ARGB image (unified)
    unsigned int _lfbPitch;  // Bytes per scanline for LFB
    unsigned char _bpp;      // Display bits per pixel (4, 8, 24, or 32)
    unsigned char _videoMode;// VGA mode (0x12, 0x13) or 0 for VBE

    static GraphicsBuffer* _frameBuffer;
    static void* _lfbAddress;     // Mapped linear framebuffer address
    static unsigned int _lfbSize; // Size of mapped LFB

    GraphicsBuffer(BufferWriter writer, const Rectangle& bounds, unsigned char bpp, unsigned char videoMode);

public:
    ~GraphicsBuffer();

    const Rectangle& Bounds() const { return _bounds; }
    Image& GetImage() { return _image; }
    const Image& GetImage() const { return _image; }
    UInt32 LfbPitch() const { return UInt32(_lfbPitch); }
    UInt8 Bpp() const { return UInt8(_bpp); }
    UInt8 VideoMode() const { return UInt8(_videoMode); }
    Boolean IsVbeMode() const { return Boolean(_videoMode == 0); }

    void Invalidate();

    // Create framebuffer for VGA modes (4bpp planar or 8bpp linear)
    static void CreateFrameBuffer(Int32 width, Int32 height, UInt8 videoMode);
    // Create framebuffer for VBE modes (24bpp or 32bpp LFB)
    static void CreateFrameBuffer32(Int32 width, Int32 height, UInt16 vbeMode,
                                    void* lfbAddr, UInt32 pitch, UInt8 bpp);
    static void DestroyFrameBuffer();
    static void FlushFrameBuffer();
    static GraphicsBuffer* GetFrameBuffer();
    static GraphicsBuffer* Create(BufferMode mode, const Rectangle& bounds);
    static void* GetLFBAddress() { return _lfbAddress; }
};

/******************************************************************************/
/*    System::Drawing::Graphics                                               */
/*                                                                            */
/*    Graphics drawing context. All drawing operations use 32-bit colors.     */
/******************************************************************************/

class Graphics {
private:
    GraphicsBuffer* _buffer;
    Rectangle _bounds;
    bool _ownsBuffer;  // Internal flag stays primitive

public:
    Graphics(BufferMode mode, const Rectangle& bounds);
    Graphics(BufferMode mode, Int32 x, Int32 y, Int32 width, Int32 height);
    ~Graphics();

    const Rectangle& Bounds() const { return _bounds; }

    void Clear(const Color& color);

    void DrawPixel(Int32 x, Int32 y, const Color& color);
    void DrawPixel(const Point& pt, const Color& color);

    void DrawLine(Int32 x1, Int32 y1, Int32 x2, Int32 y2, const Color& color);
    void DrawLine(const Point& p1, const Point& p2, const Color& color);

    void DrawRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color);
    void DrawRectangle(const Rectangle& rect, const Color& color);

    void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height, const Color& color);
    void FillRectangle(const Rectangle& rect, const Color& color);
    void FillRectangle(const Rectangle& rect, BorderStyle style);

    // Hatch pattern fills
    void FillRectangle(Int32 x, Int32 y, Int32 width, Int32 height,
                       const HatchStyle& hatch, const Color& foreColor, const Color& backColor);
    void FillRectangle(const Rectangle& rect,
                       const HatchStyle& hatch, const Color& foreColor, const Color& backColor);

    void DrawImage(const Image& image, Int32 x, Int32 y);
    void DrawImage(const Image& image, const Point& location);

    void Invalidate(Boolean flushFrameBuffer = Boolean(false));
};

}} // namespace System::Drawing

#endif // SYSTEM_DRAWING_HPP
