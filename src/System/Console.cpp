#include "Console.hpp"
#include "IO/Devices/Display.hpp"
#include "IO/Devices/Keyboard.hpp"
#include <cstdio>

namespace System
{

// Static member initialization
ConsoleColor Console::_foregroundColor = ConsoleColor::Gray;
ConsoleColor Console::_backgroundColor = ConsoleColor::Black;

// ============================================================================
// Internal helpers
// ============================================================================

UInt8 Console::_getColorAttribute()
{
    return UInt8((static_cast<unsigned char>(_backgroundColor) << 4) |
           static_cast<unsigned char>(_foregroundColor));
}

void Console::_writeChar(Char c)
{
    if (c == '\n')
    {
        _handleNewline();
        return;
    }

    if (c == '\r')
    {
        Int32 row, col;
        System::IO::Devices::Display::GetCursorPosition(row, col);
        System::IO::Devices::Display::SetCursorPosition(row, Int32(0));
        return;
    }

    if (c == '\t')
    {
        // Tab to next 8-column boundary
        Int32 row, col;
        System::IO::Devices::Display::GetCursorPosition(row, col);
        Int32 spaces = Int32(8) - (col % Int32(8));
        for (Int32 i = Int32(0); i < spaces; i += 1)
        {
            _writeChar(Char(' '));
        }
        return;
    }

    if (c == '\b')
    {
        Int32 row, col;
        System::IO::Devices::Display::GetCursorPosition(row, col);
        if (col > 0)
        {
            System::IO::Devices::Display::SetCursorPosition(row, col - 1);
        }
        return;
    }

    // Regular character - write with color and advance cursor
    Int32 rows, cols;
    System::IO::Devices::Display::GetScreenSize(rows, cols);

    Int32 row, col;
    System::IO::Devices::Display::GetCursorPosition(row, col);

    // Write character with attribute
    System::IO::Devices::Display::WriteChar(static_cast<char>(c), static_cast<unsigned char>(_getColorAttribute()));

    // Advance cursor
    col += 1;
    if (col >= cols)
    {
        col = Int32(0);
        row += 1;
        if (row >= rows)
        {
            // Scroll up
            System::IO::Devices::Display::ScrollUp(Int32(1), _getColorAttribute(), Int32(0), Int32(0), cols - 1, rows - 1);
            row = rows - 1;
        }
    }

    System::IO::Devices::Display::SetCursorPosition(row, col);
}

void Console::_handleNewline()
{
    Int32 rows, cols;
    System::IO::Devices::Display::GetScreenSize(rows, cols);

    Int32 row, col;
    System::IO::Devices::Display::GetCursorPosition(row, col);

    row += 1;
    col = Int32(0);

    if (row >= rows)
    {
        // Scroll up
        System::IO::Devices::Display::ScrollUp(Int32(1), _getColorAttribute(), Int32(0), Int32(0), cols - 1, rows - 1);
        row = rows - 1;
    }

    System::IO::Devices::Display::SetCursorPosition(row, col);
}

// ============================================================================
// Write implementations
// ============================================================================

void Console::Write(const String& value)
{
    const char* str = value.GetRawString();
    while (*str)
    {
        _writeChar(*str++);
    }
}

void Console::Write(const char* value)
{
    if (!value)
    {
        return;
    }
    while (*value)
    {
        _writeChar(*value++);
    }
}

void Console::Write(char value)
{
    _writeChar(Char(value));
}

void Console::Write(bool value)
{
    Write(value ? "True" : "False");
}

void Console::Write(int value)
{
    Write(Int32(value).ToString());
}

void Console::Write(unsigned int value)
{
    Write(UInt32(value).ToString());
}

void Console::Write(long long value)
{
    Write(Int64(value).ToString());
}

void Console::Write(unsigned long long value)
{
    Write(UInt64(value).ToString());
}

void Console::Write(float value)
{
    Write(Float32(value).ToString());
}

void Console::Write(double value)
{
    Write(Float64(value).ToString());
}

// Wrapper type overloads
void Console::Write(const Boolean& value)
{
    Write(value.ToString());
}

void Console::Write(const Char& value)
{
    _writeChar(static_cast<char>(value));
}

void Console::Write(const Int8& value)
{
    Write(value.ToString());
}

void Console::Write(const UInt8& value)
{
    Write(value.ToString());
}

void Console::Write(const Int16& value)
{
    Write(value.ToString());
}

void Console::Write(const UInt16& value)
{
    Write(value.ToString());
}

void Console::Write(const Int32& value)
{
    Write(value.ToString());
}

void Console::Write(const UInt32& value)
{
    Write(value.ToString());
}

void Console::Write(const Int64& value)
{
    Write(value.ToString());
}

void Console::Write(const UInt64& value)
{
    Write(value.ToString());
}

void Console::Write(const Float32& value)
{
    Write(value.ToString());
}

void Console::Write(const Float64& value)
{
    Write(value.ToString());
}

// ============================================================================
// WriteLine implementations
// ============================================================================

void Console::WriteLine()
{
    _handleNewline();
}

void Console::WriteLine(const String& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const char* value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(char value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(bool value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(int value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(unsigned int value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(long long value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(unsigned long long value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(float value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(double value)
{
    Write(value);
    WriteLine();
}

// Wrapper type overloads
void Console::WriteLine(const Boolean& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Char& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Int8& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const UInt8& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Int16& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const UInt16& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Int32& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const UInt32& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Int64& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const UInt64& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Float32& value)
{
    Write(value);
    WriteLine();
}

void Console::WriteLine(const Float64& value)
{
    Write(value);
    WriteLine();
}

// ============================================================================
// Input implementations
// ============================================================================

String Console::ReadLine()
{
    const Int32 bufferSize = Int32(256);
    char buffer[256];
    Int32 length = Int32(0);

    while (length < bufferSize - 1)
    {
        Char c = Char(System::IO::Devices::Keyboard::ReadChar());

        if (c == '\r' || c == '\n')
        {
            // Enter pressed
            WriteLine();
            break;
        }
        else if (c == '\b')
        {
            // Backspace
            if (length > 0)
            {
                length -= 1;
                Write('\b');
                Write(' ');
                Write('\b');
            }
        }
        else if (c >= Char(32))
        {
            // Printable character
            buffer[static_cast<int>(length)] = static_cast<char>(c);
            length += 1;
            Write(static_cast<char>(c));
        }
    }

    buffer[static_cast<int>(length)] = '\0';
    return String(buffer);
}

Char Console::ReadKey()
{
    return ReadKey(false);
}

Char Console::ReadKey(Boolean intercept)
{
    Char c = Char(System::IO::Devices::Keyboard::ReadChar());
    if (!intercept)
    {
        Write(static_cast<char>(c));
    }
    return c;
}

Boolean Console::KeyAvailable()
{
    return System::IO::Devices::Keyboard::IsKeyAvailable();
}

// ============================================================================
// Cursor position implementations
// ============================================================================

void Console::SetCursorPosition(Int32 left, Int32 top)
{
    Int32 rows, cols;
    System::IO::Devices::Display::GetScreenSize(rows, cols);

    // Clamp values
    Int32 l = left;
    Int32 t = top;
    if (l < 0)
    {
        l = Int32(0);
    }
    if (t < 0)
    {
        t = Int32(0);
    }
    if (l >= cols)
    {
        l = cols - 1;
    }
    if (t >= rows)
    {
        t = rows - 1;
    }

    System::IO::Devices::Display::SetCursorPosition(t, l);
}

Int32 Console::CursorLeft()
{
    Int32 row, col;
    System::IO::Devices::Display::GetCursorPosition(row, col);
    return col;
}

Int32 Console::CursorTop()
{
    Int32 row, col;
    System::IO::Devices::Display::GetCursorPosition(row, col);
    return row;
}

// ============================================================================
// Color implementations
// ============================================================================

ConsoleColor Console::ForegroundColor()
{
    return _foregroundColor;
}

void Console::SetForegroundColor(ConsoleColor color)
{
    _foregroundColor = color;
}

ConsoleColor Console::BackgroundColor()
{
    return _backgroundColor;
}

void Console::SetBackgroundColor(ConsoleColor color)
{
    _backgroundColor = color;
}

void Console::ResetColor()
{
    _foregroundColor = ConsoleColor::Gray;
    _backgroundColor = ConsoleColor::Black;
}

// ============================================================================
// Screen implementations
// ============================================================================

void Console::Clear()
{
    System::IO::Devices::Display::ClearScreen(_getColorAttribute());
}

Int32 Console::WindowWidth()
{
    Int32 rows, cols;
    System::IO::Devices::Display::GetScreenSize(rows, cols);
    return cols;
}

Int32 Console::WindowHeight()
{
    Int32 rows, cols;
    System::IO::Devices::Display::GetScreenSize(rows, cols);
    return rows;
}

// ============================================================================
// Beep implementation
// ============================================================================

void Console::Beep()
{
    _writeChar(Char('\a'));  // ASCII bell
}

} // namespace System
