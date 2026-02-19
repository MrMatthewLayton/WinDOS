#ifndef SYSTEM_DRAWING_COLOR_HPP
#define SYSTEM_DRAWING_COLOR_HPP

#include "../Types.hpp"

/******************************************************************************/
/*    System::Drawing::Color                                                  */
/*                                                                            */
/*    Unified 32-bit ARGB color class. All colors are represented internally  */
/*    as 32-bit ARGB (0xAARRGGBB). For low-color display modes (4bpp/8bpp),   */
/*    colors are dithered at render time.                                     */
/******************************************************************************/

namespace System::Drawing
{

/// @brief Represents a 32-bit ARGB color.
///
/// All colors are stored internally as 32-bit ARGB format (0xAARRGGBB).
/// This matches the .NET System.Drawing.Color structure. For low-color
/// display modes (4bpp/8bpp VGA), colors are dithered at render time using
/// Bayer dithering.
///
/// @par Example
/// @code
/// Color red(255, 0, 0);           // Opaque red from RGB
/// Color blue = Color::Blue;        // Predefined constant
/// Color semi = Color(0x80FF0000);  // 50% transparent red from ARGB
///
/// // Access components
/// UInt8 r = red.R();  // 255
/// UInt8 a = red.A();  // 255 (opaque)
///
/// // Linear interpolation for gradients
/// Color mid = Color::Lerp(Color::Red, Color::Blue, 0.5f);
/// @endcode
class Color
{
    unsigned int _value;  ///< ARGB format: 0xAARRGGBB

public:
    /// @brief Constructs an opaque black color.
    Color() : _value(0xFF000000)
    {
    }

    /// @brief Constructs a color from a 32-bit ARGB value.
    /// @param argb The color in ARGB format (0xAARRGGBB).
    explicit Color(unsigned int argb) : _value(argb)
    {
    }

