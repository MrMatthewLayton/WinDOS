#ifndef SYSTEM_CONSOLE_HPP
#define SYSTEM_CONSOLE_HPP

#include "Types.hpp"

namespace System
{

enum class ConsoleColor : unsigned char
{
    Black = 0,
    DarkBlue = 1,
    DarkGreen = 2,
    DarkCyan = 3,
    DarkRed = 4,
    DarkMagenta = 5,
    DarkYellow = 6,
    Gray = 7,
    DarkGray = 8,
    Blue = 9,
    Green = 10,
    Cyan = 11,
    Red = 12,
    Magenta = 13,
    Yellow = 14,
    White = 15
};

class Console
{
private:
    static ConsoleColor _foregroundColor;
    static ConsoleColor _backgroundColor;

    // Internal helper to write a single character with current colors
    static void _writeChar(Char c);

    // Internal helper to handle newline (scrolling if needed)
    static void _handleNewline();

    // Get color attribute byte from foreground and background
    static UInt8 _getColorAttribute();

public:
    // ========================================================================
    // Output - Write (no newline)
    // ========================================================================

    static void Write(const String& value);
    static void Write(const char* value);
    static void Write(char value);
    static void Write(bool value);
    static void Write(int value);
    static void Write(unsigned int value);
    static void Write(long long value);
    static void Write(unsigned long long value);
    static void Write(float value);
    static void Write(double value);

    // Wrapper type overloads
    static void Write(const Boolean& value);
    static void Write(const Char& value);
    static void Write(const Int8& value);
    static void Write(const UInt8& value);
    static void Write(const Int16& value);
    static void Write(const UInt16& value);
    static void Write(const Int32& value);
    static void Write(const UInt32& value);
    static void Write(const Int64& value);
    static void Write(const UInt64& value);
    static void Write(const Float32& value);
    static void Write(const Float64& value);

    // ========================================================================
    // Output - WriteLine (with newline)
    // ========================================================================

    static void WriteLine();
    static void WriteLine(const String& value);
    static void WriteLine(const char* value);
    static void WriteLine(char value);
    static void WriteLine(bool value);
    static void WriteLine(int value);
    static void WriteLine(unsigned int value);
    static void WriteLine(long long value);
    static void WriteLine(unsigned long long value);
    static void WriteLine(float value);
    static void WriteLine(double value);

    // Wrapper type overloads
    static void WriteLine(const Boolean& value);
    static void WriteLine(const Char& value);
    static void WriteLine(const Int8& value);
    static void WriteLine(const UInt8& value);
    static void WriteLine(const Int16& value);
    static void WriteLine(const UInt16& value);
    static void WriteLine(const Int32& value);
    static void WriteLine(const UInt32& value);
    static void WriteLine(const Int64& value);
    static void WriteLine(const UInt64& value);
    static void WriteLine(const Float32& value);
    static void WriteLine(const Float64& value);

    // ========================================================================
    // Input
    // ========================================================================

    // Read a line of text from standard input
    static String ReadLine();

    // Read a single key (blocking)
    static Char ReadKey();

    // Read a single key, optionally suppressing display
    static Char ReadKey(Boolean intercept);

    // Check if a key is available to read
    static Boolean KeyAvailable();

    // ========================================================================
    // Cursor Position
    // ========================================================================

    // Set cursor position (0-based)
    static void SetCursorPosition(Int32 left, Int32 top);

    // Get current cursor column (0-based)
    static Int32 CursorLeft();

    // Get current cursor row (0-based)
    static Int32 CursorTop();

    // ========================================================================
    // Colors
    // ========================================================================

    // Get current foreground color
    static ConsoleColor ForegroundColor();

    // Set foreground color
    static void SetForegroundColor(ConsoleColor color);

    // Get current background color
    static ConsoleColor BackgroundColor();

    // Set background color
    static void SetBackgroundColor(ConsoleColor color);

    // Reset colors to default (gray on black)
    static void ResetColor();

    // ========================================================================
    // Screen
    // ========================================================================

    // Clear the screen
    static void Clear();

    // Get window/buffer dimensions
    static Int32 WindowWidth();
    static Int32 WindowHeight();

    // ========================================================================
    // Beep
    // ========================================================================

    // Sound the console beep
    static void Beep();
};

} // namespace System

#endif // SYSTEM_CONSOLE_HPP
