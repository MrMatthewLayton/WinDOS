#include "Environment.hpp"
#include "String.hpp"
#include <dos.h>
#include <dpmi.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

namespace System
{
    static constexpr int MAX_PATH_LENGTH_FOR_DOS = 260;


    // void Environment::BiosGetVersion(int& major, int& minor)
    // {

    // }

    /******************************************************************************/
    /*    Environment public methods                                              */
    /******************************************************************************/

    void Environment::Exit(Int32 code)
    {
        __dpmi_regs regs;
        regs.h.ah = 0x4C; // Terminate program
        regs.h.al = code;
        __dpmi_int(0x21, &regs);
    }

    String Environment::GetCommandLine()
    {
        // Get command line from __crt0_argv (DJGPP global)
        extern char **__crt0_argv; // NOLINT(*-reserved-identifier)
        extern int __crt0_argc; // NOLINT(*-reserved-identifier)

        if (__crt0_argc == 0 || __crt0_argv == nullptr)
            return "";

        StringBuilder sb;

        // ReSharper disable once CppUseAuto
        for (Int32 index = 0; index < Int32(__crt0_argc); ++index) // NOLINT(*-use-auto)
        {
            if (index > Int32::Zero)
                sb.Append(' ');

            sb.Append(__crt0_argv[index]);
        }

        return sb.ToString();
    }

    String Environment::GetEnvironmentVariable(const String &name)
    {
        const char *value = std::getenv(name.GetRawString());

        if (value == nullptr)
            return "";

        return value;
    }

    String Environment::GetCurrentDirectory()
    {
        char buffer[MAX_PATH_LENGTH_FOR_DOS];

        if (getcwd(buffer, sizeof(buffer)) == nullptr)
            return "";

        return buffer;
    }

    void Environment::SetCurrentDirectory(const String &path)
    {
        chdir(path.GetRawString());
    }

    String Environment::GetOSVersion()
    {
        __dpmi_regs regs;
        regs.h.ah = 0x30; // Get DOS version
        __dpmi_int(0x21, &regs);

        Int32 major = regs.h.al;
        Int32 minor = regs.h.ah;
        StringBuilder sb;

        sb.Append(Int32(major));
        sb.Append('.');

        if (minor < Int32(10))
            sb.Append('0');

        sb.Append(Int32(minor));

        return sb.ToString();
    }
} // namespace System
