#ifndef SYSTEM_DRAWING_SIZE_HPP
#define SYSTEM_DRAWING_SIZE_HPP

#include "../Types.hpp"

/******************************************************************************/
/*    System::Drawing::Size                                                   */
/******************************************************************************/

namespace System::Drawing
{

/// @brief Represents a size with integer width and height.
///
/// @par Example
/// @code
/// Size empty;                      // (0, 0)
/// Size s1(640, 480);               // (640, 480)
///
/// if (s1.IsEmpty()) { ... }        // false
/// if (s1 == Size::IconMedium) { ... }
/// @endcode
class Size
{
public:
    Int32 width;   ///< Width
    Int32 height;  ///< Height

    /// @brief Constructs an empty size (0, 0).
    Size() : width(0), height(0)
    {
    }

    /// @brief Constructs a size with the specified dimensions.
    /// @param width Width in pixels.
    /// @param height Height in pixels.
    Size(Int32 width, Int32 height) : width(width), height(height)
    {
    }

    /// @brief Copy constructor.
    /// @param other The size to copy.
    Size(const Size& other) : width(other.width), height(other.height)
    {
    }

    /// @brief Assignment operator.
    /// @param other The size to assign.
    /// @return Reference to this size.
    Size& operator=(const Size& other)
    {
        width = other.width;
        height = other.height;
        return *this;
    }

    /// @brief Equality comparison.
    /// @param other The size to compare with.
    /// @return True if both dimensions are equal.
    Boolean operator==(const Size& other) const
    {
        return Boolean(static_cast<int>(width) == static_cast<int>(other.width) &&
                       static_cast<int>(height) == static_cast<int>(other.height));
    }

    /// @brief Inequality comparison.
    /// @param other The size to compare with.
    /// @return True if any dimension differs.
    Boolean operator!=(const Size& other) const
    {
        return !(*this == other);
    }

    /// @brief Checks if either dimension is zero.
    /// @return True if width or height is zero.
    Boolean IsEmpty() const
    {
        return Boolean(static_cast<int>(width) == 0 || static_cast<int>(height) == 0);
    }

    static const Size Empty;       ///< Empty size (0, 0)
    static const Size IconSmall;   ///< 16x16 pixels (small icon)
    static const Size IconMedium;  ///< 32x32 pixels (standard icon)
    static const Size IconLarge;   ///< 48x48 pixels (large icon)
    static const Size IconCursor;  ///< 24x24 pixels (cursor)
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_SIZE_HPP
