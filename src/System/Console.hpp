#ifndef SYSTEM_CONSOLE_HPP
#define SYSTEM_CONSOLE_HPP

#include "Types.hpp"

namespace System
{

/// @brief Specifies constants that define foreground and background colors for the console.
///
/// The ConsoleColor enum provides 16 standard DOS/Windows console colors,
/// ranging from Black (0) to White (15). These colors can be used with
/// Console::SetForegroundColor() and Console::SetBackgroundColor() to
/// customize console output appearance.
enum class ConsoleColor : unsigned char
{
    Black = 0,        ///< The color black (RGB: 0, 0, 0).
    DarkBlue = 1,     ///< The color dark blue (RGB: 0, 0, 128).
    DarkGreen = 2,    ///< The color dark green (RGB: 0, 128, 0).
    DarkCyan = 3,     ///< The color dark cyan (RGB: 0, 128, 128).
    DarkRed = 4,      ///< The color dark red (RGB: 128, 0, 0).
    DarkMagenta = 5,  ///< The color dark magenta (RGB: 128, 0, 128).
    DarkYellow = 6,   ///< The color dark yellow (brown) (RGB: 128, 128, 0).
    Gray = 7,         ///< The color gray (RGB: 192, 192, 192).
    DarkGray = 8,     ///< The color dark gray (RGB: 128, 128, 128).
    Blue = 9,         ///< The color blue (RGB: 0, 0, 255).
    Green = 10,       ///< The color green (RGB: 0, 255, 0).
    Cyan = 11,        ///< The color cyan (RGB: 0, 255, 255).
    Red = 12,         ///< The color red (RGB: 255, 0, 0).
    Magenta = 13,     ///< The color magenta (RGB: 255, 0, 255).
    Yellow = 14,      ///< The color yellow (RGB: 255, 255, 0).
    White = 15        ///< The color white (RGB: 255, 255, 255).
};

/// @brief Represents the standard input, output, and error streams for console applications.
///
/// The Console class provides static methods for reading from and writing to the console,
/// controlling cursor position, and managing text colors. This class mirrors the .NET
/// System.Console API and provides a familiar interface for console I/O operations in DOS.
///
/// @note All methods are static; the Console class cannot be instantiated.
///
/// Example usage:
/// @code
/// Console::SetForegroundColor(ConsoleColor::Green);
/// Console::WriteLine("Hello, World!");
/// Console::ResetColor();
///
/// Console::Write("Enter your name: ");
/// String name = Console::ReadLine();
/// Console::WriteLine("Welcome, " + name + "!");
/// @endcode
class Console
{
    static ConsoleColor _foregroundColor;
    static ConsoleColor _backgroundColor;

    /// @brief Writes a single character to the console with current color attributes.
    /// @param c The character to write.
    static void _writeChar(Char c);

    /// @brief Handles newline character, scrolling the screen if necessary.
    static void _handleNewline();

    /// @brief Combines foreground and background colors into a single attribute byte.
    /// @return The color attribute byte for VGA text mode.
    static UInt8 _getColorAttribute();

public:
    // ========================================================================
    // Output - Write (no newline)
    // ========================================================================

    /// @brief Writes the specified String value to the standard output stream.
    /// @param value The String to write.
    static void Write(const String& value);

    /// @brief Writes the specified C-string to the standard output stream.
    /// @param value The null-terminated string to write.
    static void Write(const char* value);

    /// @brief Writes the specified character to the standard output stream.
    /// @param value The character to write.
    static void Write(char value);

    /// @brief Writes the text representation of the specified Boolean value to the standard output stream.
    /// @param value The Boolean value to write ("True" or "False").
    static void Write(bool value);

    /// @brief Writes the text representation of the specified 32-bit signed integer to the standard output stream.
    /// @param value The integer value to write.
    static void Write(int value);

    /// @brief Writes the text representation of the specified 32-bit unsigned integer to the standard output stream.
    /// @param value The unsigned integer value to write.
    static void Write(unsigned int value);

    /// @brief Writes the text representation of the specified 64-bit signed integer to the standard output stream.
    /// @param value The 64-bit integer value to write.
    static void Write(long long value);

    /// @brief Writes the text representation of the specified 64-bit unsigned integer to the standard output stream.
    /// @param value The 64-bit unsigned integer value to write.
    static void Write(unsigned long long value);

    /// @brief Writes the text representation of the specified single-precision floating-point value to the standard output stream.
    /// @param value The float value to write.
    static void Write(float value);

    /// @brief Writes the text representation of the specified double-precision floating-point value to the standard output stream.
    /// @param value The double value to write.
    static void Write(double value);

