#ifndef PLATFORM_DOS_VIDEO_HPP
#define PLATFORM_DOS_VIDEO_HPP

namespace Platform::DOS
{

class Video
{
public:
    // Set cursor position (0-based row and column)
    static void SetCursorPosition(int row, int col);

    // Get current cursor position
    static void GetCursorPosition(int& row, int& col);

    // Write character with attribute at current cursor position
    static void WriteChar(char c, unsigned char attr);

    // Write character at cursor position (uses current attribute)
    static void WriteCharAtCursor(char c);

    // Set video mode (e.g., 0x03 for 80x25 text)
    static void SetVideoMode(int mode);

    // Scroll window up
    static void ScrollUp(int lines, unsigned char attr, int top, int left, int bottom, int right);

    // Get screen dimensions
    static void GetScreenSize(int& rows, int& cols);

    // Clear screen with attribute
    static void ClearScreen(unsigned char attr);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_VIDEO_HPP
