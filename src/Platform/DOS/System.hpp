#ifndef PLATFORM_DOS_SYSTEM_HPP
#define PLATFORM_DOS_SYSTEM_HPP

namespace Platform::DOS
{

/// @brief Low-level DOS system services wrapper.
///
/// Provides direct access to DOS system calls for console I/O and process control.
/// This class serves as the platform abstraction layer, isolating DOS-specific
/// INT 21h calls from the higher-level System namespace classes.
///
/// All methods are static as this is a stateless facade over DOS services.
///
/// @note This class is intended for internal use by the BCL. Application code
///       should use System::Console for I/O operations.
class DOSSystem
{
public:
    /// @brief Writes a null-terminated string to standard output.
    ///
    /// Outputs each character of the string to stdout using DOS INT 21h.
    /// The null terminator is not written.
    ///
    /// @param s Pointer to a null-terminated string to write.
    ///          If nullptr, no output is produced.
    static void WriteString(const char* s);

    /// @brief Writes a single character to standard output.
    ///
    /// Outputs one character to stdout using DOS INT 21h function 02h.
    ///
    /// @param c The character to write.
    static void WriteChar(char c);

    /// @brief Reads a single character from standard input.
    ///
    /// Blocks until a character is available from stdin.
    /// Uses DOS INT 21h function 01h which echoes the character.
    ///
    /// @return The character read from stdin.
    static char ReadChar();

    /// @brief Reads a line of text from standard input into a buffer.
    ///
    /// Reads characters until Enter is pressed or the buffer is full.
    /// The newline character is not included in the buffer.
    /// The buffer is null-terminated.
    ///
    /// @param buffer Pointer to the buffer to receive the input.
    ///               Must be at least maxLength bytes.
    /// @param maxLength Maximum number of characters to read, including
    ///                  the null terminator.
    /// @return The number of characters read, not including the null terminator.
    static int ReadLine(char* buffer, int maxLength);

    /// @brief Terminates the program with the specified exit code.
    ///
    /// Calls DOS INT 21h function 4Ch to terminate the process.
    /// This function does not return.
    ///
    /// @param code The exit code to return to the parent process.
    ///             By convention, 0 indicates success.
    static void Exit(int code);

    /// @brief Retrieves the DOS version number.
    ///
    /// Queries the DOS version using INT 21h function 30h.
    /// For DJGPP programs running under CWSDPMI, this returns
    /// the underlying DOS version.
    ///
    /// @param[out] major Receives the major version number (e.g., 7 for DOS 7.x).
    /// @param[out] minor Receives the minor version number (e.g., 10 for DOS 7.10).
    static void GetVersion(int& major, int& minor);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_SYSTEM_HPP
