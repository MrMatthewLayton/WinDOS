#include "Environment.hpp"
#include "../StringBuilder.hpp"
#include <dos.h>
#include <dpmi.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

namespace System::IO
{

/******************************************************************************/
/*    Environment private BIOS methods                                        */
/******************************************************************************/

void Environment::BiosExit(int code)
{
    __dpmi_regs regs;
    regs.h.ah = 0x4C;       // Terminate program
    regs.h.al = code;       // Return code
    __dpmi_int(0x21, &regs);

    // Fallback (should never reach here)
    std::exit(code);
}

void Environment::BiosGetVersion(int& major, int& minor)
{
    __dpmi_regs regs;
    regs.h.ah = 0x30;       // Get DOS version
    __dpmi_int(0x21, &regs);
    major = regs.h.al;
    minor = regs.h.ah;
}

/******************************************************************************/
/*    Environment public methods                                              */
/******************************************************************************/

void Environment::Exit(Int32 exitCode)
{
    BiosExit(static_cast<int>(exitCode));
}

String Environment::GetCommandLine()
{
    // Get command line from __crt0_argv (DJGPP global)
    extern char** __crt0_argv;
    extern int __crt0_argc;

    if (__crt0_argc == 0 || __crt0_argv == nullptr)
    {
        return String("");
    }

    StringBuilder sb;
    for (Int32 i = Int32(0); static_cast<int>(i) < __crt0_argc; i += 1)
    {
        if (static_cast<int>(i) > 0)
        {
            sb.Append(' ');
        }
        sb.Append(__crt0_argv[static_cast<int>(i)]);
    }

    return sb.ToString();
}

String Environment::GetEnvironmentVariable(const String& name)
{
    const char* value = std::getenv(name.CStr());
    if (value == nullptr)
    {
        return String("");
    }
    return String(value);
}

String Environment::GetCurrentDirectory()
{
    char buffer[260];  // MAX_PATH for DOS
    if (getcwd(buffer, sizeof(buffer)) == nullptr)
    {
        return String("");
    }
    return String(buffer);
}

void Environment::SetCurrentDirectory(const String& path)
{
    chdir(path.CStr());
}

String Environment::GetOSVersion()
{
    int major, minor;
    BiosGetVersion(major, minor);

    StringBuilder sb;
    sb.Append(Int32(major));
    sb.Append('.');
    if (minor < 10)
    {
        sb.Append('0');
    }
    sb.Append(Int32(minor));

    return sb.ToString();
}

} // namespace System::IO
