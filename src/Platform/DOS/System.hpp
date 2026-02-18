#ifndef PLATFORM_DOS_SYSTEM_HPP
#define PLATFORM_DOS_SYSTEM_HPP

namespace Platform::DOS
{

class DOSSystem
{
public:
    // Write a null-terminated string to stdout
    static void WriteString(const char* s);

    // Write a single character to stdout
    static void WriteChar(char c);

    // Read a character from stdin (blocking)
    static char ReadChar();

    // Read a line from stdin into buffer, returns length
    static int ReadLine(char* buffer, int maxLength);

    // Exit program with return code
    static void Exit(int code);

    // Get DOS version
    static void GetVersion(int& major, int& minor);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_SYSTEM_HPP