    /// @brief Constructs an opaque color from RGB components.
    /// @param r Red component (0-255).
    /// @param g Green component (0-255).
    /// @param b Blue component (0-255).
    Color(UInt8 r, UInt8 g, UInt8 b)
        : _value(0xFF000000 |
                 (static_cast<unsigned int>(static_cast<unsigned char>(r)) << 16) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(g)) << 8) |
                 static_cast<unsigned int>(static_cast<unsigned char>(b)))
    {
    }

    /// @brief Constructs a color from ARGB components.
    /// @param a Alpha component (0-255, where 0=transparent, 255=opaque).
    /// @param r Red component (0-255).
    /// @param g Green component (0-255).
    /// @param b Blue component (0-255).
    Color(UInt8 a, UInt8 r, UInt8 g, UInt8 b)
        : _value((static_cast<unsigned int>(static_cast<unsigned char>(a)) << 24) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(r)) << 16) |
                 (static_cast<unsigned int>(static_cast<unsigned char>(g)) << 8) |
                 static_cast<unsigned int>(static_cast<unsigned char>(b)))
    {
    }

    /// @brief Copy constructor.
    /// @param other The color to copy.
    Color(const Color& other) : _value(other._value)
    {
    }

    /// @brief Assignment operator.
    /// @param other The color to assign.
    /// @return Reference to this color.
    Color& operator=(const Color& other)
    {
        _value = other._value;
        return *this;
    }

    /// @brief Gets the alpha component.
    /// @return Alpha value (0-255, where 0=transparent, 255=opaque).
    UInt8 A() const
    {
        return UInt8((_value >> 24) & 0xFF);
    }

    /// @brief Gets the red component.
    /// @return Red value (0-255).
    UInt8 R() const
    {
        return UInt8((_value >> 16) & 0xFF);
    }

    /// @brief Gets the green component.
    /// @return Green value (0-255).
    UInt8 G() const
    {
        return UInt8((_value >> 8) & 0xFF);
    }

    /// @brief Gets the blue component.
    /// @return Blue value (0-255).
    UInt8 B() const
    {
        return UInt8(_value & 0xFF);
    }

    /// @brief Converts the color to a 32-bit ARGB value.
    /// @return The color as 0xAARRGGBB.
    UInt32 ToArgb() const
    {
        return UInt32(_value);
    }

    /// @brief Implicit conversion to unsigned int for use with raw pixel APIs.
    /// @return The color as 0xAARRGGBB.
    operator unsigned int() const
    {
        return _value;
    }

    /// @brief Equality comparison.
    /// @param other The color to compare with.
    /// @return True if colors are identical (all components equal).
    Boolean operator==(const Color& other) const
    {
        return Boolean(_value == other._value);
    }

    /// @brief Inequality comparison.
    /// @param other The color to compare with.
    /// @return True if colors differ in any component.
    Boolean operator!=(const Color& other) const
    {
        return Boolean(_value != other._value);
    }

    /// @brief Linearly interpolates between two colors.
    /// @param c1 The start color (t=0).
    /// @param c2 The end color (t=1).
    /// @param t Interpolation factor (0.0 to 1.0).
    /// @return The interpolated color.
    ///
    /// @par Example
    /// @code
    /// // Create a gradient from red to blue
    /// for (int i = 0; i < 100; i++) {
    ///     Color c = Color::Lerp(Color::Red, Color::Blue, i / 99.0f);
    ///     // Use color c for gradient position i
    /// }
    /// @endcode
    static Color Lerp(const Color& c1, const Color& c2, Float32 t);

    /// @brief Finds the closest VGA palette index (0-15) for this color.
    /// @return VGA palette index (0-15).
    ///
    /// Uses Euclidean distance in RGB space to find the closest match
    /// in the standard 16-color VGA palette.
    UInt8 ToVgaIndex() const;

    /// @brief Finds the closest VGA palette index for given RGB values.
    /// @param r Red component (0-255).
    /// @param g Green component (0-255).
    /// @param b Blue component (0-255).
    /// @return VGA palette index (0-15).
    static UInt8 RgbToVgaIndex(UInt8 r, UInt8 g, UInt8 b);

    /// @brief Builds a remap table from BMP palette to VGA palette.
    /// @param paletteData Pointer to BMP palette entries (4 bytes each: BGRA).
    /// @param paletteCount Number of palette entries to remap.
    /// @param remap Output array of 16 VGA indices.
    ///
    /// Used when loading indexed BMP files to map their palette colors
    /// to the nearest VGA palette colors.
    static void BuildVgaRemap(const unsigned char* paletteData, UInt32 paletteCount, unsigned char remap[16]);

    // Standard colors (all opaque, 32-bit ARGB)
    static const Color Black;       ///< Black (0xFF000000)
    static const Color White;       ///< White (0xFFFFFFFF)
    static const Color Red;         ///< Red (0xFFFF0000)
    static const Color Green;       ///< Green (0xFF00FF00)
    static const Color Blue;        ///< Blue (0xFF0000FF)
    static const Color Cyan;        ///< Cyan (0xFF00FFFF)
    static const Color Magenta;     ///< Magenta (0xFFFF00FF)
    static const Color Yellow;      ///< Yellow (0xFFFFFF00)
    static const Color Gray;        ///< Gray (0xFFC0C0C0)
    static const Color DarkGray;    ///< Dark gray (0xFF808080)
    static const Color DarkBlue;    ///< Dark blue (0xFF000080)
    static const Color DarkGreen;   ///< Dark green (0xFF008000)
    static const Color DarkCyan;    ///< Dark cyan (0xFF008080)
    static const Color DarkRed;     ///< Dark red (0xFF800000)
    static const Color DarkMagenta; ///< Dark magenta (0xFF800080)
    static const Color DarkYellow;  ///< Dark yellow/olive (0xFF808000)
    static const Color Transparent; ///< Fully transparent (0x00000000)
};

/// @brief Backwards compatibility alias for Color.
/// @deprecated Use Color instead. Will be removed in a future version.
typedef Color Color32;

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_COLOR_HPP
