#ifndef SYSTEM_IO_HPP
#define SYSTEM_IO_HPP

#include "../Array.hpp"
#include "../Types.hpp"
#include "../Exception.hpp"

namespace System::IO
{

/******************************************************************************/
/*    System::IO::File                                                        */
/******************************************************************************/

/// @brief Provides static methods for file operations.
///
/// This class provides a simple interface for reading files, similar to
/// the .NET System.IO.File class. All methods are static; this class
/// cannot be instantiated.
///
/// @code
/// // Check if a file exists and read its contents
/// if (File::Exists("C:\\DATA.BIN")) {
///     Array<UInt8> bytes = File::ReadAllBytes("C:\\DATA.BIN");
///     Int64 size = File::GetSize("C:\\DATA.BIN");
/// }
/// @endcode
class File
{
private:
    /// @brief Deleted constructor to prevent instantiation.
    ///
    /// File is a static utility class and should not be instantiated.
    File() = delete;

public:
    /// @brief Reads all bytes from a file and returns them as an Array.
    ///
    /// Opens the specified file, reads its entire contents into memory,
    /// and returns the data as an Array<UInt8>. The file is automatically
    /// closed after reading.
    ///
    /// @param path The path to the file to read. Use backslashes for DOS paths.
    /// @return Array<UInt8> containing the complete file contents.
    /// @throws FileNotFoundException If the file does not exist at the specified path.
    /// @throws IOException If the file cannot be opened or read due to an I/O error.
    ///
    /// @note This method reads the entire file into memory. For very large files,
    ///       consider using a streaming approach instead.
    ///
    /// @code
    /// Array<UInt8> data = File::ReadAllBytes("C:\\IMAGE.BMP");
    /// UInt8 firstByte = data[0];
    /// @endcode
    static Array<UInt8> ReadAllBytes(const char* path);

    /// @brief Checks if a file exists at the specified path.
    ///
    /// Determines whether the specified file exists. This method does not
    /// throw an exception if the file does not exist; it simply returns false.
    ///
    /// @param path The path to check for existence.
    /// @return true if the file exists and is accessible; false otherwise.
    ///
    /// @note This method may return false for files that exist but are not
    ///       accessible due to permissions or other restrictions.
    ///
    /// @code
    /// if (File::Exists("C:\\CONFIG.SYS")) {
    ///     // File exists, safe to read
    /// }
    /// @endcode
    static Boolean Exists(const char* path);

    /// @brief Gets the size of a file in bytes.
    ///
    /// Returns the size of the specified file in bytes. The file must exist;
    /// otherwise, a FileNotFoundException is thrown.
    ///
    /// @param path The path to the file.
    /// @return The size of the file in bytes as an Int64 value.
    /// @throws FileNotFoundException If the file does not exist at the specified path.
    ///
    /// @code
    /// Int64 fileSize = File::GetSize("C:\\DATA.BIN");
    /// if (fileSize > 1024 * 1024) {
    ///     // File is larger than 1 MB
    /// }
    /// @endcode
    static Int64 GetSize(const char* path);
};

} // namespace System::IO

#endif // SYSTEM_IO_HPP
