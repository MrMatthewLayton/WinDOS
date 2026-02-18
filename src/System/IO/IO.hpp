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

/**
 * Provides static methods for file operations.
 *
 * This class provides a simple interface for reading files, similar to
 * the .NET System.IO.File class.
 */
class File
{
private:
    File() = delete;  // Static class, no instances

public:
    /**
     * Reads all bytes from a file and returns them as an Array<UInt8>.
     *
     * @param path The path to the file to read
     * @return Array<UInt8> containing the file contents
     * @throws FileNotFoundException if the file does not exist
     * @throws IOException if the file cannot be read
     */
    static Array<UInt8> ReadAllBytes(const char* path);

    /**
     * Checks if a file exists at the specified path.
     *
     * @param path The path to check
     * @return true if the file exists, false otherwise
     */
    static Boolean Exists(const char* path);

    /**
     * Gets the size of a file in bytes.
     *
     * @param path The path to the file
     * @return The size of the file in bytes
     * @throws FileNotFoundException if the file does not exist
     */
    static Int64 GetSize(const char* path);
};

} // namespace System::IO

#endif // SYSTEM_IO_HPP
