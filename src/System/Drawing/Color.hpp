#ifndef SYSTEM_DRAWING_COLOR_HPP
#define SYSTEM_DRAWING_COLOR_HPP

#include "../Types.hpp"

namespace System::Drawing
{
    /// @brief Represents a 32-bit ARGB color.
    ///
    /// All colors are stored internally as 32-bit ARGB format (0xAARRGGBB).
    /// This matches the .NET System.Drawing.Color structure. For low-color
    /// display modes (4bpp/8bpp VGA), colors are dithered at render time using
    /// Bayer dithering.
    class Color
    {
        UInt32 mValue; /// ARGB format: 0xAARRGGBB

    public:

        /// @brief Constructs an opaque black color.
        Color() : mValue(0xFF000000)
        {
        }

        /// @brief Constructs a color from a 32-bit ARGB value.
        /// @param argb The color in ARGB format (0xAARRGGBB).
        explicit Color(const UInt32 argb) : mValue(argb)
        {
        }

        /// @brief Constructs an opaque color from RGB components.
        /// @param red Red component (0-255).
        /// @param green Green component (0-255).
        /// @param blue Blue component (0-255).
        /// @param alpha Alpha component (0-255, where 0=transparent, 255=opaque).
        Color(const UInt8 red, const UInt8 green, const UInt8 blue, const UInt8 alpha = UInt8::MaxValue)
            : mValue(UInt32(alpha) << 24 | UInt32(red) << 16 | UInt32(green) << 8 | UInt32(blue))
        {
        }

        /// @brief Copy constructor.
        /// @param other The color to copy.
        Color(const Color &other) = default;

        /// @brief Assignment operator.
        /// @param other The color to assign.
        /// @return Reference to this color.
        Color &operator=(const Color &other) = default;

        /// @brief Gets the alpha component.
        /// @return Alpha value (0-255, where 0=transparent, 255=opaque).
        [[nodiscard]] UInt8 A() const
        {
            return mValue >> 24 & UInt8::MaxValue;
        }

        /// @brief Gets the red component.
        /// @return Red value (0-255).
        [[nodiscard]] UInt8 R() const
        {
            return mValue >> 16 & UInt8::MaxValue;
        }

        /// @brief Gets the green component.
        /// @return Green value (0-255).
        [[nodiscard]] UInt8 G() const
        {
            return mValue >> 8 & UInt8::MaxValue;
        }

        /// @brief Gets the blue component.
        /// @return Blue value (0-255).
        [[nodiscard]] UInt8 B() const
        {
            return mValue & UInt8::MaxValue;
        }

        /// @brief Converts the color to a 32-bit ARGB value.
        /// @return The color as 0xAARRGGBB.
        [[nodiscard]] UInt32 ToArgb() const
        {
            return mValue;
        }

        /// @brief Implicit conversion to unsigned int for use with raw pixel APIs.
        /// @return The color as 0xAARRGGBB.
        explicit operator unsigned int() const
        {
            return mValue;
        }

        /// @brief Equality comparison.
        /// @param other The color to compare with.
        /// @return True if colors are identical (all components equal).
        Boolean operator==(const Color &other) const
        {
            return mValue == other.mValue;
        }

        /// @brief Inequality comparison.
        /// @param other The color to compare with.
        /// @return True if colors differ in any component.
        Boolean operator!=(const Color &other) const
        {
            return mValue != other.mValue;
        }

        /// @brief Linearly interpolates between two colors.
        /// @param first The start color (t=0).
        /// @param second The end color (t=1).
        /// @param factor Interpolation factor (0.0 to 1.0).
        /// @return The interpolated color.
        static Color Lerp(const Color &first, const Color &second, Float32 factor);

        /// @brief Finds the closest VGA palette index (0-15) for this color using Euclidean distance in RGB space.
        /// @return VGA palette index (0-15).
        [[nodiscard]] UInt8 ToVgaIndex() const;

        /// @brief Finds the closest VGA palette index for given RGB values using Euclidean distance in RGB space.
        /// @param red Red component (0-255).
        /// @param green Green component (0-255).
        /// @param blue Blue component (0-255).
        /// @return VGA palette index (0-15).
        static UInt8 RgbToVgaIndex(UInt8 red, UInt8 green, UInt8 blue);

        /// @brief Builds a remap table from BMP palette to VGA palette.
        /// @param paletteData Pointer to BMP palette entries (4 bytes each: BGRA).
        /// @param paletteCount Number of palette entries to remap.
        /// @param remap Output array of 16 VGA indices.
        ///
        /// Used when loading indexed BMP files to map their palette colors
        /// to the nearest VGA palette colors.
        static void BuildVgaRemap(const UInt8 *paletteData, UInt32 paletteCount, UInt8 remap[16]);

        // Standard colors (all opaque, 32-bit ARGB)
        static const Color Black; ///< Black (0xFF000000)
        static const Color White; ///< White (0xFFFFFFFF)
        static const Color Red; ///< Red (0xFFFF0000)
        static const Color Green; ///< Green (0xFF00FF00)
        static const Color Blue; ///< Blue (0xFF0000FF)
        static const Color Cyan; ///< Cyan (0xFF00FFFF)
        static const Color Magenta; ///< Magenta (0xFFFF00FF)
        static const Color Yellow; ///< Yellow (0xFFFFFF00)
        static const Color Gray; ///< Gray (0xFFC0C0C0)
        static const Color DarkGray; ///< Dark gray (0xFF808080)
        static const Color DarkBlue; ///< Dark blue (0xFF000080)
        static const Color DarkGreen; ///< Dark green (0xFF008000)
        static const Color DarkCyan; ///< Dark cyan (0xFF008080)
        static const Color DarkRed; ///< Dark red (0xFF800000)
        static const Color DarkMagenta; ///< Dark magenta (0xFF800080)
        static const Color DarkYellow; ///< Dark yellow/olive (0xFF808000)
        static const Color Transparent; ///< Fully transparent (0x00000000)
    };
} // namespace System::Drawing

#endif // SYSTEM_DRAWING_COLOR_HPP