    /// @brief Writes the text representation of the specified Boolean wrapper to the standard output stream.
    /// @param value The Boolean wrapper value to write.
    static void Write(const Boolean& value);

    /// @brief Writes the specified Char wrapper to the standard output stream.
    /// @param value The Char wrapper value to write.
    static void Write(const Char& value);

    /// @brief Writes the text representation of the specified Int8 wrapper to the standard output stream.
    /// @param value The Int8 wrapper value to write.
    static void Write(const Int8& value);

    /// @brief Writes the text representation of the specified UInt8 wrapper to the standard output stream.
    /// @param value The UInt8 wrapper value to write.
    static void Write(const UInt8& value);

    /// @brief Writes the text representation of the specified Int16 wrapper to the standard output stream.
    /// @param value The Int16 wrapper value to write.
    static void Write(const Int16& value);

    /// @brief Writes the text representation of the specified UInt16 wrapper to the standard output stream.
    /// @param value The UInt16 wrapper value to write.
    static void Write(const UInt16& value);

    /// @brief Writes the text representation of the specified Int32 wrapper to the standard output stream.
    /// @param value The Int32 wrapper value to write.
    static void Write(const Int32& value);

    /// @brief Writes the text representation of the specified UInt32 wrapper to the standard output stream.
    /// @param value The UInt32 wrapper value to write.
    static void Write(const UInt32& value);

    /// @brief Writes the text representation of the specified Int64 wrapper to the standard output stream.
    /// @param value The Int64 wrapper value to write.
    static void Write(const Int64& value);

    /// @brief Writes the text representation of the specified UInt64 wrapper to the standard output stream.
    /// @param value The UInt64 wrapper value to write.
    static void Write(const UInt64& value);

    /// @brief Writes the text representation of the specified Float32 wrapper to the standard output stream.
    /// @param value The Float32 wrapper value to write.
    static void Write(const Float32& value);

    /// @brief Writes the text representation of the specified Float64 wrapper to the standard output stream.
    /// @param value The Float64 wrapper value to write.
    static void Write(const Float64& value);

    // ========================================================================
    // Output - WriteLine (with newline)
    // ========================================================================

    /// @brief Writes the current line terminator to the standard output stream.
    static void WriteLine();

    /// @brief Writes the specified String value followed by the current line terminator to the standard output stream.
    /// @param value The String to write.
    static void WriteLine(const String& value);

    /// @brief Writes the specified C-string followed by the current line terminator to the standard output stream.
    /// @param value The null-terminated string to write.
    static void WriteLine(const char* value);

    /// @brief Writes the specified character followed by the current line terminator to the standard output stream.
    /// @param value The character to write.
    static void WriteLine(char value);

    /// @brief Writes the text representation of the specified Boolean value followed by the current line terminator.
    /// @param value The Boolean value to write ("True" or "False").
    static void WriteLine(bool value);

    /// @brief Writes the text representation of the specified 32-bit signed integer followed by the current line terminator.
    /// @param value The integer value to write.
    static void WriteLine(int value);

    /// @brief Writes the text representation of the specified 32-bit unsigned integer followed by the current line terminator.
    /// @param value The unsigned integer value to write.
    static void WriteLine(unsigned int value);

    /// @brief Writes the text representation of the specified 64-bit signed integer followed by the current line terminator.
    /// @param value The 64-bit integer value to write.
    static void WriteLine(long long value);

    /// @brief Writes the text representation of the specified 64-bit unsigned integer followed by the current line terminator.
    /// @param value The 64-bit unsigned integer value to write.
    static void WriteLine(unsigned long long value);

    /// @brief Writes the text representation of the specified single-precision floating-point value followed by the current line terminator.
    /// @param value The float value to write.
    static void WriteLine(float value);

    /// @brief Writes the text representation of the specified double-precision floating-point value followed by the current line terminator.
    /// @param value The double value to write.
    static void WriteLine(double value);

    /// @brief Writes the text representation of the specified Boolean wrapper followed by the current line terminator.
    /// @param value The Boolean wrapper value to write.
    static void WriteLine(const Boolean& value);

    /// @brief Writes the specified Char wrapper followed by the current line terminator to the standard output stream.
    /// @param value The Char wrapper value to write.
    static void WriteLine(const Char& value);

    /// @brief Writes the text representation of the specified Int8 wrapper followed by the current line terminator.
    /// @param value The Int8 wrapper value to write.
    static void WriteLine(const Int8& value);

    /// @brief Writes the text representation of the specified UInt8 wrapper followed by the current line terminator.
    /// @param value The UInt8 wrapper value to write.
    static void WriteLine(const UInt8& value);

    /// @brief Writes the text representation of the specified Int16 wrapper followed by the current line terminator.
    /// @param value The Int16 wrapper value to write.
    static void WriteLine(const Int16& value);

