#ifndef PLATFORM_DOS_VIDEO_HPP
#define PLATFORM_DOS_VIDEO_HPP

namespace Platform::DOS
{

/// @brief Provides low-level DOS video services via BIOS interrupts.
///
/// The Video class encapsulates INT 10h BIOS calls for text-mode video operations
/// including cursor manipulation, character output, screen scrolling, and mode
/// switching. All methods are static as video hardware is a system-wide resource.
///
/// @note This class operates in text mode only. For graphics modes, use the
///       Graphics class instead.
class Video
{
public:
    /// @brief Sets the text cursor position on screen.
    ///
    /// Moves the blinking text cursor to the specified row and column.
    /// Uses INT 10h AH=02h (Set Cursor Position).
    ///
    /// @param row The row position (0-based, where 0 is the top row).
    /// @param col The column position (0-based, where 0 is the leftmost column).
    static void SetCursorPosition(int row, int col);

    /// @brief Gets the current text cursor position.
    ///
    /// Retrieves the current cursor row and column from the BIOS data area.
    /// Uses INT 10h AH=03h (Get Cursor Position).
    ///
    /// @param[out] row Receives the current row position (0-based).
    /// @param[out] col Receives the current column position (0-based).
    static void GetCursorPosition(int& row, int& col);

    /// @brief Writes a character with a specific attribute at the current cursor position.
    ///
    /// Outputs a character to the screen at the current cursor location with the
    /// specified foreground and background color attribute. The cursor is not
    /// advanced. Uses INT 10h AH=09h (Write Character and Attribute).
    ///
    /// @param c The character to write.
    /// @param attr The color attribute byte (bits 0-3: foreground, bits 4-6: background,
    ///             bit 7: blink if enabled).
    static void WriteChar(char c, unsigned char attr);

    /// @brief Writes a character at the current cursor position using the existing attribute.
    ///
    /// Outputs a character and advances the cursor, using whatever attribute is
    /// already present at that screen location. Uses INT 10h AH=0Eh (Teletype Output).
    ///
    /// @param c The character to write.
    static void WriteCharAtCursor(char c);

    /// @brief Sets the video mode.
    ///
    /// Changes the display to a different video mode. Common modes include:
    /// - 0x03: 80x25 16-color text mode
    /// - 0x12: 640x480 16-color VGA graphics
    /// - 0x13: 320x200 256-color VGA graphics
    ///
    /// Uses INT 10h AH=00h (Set Video Mode).
    ///
    /// @param mode The BIOS video mode number to set.
    static void SetVideoMode(int mode);

    /// @brief Scrolls a rectangular window region up.
    ///
    /// Scrolls the contents of a rectangular screen region upward by the specified
    /// number of lines. New lines at the bottom are filled with spaces using the
    /// given attribute. Uses INT 10h AH=06h (Scroll Up Window).
    ///
    /// @param lines The number of lines to scroll up (0 clears the entire window).
    /// @param attr The attribute to use for newly blank lines.
    /// @param top The top row of the scroll region (0-based).
    /// @param left The left column of the scroll region (0-based).
    /// @param bottom The bottom row of the scroll region (0-based).
    /// @param right The right column of the scroll region (0-based).
    static void ScrollUp(int lines, unsigned char attr, int top, int left, int bottom, int right);

    /// @brief Gets the current screen dimensions in rows and columns.
    ///
    /// Retrieves the number of text rows and columns for the current video mode
    /// from the BIOS data area.
    ///
    /// @param[out] rows Receives the number of screen rows (typically 25, 43, or 50).
    /// @param[out] cols Receives the number of screen columns (typically 80).
    static void GetScreenSize(int& rows, int& cols);

    /// @brief Clears the entire screen with a specified attribute.
    ///
    /// Fills the entire screen with space characters using the given color
    /// attribute, and resets the cursor to the home position (0, 0).
    ///
    /// @param attr The color attribute to fill the screen with.
    static void ClearScreen(unsigned char attr);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_VIDEO_HPP
