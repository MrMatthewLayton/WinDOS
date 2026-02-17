#include "Console.hpp"
#include "../Platform/Platform.hpp"
#include <cstdio>

namespace System {

// Static member initialization
ConsoleColor Console::_foregroundColor = ConsoleColor::Gray;
ConsoleColor Console::_backgroundColor = ConsoleColor::Black;

// ============================================================================
// Internal helpers
// ============================================================================

unsigned char Console::_getColorAttribute() {
    return (static_cast<unsigned char>(_backgroundColor) << 4) |
           static_cast<unsigned char>(_foregroundColor);
}

void Console::_writeChar(char c) {
    if (c == '\n') {
        _handleNewline();
        return;
    }

    if (c == '\r') {
        int row, col;
        Platform::DOS::Video::GetCursorPosition(row, col);
        Platform::DOS::Video::SetCursorPosition(row, 0);
        return;
    }

    if (c == '\t') {
        // Tab to next 8-column boundary
        int row, col;
        Platform::DOS::Video::GetCursorPosition(row, col);
        int spaces = 8 - (col % 8);
        for (int i = 0; i < spaces; i++) {
            _writeChar(' ');
        }
        return;
    }

    if (c == '\b') {
        int row, col;
        Platform::DOS::Video::GetCursorPosition(row, col);
        if (col > 0) {
            Platform::DOS::Video::SetCursorPosition(row, col - 1);
        }
        return;
    }

    // Regular character - write with color and advance cursor
    int rows, cols;
    Platform::DOS::Video::GetScreenSize(rows, cols);

    int row, col;
    Platform::DOS::Video::GetCursorPosition(row, col);

    // Write character with attribute
    Platform::DOS::Video::WriteChar(c, _getColorAttribute());

    // Advance cursor
    col++;
    if (col >= cols) {
        col = 0;
        row++;
        if (row >= rows) {
            // Scroll up
            Platform::DOS::Video::ScrollUp(1, _getColorAttribute(), 0, 0, rows - 1, cols - 1);
            row = rows - 1;
        }
    }

    Platform::DOS::Video::SetCursorPosition(row, col);
}

void Console::_handleNewline() {
    int rows, cols;
    Platform::DOS::Video::GetScreenSize(rows, cols);

    int row, col;
    Platform::DOS::Video::GetCursorPosition(row, col);

    row++;
    col = 0;

    if (row >= rows) {
        // Scroll up
        Platform::DOS::Video::ScrollUp(1, _getColorAttribute(), 0, 0, rows - 1, cols - 1);
        row = rows - 1;
    }

    Platform::DOS::Video::SetCursorPosition(row, col);
}

// ============================================================================
// Write implementations
// ============================================================================

void Console::Write(const String& value) {
    const char* str = value.CStr();
    while (*str) {
        _writeChar(*str++);
    }
}

void Console::Write(const char* value) {
    if (!value) return;
    while (*value) {
        _writeChar(*value++);
    }
}

void Console::Write(char value) {
    _writeChar(value);
}

void Console::Write(bool value) {
    Write(value ? "True" : "False");
}

void Console::Write(int value) {
    Write(Int32(value).ToString());
}

void Console::Write(unsigned int value) {
    Write(UInt32(value).ToString());
}

void Console::Write(long long value) {
    Write(Int64(value).ToString());
}

void Console::Write(unsigned long long value) {
    Write(UInt64(value).ToString());
}

void Console::Write(float value) {
    Write(Float32(value).ToString());
}

void Console::Write(double value) {
    Write(Float64(value).ToString());
}

// Wrapper type overloads
void Console::Write(const Boolean& value) { Write(value.ToString()); }
void Console::Write(const Char& value) { _writeChar(static_cast<char>(value)); }
void Console::Write(const Int8& value) { Write(value.ToString()); }
void Console::Write(const UInt8& value) { Write(value.ToString()); }
void Console::Write(const Int16& value) { Write(value.ToString()); }
void Console::Write(const UInt16& value) { Write(value.ToString()); }
void Console::Write(const Int32& value) { Write(value.ToString()); }
void Console::Write(const UInt32& value) { Write(value.ToString()); }
void Console::Write(const Int64& value) { Write(value.ToString()); }
void Console::Write(const UInt64& value) { Write(value.ToString()); }
void Console::Write(const Float32& value) { Write(value.ToString()); }
void Console::Write(const Float64& value) { Write(value.ToString()); }

// ============================================================================
// WriteLine implementations
// ============================================================================

void Console::WriteLine() {
    _handleNewline();
}

void Console::WriteLine(const String& value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(const char* value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(char value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(bool value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(int value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(unsigned int value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(long long value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(unsigned long long value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(float value) {
    Write(value);
    WriteLine();
}

void Console::WriteLine(double value) {
    Write(value);
    WriteLine();
}

// Wrapper type overloads
void Console::WriteLine(const Boolean& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Char& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Int8& value) { Write(value); WriteLine(); }
void Console::WriteLine(const UInt8& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Int16& value) { Write(value); WriteLine(); }
void Console::WriteLine(const UInt16& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Int32& value) { Write(value); WriteLine(); }
void Console::WriteLine(const UInt32& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Int64& value) { Write(value); WriteLine(); }
void Console::WriteLine(const UInt64& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Float32& value) { Write(value); WriteLine(); }
void Console::WriteLine(const Float64& value) { Write(value); WriteLine(); }

// ============================================================================
// Input implementations
// ============================================================================

String Console::ReadLine() {
    const int bufferSize = 256;
    char buffer[bufferSize];
    int length = 0;

    while (length < bufferSize - 1) {
        char c = Platform::DOS::Keyboard::ReadChar();

        if (c == '\r' || c == '\n') {
            // Enter pressed
            WriteLine();
            break;
        }
        else if (c == '\b') {
            // Backspace
            if (length > 0) {
                length--;
                Write('\b');
                Write(' ');
                Write('\b');
            }
        }
        else if (c >= 32) {
            // Printable character
            buffer[length++] = c;
            Write(c);
        }
    }

    buffer[length] = '\0';
    return String(buffer);
}

Char Console::ReadKey() {
    return ReadKey(false);
}

Char Console::ReadKey(bool intercept) {
    char c = Platform::DOS::Keyboard::ReadChar();
    if (!intercept) {
        Write(c);
    }
    return Char(c);
}

Boolean Console::KeyAvailable() {
    return Platform::DOS::Keyboard::IsKeyAvailable();
}

// ============================================================================
// Cursor position implementations
// ============================================================================

void Console::SetCursorPosition(Int32 left, Int32 top) {
    int rows, cols;
    Platform::DOS::Video::GetScreenSize(rows, cols);

    // Clamp values
    int l = static_cast<int>(left);
    int t = static_cast<int>(top);
    if (l < 0) l = 0;
    if (t < 0) t = 0;
    if (l >= cols) l = cols - 1;
    if (t >= rows) t = rows - 1;

    Platform::DOS::Video::SetCursorPosition(t, l);
}

Int32 Console::CursorLeft() {
    int row, col;
    Platform::DOS::Video::GetCursorPosition(row, col);
    return col;
}

Int32 Console::CursorTop() {
    int row, col;
    Platform::DOS::Video::GetCursorPosition(row, col);
    return row;
}

// ============================================================================
// Color implementations
// ============================================================================

ConsoleColor Console::ForegroundColor() {
    return _foregroundColor;
}

void Console::SetForegroundColor(ConsoleColor color) {
    _foregroundColor = color;
}

ConsoleColor Console::BackgroundColor() {
    return _backgroundColor;
}

void Console::SetBackgroundColor(ConsoleColor color) {
    _backgroundColor = color;
}

void Console::ResetColor() {
    _foregroundColor = ConsoleColor::Gray;
    _backgroundColor = ConsoleColor::Black;
}

// ============================================================================
// Screen implementations
// ============================================================================

void Console::Clear() {
    Platform::DOS::Video::ClearScreen(_getColorAttribute());
}

Int32 Console::WindowWidth() {
    int rows, cols;
    Platform::DOS::Video::GetScreenSize(rows, cols);
    return cols;
}

Int32 Console::WindowHeight() {
    int rows, cols;
    Platform::DOS::Video::GetScreenSize(rows, cols);
    return rows;
}

// ============================================================================
// Beep implementation
// ============================================================================

void Console::Beep() {
    Write('\a');  // ASCII bell
}

} // namespace System
