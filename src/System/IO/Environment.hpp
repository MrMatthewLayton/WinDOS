#ifndef SYSTEM_IO_ENVIRONMENT_HPP
#define SYSTEM_IO_ENVIRONMENT_HPP

#include "../Types.hpp"
#include "../String.hpp"

namespace System::IO
{

/// @brief Provides information about and access to the current environment.
///
/// The Environment class offers static methods for accessing system-level
/// information and services such as environment variables, working directory,
/// OS version, and process termination. This class mirrors the .NET
/// System.Environment API.
///
/// @note This class is a static facade and cannot be instantiated.
class Environment
{
    Environment() = delete;  // Static class

    // Low-level DOS system calls - private
    static void BiosExit(int code);
    static void BiosGetVersion(int& major, int& minor);

public:
    /// @brief Terminates the process with the specified exit code.
    /// @param exitCode The exit code to return to the operating system
    ///
    /// Calls DOS INT 21h function 4Ch to terminate the process.
    /// This function does not return. By convention, 0 indicates success.
    static void Exit(Int32 exitCode);

    /// @brief Gets the command line for the process.
    /// @return The command line string
    ///
    /// Returns the full command line including the program name and all arguments.
    static String GetCommandLine();

    /// @brief Gets the value of an environment variable.
    /// @param name The name of the environment variable
    /// @return The value, or empty string if not found
    ///
    /// Queries the DOS environment for the specified variable name.
    /// Variable names are case-insensitive in DOS.
    static String GetEnvironmentVariable(const String& name);

    /// @brief Gets the current working directory.
    /// @return The full path of the current directory
    ///
    /// Returns the current working directory as a full path (e.g., "C:\\DOS").
    static String GetCurrentDirectory();

    /// @brief Sets the current working directory.
    /// @param path The new current directory path
    ///
    /// Changes the current working directory to the specified path.
    /// The path can be relative or absolute.
    static void SetCurrentDirectory(const String& path);

    /// @brief Gets the DOS version.
    /// @return The major and minor version as a formatted string (e.g., "7.10")
    ///
    /// Queries the DOS version using INT 21h function 30h.
    /// For DJGPP programs running under CWSDPMI, this returns
    /// the underlying DOS version.
    static String GetOSVersion();

    /// @brief Gets the newline string for this environment.
    /// @return "\r\n" for DOS
    ///
    /// DOS uses CRLF (carriage return + line feed) for line endings.
    static const char* NewLine()
    {
        return "\r\n";
    }
};

} // namespace System::IO

#endif // SYSTEM_IO_ENVIRONMENT_HPP
