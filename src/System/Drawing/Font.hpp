/// @file Font.hpp
/// @brief System.Drawing.Font - Font representation and text rendering support.

#ifndef SYSTEM_DRAWING_FONT_HPP
#define SYSTEM_DRAWING_FONT_HPP

#include "../Types.hpp"
#include "../String.hpp"
#include "Enums.hpp"
#include "Size.hpp"

// Forward declare Image to avoid circular dependency
namespace System::Drawing { class Image; }

namespace System::Drawing
{

/******************************************************************************/
/*    System::Drawing::Font                                                   */
/*                                                                            */
/*    Represents a font for rendering text. Supports Windows FON bitmap fonts.*/
/*    Use Font::FromFile() to load fonts, or Font::SystemFont()/FixedFont()   */
/*    for built-in fonts.                                                     */
/******************************************************************************/

/// @brief Represents a font for rendering text.
///
/// Supports both Windows FON bitmap fonts (NE format) and TrueType fonts (TTF).
/// Bitmap fonts are loaded from .FON files and provide crisp rendering at specific
/// sizes. TrueType fonts are rasterized at load time using stb_truetype.
///
/// @par Example
/// @code
/// // Load a TrueType font
/// Font arial = Font::FromTrueType("ARIAL.TTF", 14);
///
/// // Load a bitmap font
/// Font fixed = Font::FromFile("FIXEDSYS.FON", 8);
///
/// // Use system defaults
/// Font sysFont = Font::SystemFont();
///
/// // Measure text
/// Size textSize = arial.MeasureString("Hello World");
/// @endcode
class Font
{
    /// @brief Forward declaration of internal font data structure.
    struct FontData;
    FontData* _data;  ///< Pointer to internal font data

    /// @brief Private constructor from internal data.
    Font(FontData* data);

public:
    /// @brief Constructs an invalid/empty font.
    Font();

    /// @brief Copy constructor (deep copy of glyph data).
    /// @param other The font to copy.
    Font(const Font& other);

    /// @brief Move constructor.
    /// @param other The font to move from.
    Font(Font&& other) noexcept;

    /// @brief Destructor, frees font data.
    ~Font();

    /// @brief Copy assignment operator.
    /// @param other The font to copy.
    /// @return Reference to this font.
    Font& operator=(const Font& other);

    /// @brief Move assignment operator.
    /// @param other The font to move from.
    /// @return Reference to this font.
    Font& operator=(Font&& other) noexcept;

    /// @brief Loads a bitmap font from a FON file (NE format).
    /// @param path Path to the .FON file.
    /// @param size Desired point size (finds closest match in file).
    /// @param style Font style flags.
    /// @return Loaded font.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid FON.
    static Font FromFile(const char* path, Int32 size, FontStyle style = FontStyle::Regular);

    /// @brief Loads a TrueType font from a TTF file.
    /// @param path Path to the .TTF file.
    /// @param pixelHeight Desired font height in pixels.
    /// @param style Font style flags (Bold causes fake bold effect).
    /// @return Loaded font.
    /// @throws FileNotFoundException if the file does not exist.
    /// @throws InvalidDataException if the file is not a valid TTF.
    ///
    /// Glyphs are rendered using stb_truetype with sharp threshold rendering
    /// for crisp text. Anti-aliasing is not used to prevent blur at small sizes.
    static Font FromTrueType(const char* path, Int32 pixelHeight, FontStyle style = FontStyle::Regular);

    /// @brief Gets the default system font (MS Sans Serif 8pt).
    /// @return System font.
    /// @throws FileNotFoundException if MSSANS.FON is not found.
    static Font SystemFont();

    /// @brief Gets the bold system font (MS Sans Serif 8pt Bold).
    /// @return Bold system font.
    /// @throws FileNotFoundException if MSSANS.FON is not found.
    ///
    /// @note Uses fake bold effect (not a true bold variant).
    static Font SystemFontBold();

    /// @brief Gets the fixed-width system font (Fixedsys 8pt).
    /// @return Fixed-width font.
    /// @throws FileNotFoundException if FIXEDSYS.FON is not found.
    static Font FixedFont();

    /// @brief Gets the font family name.
    /// @return Font name (e.g., "MS Sans Serif", "Arial").
    String Name() const;

    /// @brief Gets the point size.
    /// @return Point size as specified when loading.
    Int32 Size() const;

    /// @brief Gets the line height in pixels.
    /// @return Height from one baseline to the next.
    Int32 Height() const;

    /// @brief Gets the ascent in pixels.
    /// @return Pixels from baseline to top of tallest glyph.
    Int32 Ascent() const;

    /// @brief Gets the font style.
    /// @return FontStyle flags.
    FontStyle Style() const;

    /// @brief Checks if this font is valid and usable.
    /// @return True if font data was loaded successfully.
    Boolean IsValid() const;

    /// @brief Checks if this is a TrueType font.
    /// @return True for TTF fonts, false for bitmap FON fonts.
    Boolean IsTrueType() const;

    /// @brief Gets the internal stb_truetype font info (for TTF fonts only).
    /// @return Pointer to stbtt_fontinfo, or nullptr for non-TTF fonts.
    ///
    /// @warning For internal use only. The returned pointer is owned by the Font.
    void* GetTTFInfo() const;

    /// @brief Gets the TTF scale factor (for TTF fonts only).
    /// @return Scale factor for stbtt functions, or 0 for non-TTF fonts.
    float GetTTFScale() const;

    /// @brief Gets the width of a character in pixels.
    /// @param c Character to measure.
    /// @return Width in pixels.
    Int32 GetCharWidth(Char c) const;

    /// @brief Measures the size of rendered text.
    /// @param text Text to measure.
    /// @return Size in pixels (width, height).
    class Size MeasureString(const String& text) const;

    /// @brief Measures the size of rendered text.
    /// @param text Null-terminated C string to measure.
    /// @return Size in pixels (width, height).
    class Size MeasureString(const char* text) const;

    /// @brief Gets the cached glyph bitmap for a character.
    /// @param c Character to get glyph for.
    /// @return Image containing glyph (white on transparent).
    ///
    /// @warning For internal use by Graphics::DrawString.
    const Image& GetGlyph(Char c) const;
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_FONT_HPP
