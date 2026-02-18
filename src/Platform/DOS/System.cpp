#include "System.hpp"
#include <dos.h>
#include <dpmi.h>
#include <cstdlib>

namespace Platform::DOS
{

void DOSSystem::WriteString(const char* s)
{
    while (*s)
    {
        WriteChar(*s++);
    }
}

void DOSSystem::WriteChar(char c)
{
    __dpmi_regs regs;
    regs.h.ah = 0x02;       // Write character to stdout
    regs.h.dl = c;          // Character to write
    __dpmi_int(0x21, &regs);
}

char DOSSystem::ReadChar()
{
    __dpmi_regs regs;
    regs.h.ah = 0x01;       // Read character with echo
    __dpmi_int(0x21, &regs);
    return regs.h.al;
}

int DOSSystem::ReadLine(char* buffer, int maxLength)
{
    int i = 0;
    char c;

    while (i < maxLength - 1)
    {
        c = ReadChar();

        if (c == '\r')      // Enter pressed
        {
            WriteChar('\n');
            break;
        }
        else if (c == '\b' && i > 0)    // Backspace
        {
            i--;
            WriteChar(' ');     // Erase character
            WriteChar('\b');
        }
        else if (c >= 32)   // Printable character
        {
            buffer[i++] = c;
        }
    }

    buffer[i] = '\0';
    return i;
}

void DOSSystem::Exit(int code)
{
    __dpmi_regs regs;
    regs.h.ah = 0x4C;       // Terminate program
    regs.h.al = code;       // Return code
    __dpmi_int(0x21, &regs);

    // Fallback (should never reach here)
    std::exit(code);
}

void DOSSystem::GetVersion(int& major, int& minor)
{
    __dpmi_regs regs;
    regs.h.ah = 0x30;       // Get DOS version
    __dpmi_int(0x21, &regs);
    major = regs.h.al;
    minor = regs.h.ah;
}

} // namespace Platform::DOS
