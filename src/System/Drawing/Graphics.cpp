/// @file Graphics.cpp
/// @brief Implementation of System.Drawing.Graphics class.

#include "Graphics.hpp"
#include "GraphicsBuffer.hpp"
#include "Image.hpp"
#include "Font.hpp"
#include "../../ThirdParty/stb_truetype.h"
#include <cstdlib>
#include <cstring>

namespace System::Drawing
{

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
