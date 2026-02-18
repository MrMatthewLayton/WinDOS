#include "IO.hpp"
#include <cstdio>
#include <cstdlib>

namespace System { namespace IO {

/******************************************************************************/
/*    File Implementation                                                     */
/******************************************************************************/

Array<UInt8> File::ReadAllBytes(const char* path) {
    if (!path) {
        throw ArgumentNullException("path");
    }

    FILE* file = std::fopen(path, "rb");
    if (!file) {
        throw FileNotFoundException(path);
    }

    // Get file size
    std::fseek(file, 0, SEEK_END);
    long fileSize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        std::fclose(file);
        throw IOException("File is empty or size cannot be determined.");
    }

    // Allocate buffer
    unsigned char* buffer = static_cast<unsigned char*>(std::malloc(fileSize));
    if (!buffer) {
        std::fclose(file);
        throw IOException("Failed to allocate memory for file contents.");
    }

    // Read file contents
    size_t bytesRead = std::fread(buffer, 1, fileSize, file);
    std::fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize)) {
        std::free(buffer);
        throw IOException("Failed to read complete file contents.");
    }

    // Create array and copy data
    Int32 length = Int32(static_cast<int>(fileSize));
    Array<UInt8> result(length);
    for (Int32 i = Int32(0); i < length; i += 1) {
        result[static_cast<int>(i)] = UInt8(buffer[static_cast<int>(i)]);
    }

    std::free(buffer);
    return result;
}

Boolean File::Exists(const char* path) {
    if (!path) {
        return Boolean(false);
    }

    FILE* file = std::fopen(path, "rb");
    if (file) {
        std::fclose(file);
        return Boolean(true);
    }
    return Boolean(false);
}

Int64 File::GetSize(const char* path) {
    if (!path) {
        throw ArgumentNullException("path");
    }

    FILE* file = std::fopen(path, "rb");
    if (!file) {
        throw FileNotFoundException(path);
    }

    std::fseek(file, 0, SEEK_END);
    long fileSize = std::ftell(file);
    std::fclose(file);

    return Int64(fileSize);
}

}} // namespace System::IO