    /// @brief Writes the text representation of the specified UInt16 wrapper followed by the current line terminator.
    /// @param value The UInt16 wrapper value to write.
    static void WriteLine(const UInt16& value);

    /// @brief Writes the text representation of the specified Int32 wrapper followed by the current line terminator.
    /// @param value The Int32 wrapper value to write.
    static void WriteLine(const Int32& value);

    /// @brief Writes the text representation of the specified UInt32 wrapper followed by the current line terminator.
    /// @param value The UInt32 wrapper value to write.
    static void WriteLine(const UInt32& value);

    /// @brief Writes the text representation of the specified Int64 wrapper followed by the current line terminator.
    /// @param value The Int64 wrapper value to write.
    static void WriteLine(const Int64& value);

    /// @brief Writes the text representation of the specified UInt64 wrapper followed by the current line terminator.
    /// @param value The UInt64 wrapper value to write.
    static void WriteLine(const UInt64& value);

    /// @brief Writes the text representation of the specified Float32 wrapper followed by the current line terminator.
    /// @param value The Float32 wrapper value to write.
    static void WriteLine(const Float32& value);

    /// @brief Writes the text representation of the specified Float64 wrapper followed by the current line terminator.
    /// @param value The Float64 wrapper value to write.
    static void WriteLine(const Float64& value);

    // ========================================================================
    // Input
    // ========================================================================

    /// @brief Reads the next line of characters from the standard input stream.
    /// @return A String containing the next line of characters from the input stream, or an empty String if no more lines are available.
    static String ReadLine();

    /// @brief Obtains the next character or function key pressed by the user. The pressed key is displayed in the console window.
    /// @return A Char representing the character corresponding to the pressed console key.
    static Char ReadKey();

    /// @brief Obtains the next character or function key pressed by the user, optionally suppressing display.
    /// @param intercept If true, the pressed key is not displayed in the console window.
    /// @return A Char representing the character corresponding to the pressed console key.
    static Char ReadKey(Boolean intercept);

    /// @brief Gets a value indicating whether a key press is available in the input stream.
    /// @return True if a key press is available; otherwise, false.
    static Boolean KeyAvailable();

    // ========================================================================
    // Cursor Position
    // ========================================================================

    /// @brief Sets the position of the cursor.
    /// @param left The column position of the cursor (0-based, left edge of the buffer).
    /// @param top The row position of the cursor (0-based, top edge of the buffer).
    static void SetCursorPosition(Int32 left, Int32 top);

    /// @brief Gets the column position of the cursor within the buffer area.
    /// @return The current position of the cursor, in columns (0-based).
    static Int32 CursorLeft();

    /// @brief Gets the row position of the cursor within the buffer area.
    /// @return The current position of the cursor, in rows (0-based).
    static Int32 CursorTop();

    // ========================================================================
    // Colors
    // ========================================================================

    /// @brief Gets the foreground color of the console.
    /// @return A ConsoleColor that specifies the foreground color of the console (the color of each character that is displayed).
    static ConsoleColor ForegroundColor();

    /// @brief Sets the foreground color of the console.
    /// @param color A ConsoleColor that specifies the foreground color of the console.
    static void SetForegroundColor(ConsoleColor color);

    /// @brief Gets the background color of the console.
    /// @return A ConsoleColor that specifies the background color of the console (the color behind each character).
    static ConsoleColor BackgroundColor();

    /// @brief Sets the background color of the console.
    /// @param color A ConsoleColor that specifies the background color of the console.
    static void SetBackgroundColor(ConsoleColor color);

    /// @brief Sets the foreground and background console colors to their defaults.
    ///
    /// The default foreground color is Gray, and the default background color is Black.
    static void ResetColor();

    // ========================================================================
    // Screen
    // ========================================================================

    /// @brief Clears the console buffer and corresponding console window of display information.
    ///
    /// The cursor is reset to the top-left corner (0, 0) and the entire screen is filled
    /// with spaces using the current background color.
    static void Clear();

    /// @brief Gets the width of the console window.
    /// @return The width of the console window measured in columns (typically 80 in DOS text mode).
    static Int32 WindowWidth();

    /// @brief Gets the height of the console window.
    /// @return The height of the console window measured in rows (typically 25 in DOS text mode).
    static Int32 WindowHeight();

    // ========================================================================
    // Beep
    // ========================================================================

    /// @brief Plays the sound of a beep through the console speaker.
    ///
    /// By default, the beep plays at a frequency of 800 hertz for a duration of 200 milliseconds.
    static void Beep();
};

} // namespace System

#endif // SYSTEM_CONSOLE_HPP
