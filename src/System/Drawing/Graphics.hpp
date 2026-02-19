/// @file Graphics.hpp
/// @brief System.Drawing.Graphics - Drawing context for rendering primitives, text, and images.

#ifndef SYSTEM_DRAWING_GRAPHICS_HPP
#define SYSTEM_DRAWING_GRAPHICS_HPP

#include "../Types.hpp"
#include "../String.hpp"
#include "Enums.hpp"
#include "Color.hpp"
#include "Point.hpp"
#include "Size.hpp"
#include "Rectangle.hpp"
#include "HatchStyle.hpp"

// Forward declarations to avoid circular dependencies
namespace System::Drawing
{
    class Image;
    class Font;
    class GraphicsBuffer;
}

namespace System::Drawing
{

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

#endif // SYSTEM_DRAWING_GRAPHICS_HPP
