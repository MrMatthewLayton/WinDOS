#ifndef SYSTEM_DRAWING_ENUMS_HPP
#define SYSTEM_DRAWING_ENUMS_HPP

/******************************************************************************/
/*    System::Drawing Enumerations                                            */
/*                                                                            */
/*    Common enumerations used throughout the Drawing namespace.              */
/******************************************************************************/

namespace System::Drawing
{

/******************************************************************************/
/*    System::Drawing::BufferMode                                             */
/******************************************************************************/

/// @brief Specifies the buffering mode for graphics operations.
enum class BufferMode
{
    Single,  ///< Single-buffered (direct to screen)
    Double   ///< Double-buffered (render to back buffer, then flip)
};

/******************************************************************************/
/*    System::Drawing::BorderStyle                                            */
/******************************************************************************/

/// @brief Specifies the style of a control's border.
enum class BorderStyle
{
    None,         ///< No border
    Flat,         ///< Flat single-pixel border
    Raised,       ///< 3D raised border (button-like)
    Sunken,       ///< 3D sunken border (input field-like)
    RaisedDouble, ///< Double 3D raised border
    SunkenDouble, ///< Double 3D sunken border
    Window        ///< Window-style border with title bar area
};

/******************************************************************************/
/*    System::Drawing::FontStyle                                              */
/*                                                                            */
/*    Flags that specify style information applied to text. Can be combined.  */
/******************************************************************************/

/// @brief Specifies style information applied to text.
///
/// FontStyle values can be combined using bitwise OR to apply multiple
/// styles to text.
///
/// @par Example
/// @code
/// FontStyle style = FontStyle::Bold | FontStyle::Italic;
/// Font f = Font::FromFile("ARIAL.FON", 12, style);
/// @endcode
enum class FontStyle : unsigned char
{
    Regular   = 0x00,  ///< Normal text
    Bold      = 0x01,  ///< Bold text
    Italic    = 0x02,  ///< Italic text
    Underline = 0x04,  ///< Underlined text
    Strikeout = 0x08   ///< Strikethrough text
};

/// @brief Bitwise OR operator for combining FontStyle flags.
/// @param a First style flag.
/// @param b Second style flag.
/// @return Combined style flags.
inline FontStyle operator|(FontStyle a, FontStyle b)
{
    return static_cast<FontStyle>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

/// @brief Bitwise AND operator for testing FontStyle flags.
/// @param a First style flag.
/// @param b Second style flag.
/// @return Intersection of style flags.
inline FontStyle operator&(FontStyle a, FontStyle b)
{
    return static_cast<FontStyle>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
}

/// @brief Logical NOT operator for testing if FontStyle is Regular.
/// @param a Style flag to test.
/// @return True if style is Regular (no flags set).
inline bool operator!(FontStyle a)
{
    return static_cast<unsigned char>(a) == 0;
}

/******************************************************************************/
/*    System::Drawing::StringAlignment                                        */
/*                                                                            */
/*    Specifies the alignment of text within a rectangle.                     */
/******************************************************************************/

/// @brief Specifies the alignment of text within a layout rectangle.
enum class StringAlignment : unsigned char
{
    Near,    ///< Left (horizontal) or Top (vertical) aligned
    Center,  ///< Center aligned
    Far      ///< Right (horizontal) or Bottom (vertical) aligned
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_ENUMS_HPP
