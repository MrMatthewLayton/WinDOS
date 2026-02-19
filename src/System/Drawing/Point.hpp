#ifndef SYSTEM_DRAWING_POINT_HPP
#define SYSTEM_DRAWING_POINT_HPP

#include "../Types.hpp"

/******************************************************************************/
/*    System::Drawing::Point                                                  */
/******************************************************************************/

namespace System::Drawing
{

/// @brief Represents a point in 2D space with integer coordinates.
///
/// @par Example
/// @code
/// Point origin;                    // (0, 0)
/// Point p1(100, 200);              // (100, 200)
/// Point p2 = p1.Offset(10, -5);    // (110, 195)
///
/// if (p1 == p2) { ... }
/// @endcode
class Point
{
public:
    Int32 x;  ///< X coordinate
    Int32 y;  ///< Y coordinate

    /// @brief Constructs a point at the origin (0, 0).
    Point() : x(0), y(0)
    {
    }

    /// @brief Constructs a point with the specified coordinates.
    /// @param x X coordinate.
    /// @param y Y coordinate.
    Point(Int32 x, Int32 y) : x(x), y(y)
    {
    }

    /// @brief Copy constructor.
    /// @param other The point to copy.
    Point(const Point& other) : x(other.x), y(other.y)
    {
    }

    /// @brief Assignment operator.
    /// @param other The point to assign.
    /// @return Reference to this point.
    Point& operator=(const Point& other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    /// @brief Equality comparison.
    /// @param other The point to compare with.
    /// @return True if both coordinates are equal.
    Boolean operator==(const Point& other) const
    {
        return Boolean(static_cast<int>(x) == static_cast<int>(other.x) &&
                       static_cast<int>(y) == static_cast<int>(other.y));
    }

    /// @brief Inequality comparison.
    /// @param other The point to compare with.
    /// @return True if any coordinate differs.
    Boolean operator!=(const Point& other) const
    {
        return !(*this == other);
    }

    /// @brief Creates a new point offset by the specified amounts.
    /// @param dx Amount to add to X coordinate.
    /// @param dy Amount to add to Y coordinate.
    /// @return New point at offset position.
    Point Offset(Int32 dx, Int32 dy) const
    {
        return Point(Int32(static_cast<int>(x) + static_cast<int>(dx)),
                     Int32(static_cast<int>(y) + static_cast<int>(dy)));
    }

    static const Point Empty;  ///< Empty point at origin (0, 0)
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_POINT_HPP
