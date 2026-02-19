#ifndef SYSTEM_DRAWING_HATCHSTYLE_HPP
#define SYSTEM_DRAWING_HATCHSTYLE_HPP

/******************************************************************************/
/*    System::Drawing::HatchStyle                                             */
/*                                                                            */
/*    Defines fill patterns for hatched brushes. Each pattern is an 8x8       */
/*    bitmap where 1 bits are drawn in the foreground color and 0 bits        */
/*    are drawn in the background color.                                      */
/******************************************************************************/

namespace System::Drawing
{

/// @brief Defines 8x8 fill patterns for hatched brushes.
///
/// Each HatchStyle contains an 8x8 bitmap pattern where 1 bits are drawn
/// in the foreground color and 0 bits in the background color. Use with
/// Graphics::FillRectangle() to create patterned fills.
///
/// @par Example
/// @code
/// Graphics g(BufferMode::Double, rect);
/// g.FillRectangle(rect, HatchStyle::DiagonalCross, Color::Black, Color::White);
/// @endcode
class HatchStyle
{
    unsigned char _pattern[8];  ///< 8x8 pattern bitmap (1 row per byte)

    /// @brief Constructs a hatch style from 8 pattern bytes.
    HatchStyle(const unsigned char p0, const unsigned char p1,
               const unsigned char p2, const unsigned char p3,
               const unsigned char p4, const unsigned char p5,
               const unsigned char p6, const unsigned char p7)
    {
        _pattern[0] = p0; _pattern[1] = p1; _pattern[2] = p2; _pattern[3] = p3;
        _pattern[4] = p4; _pattern[5] = p5; _pattern[6] = p6; _pattern[7] = p7;
    }

public:
    /// @brief Gets the pattern bit at a position.
    /// @param x X coordinate (will be wrapped to 0-7).
    /// @param y Y coordinate (will be wrapped to 0-7).
    /// @return True for foreground color, false for background color.
    bool GetBit(int x, int y) const
    {
        return (_pattern[y & 7] >> (7 - (x & 7))) & 1;
    }

    /// @brief Gets direct access to the pattern bytes.
    /// @return Pointer to 8-byte pattern array.
    const unsigned char* Pattern() const
    {
        return _pattern;
    }

    // Solid patterns
    static const HatchStyle Solid;           ///< All foreground (solid fill)
    static const HatchStyle Empty;           ///< All background (empty)

    // Horizontal/Vertical lines
    static const HatchStyle Horizontal;      ///< Horizontal lines
    static const HatchStyle Vertical;        ///< Vertical lines
    static const HatchStyle Cross;           ///< Grid (horizontal + vertical)

    // Diagonal lines (forward = top-left to bottom-right)
    static const HatchStyle ForwardDiagonal; ///< Forward diagonal (///)
    static const HatchStyle BackwardDiagonal;///< Backward diagonal (\\\)
    static const HatchStyle DiagonalCross;   ///< Diagonal cross (X pattern)

    // Dot patterns (percentage indicates foreground coverage)
    static const HatchStyle Percent05;       ///< 5% dots
    static const HatchStyle Percent10;       ///< 10% dots
    static const HatchStyle Percent20;       ///< 20% dots
    static const HatchStyle Percent25;       ///< 25% dots
    static const HatchStyle Percent30;       ///< 30% dots
    static const HatchStyle Percent40;       ///< 40% dots
    static const HatchStyle Percent50;       ///< 50% checkerboard
    static const HatchStyle Percent60;       ///< 60% dots
    static const HatchStyle Percent70;       ///< 70% dots
    static const HatchStyle Percent75;       ///< 75% dots
    static const HatchStyle Percent80;       ///< 80% dots
    static const HatchStyle Percent90;       ///< 90% dots

    // Special patterns
    static const HatchStyle LightHorizontal; ///< Thin horizontal lines
    static const HatchStyle LightVertical;   ///< Thin vertical lines
    static const HatchStyle DarkHorizontal;  ///< Thick horizontal lines
    static const HatchStyle DarkVertical;    ///< Thick vertical lines
    static const HatchStyle DashedHorizontal;///< Dashed horizontal lines
    static const HatchStyle DashedVertical;  ///< Dashed vertical lines
    static const HatchStyle SmallGrid;       ///< Small grid pattern
    static const HatchStyle LargeGrid;       ///< Large grid pattern
    static const HatchStyle DottedGrid;      ///< Dotted grid pattern
    static const HatchStyle DottedDiamond;   ///< Dotted diamond pattern
    static const HatchStyle Brick;           ///< Brick wall pattern
    static const HatchStyle Weave;           ///< Weave pattern
    static const HatchStyle Trellis;         ///< Trellis pattern
    static const HatchStyle Sphere;          ///< Sphere/circle pattern
    static const HatchStyle Wave;            ///< Wave pattern
    static const HatchStyle ZigZag;          ///< Zig-zag pattern
    static const HatchStyle Shingle;         ///< Shingle pattern
    static const HatchStyle Plaid;           ///< Plaid pattern
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_HATCHSTYLE_HPP
