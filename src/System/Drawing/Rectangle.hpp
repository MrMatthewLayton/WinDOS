#ifndef SYSTEM_DRAWING_RECTANGLE_HPP
#define SYSTEM_DRAWING_RECTANGLE_HPP

#include "../Types.hpp"
#include "Point.hpp"
#include "Size.hpp"

/******************************************************************************/
/*    System::Drawing::Rectangle                                              */
/******************************************************************************/

namespace System::Drawing
{

/// @brief Represents a rectangle with integer coordinates and dimensions.
///
/// The rectangle is defined by its top-left corner (x, y) and size (width, height).
/// The right edge is at x + width (exclusive) and bottom edge at y + height (exclusive).
///
/// @par Example
/// @code
/// Rectangle r1(10, 20, 100, 50);   // x=10, y=20, w=100, h=50
/// Rectangle r2(Point(10, 20), Size(100, 50));  // Same rectangle
///
/// if (r1.Contains(50, 30)) { ... } // true
/// Rectangle inflated = r1.Inflate(5, 5);  // x=5, y=15, w=110, h=60
/// @endcode
class Rectangle
{
public:
    Int32 x;       ///< X coordinate of top-left corner
    Int32 y;       ///< Y coordinate of top-left corner
    Int32 width;   ///< Width of rectangle
    Int32 height;  ///< Height of rectangle

    /// @brief Constructs an empty rectangle at the origin.
    Rectangle() : x(0), y(0), width(0), height(0)
    {
    }

    /// @brief Constructs a rectangle with the specified position and size.
    /// @param x X coordinate of top-left corner.
    /// @param y Y coordinate of top-left corner.
    /// @param width Width of rectangle.
    /// @param height Height of rectangle.
    Rectangle(Int32 x, Int32 y, Int32 width, Int32 height)
        : x(x), y(y), width(width), height(height)
    {
    }

    /// @brief Constructs a rectangle from a location point and size.
    /// @param location Top-left corner position.
    /// @param size Width and height.
    Rectangle(const Point& location, const Size& size)
        : x(location.x), y(location.y), width(size.width), height(size.height)
    {
    }

    /// @brief Copy constructor.
    /// @param other The rectangle to copy.
    Rectangle(const Rectangle& other)
        : x(other.x), y(other.y), width(other.width), height(other.height)
    {
    }

    /// @brief Assignment operator.
    /// @param other The rectangle to assign.
    /// @return Reference to this rectangle.
    Rectangle& operator=(const Rectangle& other)
    {
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        return *this;
    }

    /// @brief Gets the top-left corner as a Point.
    /// @return Point containing (x, y).
    Point Location() const
    {
        return Point(x, y);
    }

    /// @brief Gets the dimensions as a Size.
    /// @return Size containing (width, height).
    Size GetSize() const
    {
        return Size(width, height);
    }

    /// @brief Gets the left edge X coordinate.
    /// @return X coordinate of left edge (same as x).
    Int32 Left() const
    {
        return x;
    }

    /// @brief Gets the top edge Y coordinate.
    /// @return Y coordinate of top edge (same as y).
    Int32 Top() const
    {
        return y;
    }

    /// @brief Gets the right edge X coordinate (exclusive).
    /// @return X coordinate of right edge (x + width).
    Int32 Right() const
    {
        return Int32(static_cast<int>(x) + static_cast<int>(width));
    }

    /// @brief Gets the bottom edge Y coordinate (exclusive).
    /// @return Y coordinate of bottom edge (y + height).
    Int32 Bottom() const
    {
        return Int32(static_cast<int>(y) + static_cast<int>(height));
    }

    /// @brief Tests if a point is inside the rectangle.
    /// @param px X coordinate to test.
    /// @param py Y coordinate to test.
    /// @return True if point is inside (inclusive left/top, exclusive right/bottom).
    Boolean Contains(Int32 px, Int32 py) const
    {
        int pxi = static_cast<int>(px);
        int pyi = static_cast<int>(py);
        int xi = static_cast<int>(x);
        int yi = static_cast<int>(y);
        int wi = static_cast<int>(width);
        int hi = static_cast<int>(height);
        return Boolean(pxi >= xi && pxi < xi + wi && pyi >= yi && pyi < yi + hi);
    }

    /// @brief Tests if a point is inside the rectangle.
    /// @param pt Point to test.
    /// @return True if point is inside.
    Boolean Contains(const Point& pt) const
    {
        return Contains(pt.x, pt.y);
    }

    /// @brief Creates a new rectangle offset by the specified amounts.
    /// @param dx Amount to add to X coordinate.
    /// @param dy Amount to add to Y coordinate.
    /// @return New rectangle at offset position.
    Rectangle Offset(Int32 dx, Int32 dy) const
    {
        return Rectangle(Int32(static_cast<int>(x) + static_cast<int>(dx)),
                         Int32(static_cast<int>(y) + static_cast<int>(dy)),
                         width, height);
    }

    /// @brief Creates a new rectangle inflated by the specified amounts.
    /// @param dx Amount to add to each side horizontally.
    /// @param dy Amount to add to each side vertically.
    /// @return New rectangle expanded by (dx, dy) on each side.
    ///
    /// The resulting rectangle is larger by 2*dx horizontally and 2*dy vertically,
    /// with its center at the same position as the original.
    Rectangle Inflate(Int32 dx, Int32 dy) const
    {
        int dxi = static_cast<int>(dx);
        int dyi = static_cast<int>(dy);
        return Rectangle(Int32(static_cast<int>(x) - dxi),
                         Int32(static_cast<int>(y) - dyi),
                         Int32(static_cast<int>(width) + dxi * 2),
                         Int32(static_cast<int>(height) + dyi * 2));
    }

    static const Rectangle Empty;  ///< Empty rectangle at origin with zero size
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_RECTANGLE_HPP
