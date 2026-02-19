/// @file GraphicsBuffer.cpp
/// @brief Implementation of System.Drawing.GraphicsBuffer class.

#include "GraphicsBuffer.hpp"
#include "Color.hpp"
#include "../IO/Devices/Display.hpp"
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

} // namespace System::Drawing
