/// @file Image.cpp
/// @brief Implementation of System.Drawing.Image class.

#include "Image.hpp"
#include "../Exception.hpp"
#include "../IO/File.hpp"
#include "../../ThirdParty/stb_image.h"
#include <cstdlib>
#include <cstring>

namespace System::Drawing
{

/******************************************************************************/
/*    Image implementation (unified 32-bit ARGB)                              */
/******************************************************************************/

void Image::_allocate(Int32 w, Int32 h, UInt32 fill)
{
    _width = static_cast<int>(w);
    _height = static_cast<int>(h);
    Int32 size = Int32(static_cast<int>(w) * static_cast<int>(h));
    if (static_cast<int>(size) > 0)
    {
        _data = static_cast<unsigned int*>(std::malloc(static_cast<int>(size) * sizeof(unsigned int)));
        if (_data)
        {
            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(size); i += 1)
            {
                _data[static_cast<int>(i)] = static_cast<unsigned int>(fill);
            }
        }
    }
    else
    {
        _data = nullptr;
    }
}

void Image::_free()
{
    if (_data)
    {
        std::free(_data);
        _data = nullptr;
    }
    _width = 0;
    _height = 0;
}

void Image::_copy(const Image& other)
{
    _allocate(Int32(other._width), Int32(other._height), UInt32(0));
    if (_data && other._data)
    {
        std::memcpy(_data, other._data, _width * _height * sizeof(unsigned int));
    }
}

Image::Image() : _data(nullptr), _width(0), _height(0) {}

Image::Image(Int32 width, Int32 height, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0)
{
    _allocate(static_cast<int>(width), static_cast<int>(height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Size& size, const Color& fillColor)
    : _data(nullptr), _width(0), _height(0)
{
    _allocate(static_cast<int>(size.width), static_cast<int>(size.height),
              static_cast<unsigned int>(fillColor));
}

Image::Image(const Image& other) : _data(nullptr), _width(0), _height(0)
{
    _copy(other);
}

Image::Image(Image&& other) noexcept
    : _data(other._data), _width(other._width), _height(other._height)
{
    other._data = nullptr;
    other._width = 0;
    other._height = 0;
}

Image::~Image()
{
    _free();
}

Image& Image::operator=(const Image& other)
{
    if (this != &other)
    {
        _free();
        _copy(other);
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept
{
    if (this != &other)
    {
        _free();
        _data = other._data;
        _width = other._width;
        _height = other._height;
        other._data = nullptr;
        other._width = 0;
        other._height = 0;
    }
    return *this;
}

Color Image::GetPixel(Int32 x, Int32 y) const
{
    Int32 xi = x;
    Int32 yi = y;
    if (static_cast<int>(xi) < 0 || static_cast<int>(yi) < 0 || static_cast<int>(xi) >= _width || static_cast<int>(yi) >= _height || !_data)
    {
        return Color::Transparent;
    }
    return Color(_data[static_cast<int>(yi) * _width + static_cast<int>(xi)]);
}

void Image::SetPixel(Int32 x, Int32 y, const Color& color)
{
    Int32 xi = x;
    Int32 yi = y;
    if (static_cast<int>(xi) < 0 || static_cast<int>(yi) < 0 || static_cast<int>(xi) >= _width || static_cast<int>(yi) >= _height || !_data)
    {
        return;
    }
    _data[static_cast<int>(yi) * _width + static_cast<int>(xi)] = static_cast<unsigned int>(color);
}

void Image::SetPixel(const Point& pt, const Color& color)
{
    SetPixel(pt.x, pt.y, color);
}

void Image::Clear(const Color& color)
{
    if (_data && _width > 0 && _height > 0)
    {
        UInt32 val = UInt32(static_cast<unsigned int>(color));
        Int32 size = Int32(_width * _height);
        for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(size); i += 1)
        {
            _data[static_cast<int>(i)] = static_cast<unsigned int>(val);
        }
    }
}

void Image::CopyFrom(const Image& src, Int32 destX, Int32 destY)
{
    if (!_data || !src._data) return;

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;

        Int32 srcStartX = Int32(0);
        Int32 dstStartX = dstX;
        Int32 copyWidth = Int32(src._width);

        if (static_cast<int>(dstStartX) < 0)
        {
            srcStartX = Int32(-static_cast<int>(dstStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(dstStartX));
            dstStartX = Int32(0);
        }
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(dstStartX));
        }
        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = _data + static_cast<int>(dy) * _width + static_cast<int>(dstStartX);
        const unsigned int* srcRow = src._data + static_cast<int>(sy) * src._width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
}

void Image::CopyFrom(const Image& src, const Point& dest)
{
    CopyFrom(src, dest.x, dest.y);
}

void Image::CopyFromWithAlpha(const Image& src, Int32 destX, Int32 destY)
{
    if (!_data || !src._data) return;

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;

        for (Int32 sx = Int32(0); static_cast<int>(sx) < src._width; sx += 1)
        {
            Int32 dx = Int32(static_cast<int>(dstX) + static_cast<int>(sx));
            if (static_cast<int>(dx) < 0 || static_cast<int>(dx) >= _width) continue;

            UInt32 pixel = UInt32(src._data[static_cast<int>(sy) * src._width + static_cast<int>(sx)]);
            // Only copy if alpha >= 128 (semi-opaque or opaque)
            if ((static_cast<unsigned int>(pixel) >> 24) >= 128)
            {
                _data[static_cast<int>(dy) * _width + static_cast<int>(dx)] = static_cast<unsigned int>(pixel);
            }
        }
    }
}

void Image::CopyFromClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect)
{
    if (!_data || !src._data) return;

    // Get clip bounds
    int clipLeft = static_cast<int>(clipRect.x);
    int clipTop = static_cast<int>(clipRect.y);
    int clipRight = clipLeft + static_cast<int>(clipRect.width);
    int clipBottom = clipTop + static_cast<int>(clipRect.height);

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;
        if (static_cast<int>(dy) < clipTop || static_cast<int>(dy) >= clipBottom) continue;

        Int32 srcStartX = Int32(0);
        Int32 dstStartX = dstX;
        Int32 copyWidth = Int32(src._width);

        // Clip to image bounds
        if (static_cast<int>(dstStartX) < 0)
        {
            srcStartX = Int32(-static_cast<int>(dstStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(dstStartX));
            dstStartX = Int32(0);
        }
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(dstStartX));
        }

        // Clip to clip rectangle (left)
        if (static_cast<int>(dstStartX) < clipLeft)
        {
            Int32 diff = Int32(clipLeft - static_cast<int>(dstStartX));
            srcStartX = Int32(static_cast<int>(srcStartX) + static_cast<int>(diff));
            copyWidth = Int32(static_cast<int>(copyWidth) - static_cast<int>(diff));
            dstStartX = Int32(clipLeft);
        }
        // Clip to clip rectangle (right)
        if (static_cast<int>(dstStartX) + static_cast<int>(copyWidth) > clipRight)
        {
            copyWidth = Int32(clipRight - static_cast<int>(dstStartX));
        }

        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = _data + static_cast<int>(dy) * _width + static_cast<int>(dstStartX);
        const unsigned int* srcRow = src._data + static_cast<int>(sy) * src._width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
}

void Image::CopyFromWithAlphaClipped(const Image& src, Int32 destX, Int32 destY, const Rectangle& clipRect)
{
    if (!_data || !src._data) return;

    // Get clip bounds
    int clipLeft = static_cast<int>(clipRect.x);
    int clipTop = static_cast<int>(clipRect.y);
    int clipRight = clipLeft + static_cast<int>(clipRect.width);
    int clipBottom = clipTop + static_cast<int>(clipRect.height);

    Int32 dstX = destX;
    Int32 dstY = destY;

    for (Int32 sy = Int32(0); static_cast<int>(sy) < src._height; sy += 1)
    {
        Int32 dy = Int32(static_cast<int>(dstY) + static_cast<int>(sy));
        if (static_cast<int>(dy) < 0 || static_cast<int>(dy) >= _height) continue;
        if (static_cast<int>(dy) < clipTop || static_cast<int>(dy) >= clipBottom) continue;

        for (Int32 sx = Int32(0); static_cast<int>(sx) < src._width; sx += 1)
        {
            Int32 dx = Int32(static_cast<int>(dstX) + static_cast<int>(sx));
            if (static_cast<int>(dx) < 0 || static_cast<int>(dx) >= _width) continue;
            if (static_cast<int>(dx) < clipLeft || static_cast<int>(dx) >= clipRight) continue;

            UInt32 pixel = UInt32(src._data[static_cast<int>(sy) * src._width + static_cast<int>(sx)]);
            // Only copy if alpha >= 128 (semi-opaque or opaque)
            if ((static_cast<unsigned int>(pixel) >> 24) >= 128)
            {
                _data[static_cast<int>(dy) * _width + static_cast<int>(dx)] = static_cast<unsigned int>(pixel);
            }
        }
    }
}

Image Image::GetRegion(Int32 x, Int32 y, Int32 width, Int32 height) const
{
    Int32 xi = x;
    Int32 yi = y;
    Int32 wi = width;
    Int32 hi = height;

    Image result(width, height, Color::Transparent);
    if (!_data || !result._data) return result;

    for (Int32 dy = Int32(0); static_cast<int>(dy) < static_cast<int>(hi); dy += 1)
    {
        Int32 sy = Int32(static_cast<int>(yi) + static_cast<int>(dy));
        if (static_cast<int>(sy) < 0 || static_cast<int>(sy) >= _height) continue;

        Int32 srcStartX = xi;
        Int32 dstStartX = Int32(0);
        Int32 copyWidth = wi;

        if (static_cast<int>(srcStartX) < 0)
        {
            dstStartX = Int32(-static_cast<int>(srcStartX));
            copyWidth = Int32(static_cast<int>(copyWidth) + static_cast<int>(srcStartX));
            srcStartX = Int32(0);
        }
        if (static_cast<int>(srcStartX) + static_cast<int>(copyWidth) > _width)
        {
            copyWidth = Int32(_width - static_cast<int>(srcStartX));
        }
        if (static_cast<int>(copyWidth) <= 0) continue;

        unsigned int* dstRow = result._data + static_cast<int>(dy) * static_cast<int>(wi) + static_cast<int>(dstStartX);
        const unsigned int* srcRow = _data + static_cast<int>(sy) * _width + static_cast<int>(srcStartX);
        std::memcpy(dstRow, srcRow, static_cast<int>(copyWidth) * sizeof(unsigned int));
    }
    return result;
}

Image Image::GetRegion(const Rectangle& rect) const
{
    return GetRegion(rect.x, rect.y, rect.width, rect.height);
}

Image Image::FromBitmap(const char* path)
{
    const UInt16 BMP_SIGNATURE = UInt16(0x4D42);  // 'BM'
    const Int32 FILE_HEADER_SIZE = Int32(sizeof(BitmapFileHeader));
    const Int32 INFO_HEADER_SIZE = Int32(sizeof(BitmapInfoHeader));

    // Validate path
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < static_cast<int>(FILE_HEADER_SIZE) + static_cast<int>(INFO_HEADER_SIZE))
    {
        throw InvalidDataException("File is too small to be a valid BMP.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory for file data.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse headers
    const BitmapFileHeader* fileHeader = reinterpret_cast<const BitmapFileHeader*>(fileData);
    if (static_cast<unsigned short>(fileHeader->Type()) != static_cast<unsigned short>(BMP_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("File is not a valid BMP (invalid signature).");
    }

    const BitmapInfoHeader* infoHeader = reinterpret_cast<const BitmapInfoHeader*>(fileData + static_cast<int>(FILE_HEADER_SIZE));
    Int32 bitCount = Int32(static_cast<int>(infoHeader->BitCount()));

    if (static_cast<unsigned int>(infoHeader->Compression()) != 0)
    {
        std::free(fileData);
        throw InvalidDataException("Compressed BMP files are not supported.");
    }

    Int32 width = Int32(static_cast<int>(infoHeader->Width()));
    Int32 height = Int32(static_cast<int>(infoHeader->Height()));

    if (static_cast<int>(width) <= 0 || static_cast<int>(height) <= 0)
    {
        std::free(fileData);
        throw InvalidDataException("BMP has invalid dimensions.");
    }

    // Get pixel data
    const unsigned char* pixelData = fileData + static_cast<unsigned int>(fileHeader->Offset());

    // Create output image (32-bit ARGB)
    Image result(width, height);
    unsigned int* outPixels = result.Data();

    // Handle different bit depths
    if (static_cast<int>(bitCount) == 4)
    {
        // 4bpp - need palette
        UInt32 paletteCount = UInt32(static_cast<unsigned int>(infoHeader->UsedColors()));
        if (static_cast<unsigned int>(paletteCount) == 0) paletteCount = UInt32(16);
        const unsigned char* paletteData = fileData + static_cast<int>(FILE_HEADER_SIZE) +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        Int32 bytesPerLine = Int32((((static_cast<int>(width) + 1) / 2) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char byteVal = row[static_cast<int>(x) / 2];
                unsigned char index = ((static_cast<int>(x) & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                if (index < static_cast<unsigned int>(paletteCount))
                {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else if (static_cast<int>(bitCount) == 8)
    {
        // 8bpp - need palette
        UInt32 paletteCount = UInt32(static_cast<unsigned int>(infoHeader->UsedColors()));
        if (static_cast<unsigned int>(paletteCount) == 0) paletteCount = UInt32(256);
        const unsigned char* paletteData = fileData + static_cast<int>(FILE_HEADER_SIZE) +
                                           static_cast<unsigned int>(infoHeader->HeaderSize());

        Int32 bytesPerLine = Int32((static_cast<int>(width) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char index = row[static_cast<int>(x)];
                if (index < static_cast<unsigned int>(paletteCount))
                {
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else if (static_cast<int>(bitCount) == 24)
    {
        // 24bpp - direct RGB
        Int32 bytesPerLine = Int32(((static_cast<int>(width) * 3) + 3) & ~3);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 3 + 0];
                unsigned char g = row[static_cast<int>(x) * 3 + 1];
                unsigned char r = row[static_cast<int>(x) * 3 + 2];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    }
    else if (static_cast<int>(bitCount) == 32)
    {
        // 32bpp - direct BGRA
        Int32 bytesPerLine = Int32(static_cast<int>(width) * 4);

        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = pixelData + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(bytesPerLine);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 4 + 0];
                unsigned char g = row[static_cast<int>(x) * 4 + 1];
                unsigned char r = row[static_cast<int>(x) * 4 + 2];
                unsigned char a = row[static_cast<int>(x) * 4 + 3];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    }
    else
    {
        std::free(fileData);
        throw InvalidDataException("Unsupported BMP bit depth. Supported: 4, 8, 24, 32.");
    }

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    PE Format Structures for Icon Library Parsing                            */
/******************************************************************************/

#pragma pack(push, 1)

struct MsDosExecutableHeader {
    unsigned short signature;      // 'MZ' = 0x5A4D
    unsigned short lastPageBytes;
    unsigned short pageCount;
    unsigned short relocationCount;
    unsigned short headerParagraphs;
    unsigned short minAllocParagraphs;
    unsigned short maxAllocParagraphs;
    unsigned short initialSS;
    unsigned short initialSP;
    unsigned short checksum;
    unsigned short initialIP;
    unsigned short initialCS;
    unsigned short relocationOffset;
    unsigned short overlayNumber;
    unsigned short reserved1[4];
    unsigned short oemId;
    unsigned short oemInfo;
    unsigned short reserved2[10];
    unsigned int   newHeaderOffset;   // Offset to PE header

    unsigned short Signature() const { return signature; }
    unsigned int   NewHeaderOffset() const { return newHeaderOffset; }
};

struct PortableExecutableFileHeader {
    unsigned short machine;
    unsigned short sectionCount;
    unsigned int   timestamp;
    unsigned int   symbolTablePointer;
    unsigned int   symbolCount;
    unsigned short optionalHeaderSize;
    unsigned short characteristics;

    unsigned short SectionCount() const { return sectionCount; }
    unsigned short OptionalHeaderSize() const { return optionalHeaderSize; }
};

struct PortableExecutableDataDirectory {
    unsigned int virtualAddress;
    unsigned int size;

    unsigned int VirtualAddress() const { return virtualAddress; }
    unsigned int Size() const { return size; }
};

struct PortableExecutableOptionalHeader {
    unsigned short magic;
    unsigned char  majorLinkerVersion;
    unsigned char  minorLinkerVersion;
    unsigned int   sizeOfCode;
    unsigned int   sizeOfInitializedData;
    unsigned int   sizeOfUninitializedData;
    unsigned int   entryPointAddress;
    unsigned int   baseOfCode;
    unsigned int   baseOfData;
    unsigned int   imageBase;
    unsigned int   sectionAlignment;
    unsigned int   fileAlignment;
    unsigned short majorOSVersion;
    unsigned short minorOSVersion;
    unsigned short majorImageVersion;
    unsigned short minorImageVersion;
    unsigned short majorSubsystemVersion;
    unsigned short minorSubsystemVersion;
    unsigned int   win32VersionValue;
    unsigned int   sizeOfImage;
    unsigned int   sizeOfHeaders;
    unsigned int   checksum;
    unsigned short subsystem;
    unsigned short dllCharacteristics;
    unsigned int   sizeOfStackReserve;
    unsigned int   sizeOfStackCommit;
    unsigned int   sizeOfHeapReserve;
    unsigned int   sizeOfHeapCommit;
    unsigned int   loaderFlags;
    unsigned int   numberOfRvaAndSizes;
    PortableExecutableDataDirectory dataDirectories[16];

    const PortableExecutableDataDirectory& GetDataDirectory(int index) const {
        return dataDirectories[index];
    }
};

struct PortableExecutableNTHeaders {
    unsigned int signature;   // 'PE\0\0' = 0x00004550
    PortableExecutableFileHeader fileHeader;
    PortableExecutableOptionalHeader optionalHeader;

    unsigned int Signature() const { return signature; }
    const PortableExecutableFileHeader& GetFileHeader() const { return fileHeader; }
    const PortableExecutableOptionalHeader& GetOptionalHeader() const { return optionalHeader; }
};

struct PortableExecutableSectionHeader {
    char name[8];
    unsigned int virtualSize;
    unsigned int virtualAddress;
    unsigned int sizeOfRawData;
    unsigned int pointerToRawData;
    unsigned int pointerToRelocations;
    unsigned int pointerToLinenumbers;
    unsigned short numberOfRelocations;
    unsigned short numberOfLinenumbers;
    unsigned int characteristics;

    unsigned int VirtualAddress() const { return virtualAddress; }
    unsigned int VirtualSize() const { return virtualSize; }
    unsigned int RawDataPointer() const { return pointerToRawData; }
};

struct PortableExecutableResourceDirectory {
    unsigned int characteristics;
    unsigned int timestamp;
    unsigned short majorVersion;
    unsigned short minorVersion;
    unsigned short namedEntryCount;
    unsigned short idEntryCount;

    unsigned short NamedEntryCount() const { return namedEntryCount; }
    unsigned short IdEntryCount() const { return idEntryCount; }
    unsigned short TotalEntries() const { return namedEntryCount + idEntryCount; }
};

struct PortableExecutableResourceDirectoryEntry {
    unsigned int nameOrId;
    unsigned int offsetToData;

    bool IsNamed() const { return (nameOrId & 0x80000000) != 0; }
    unsigned int GetId() const { return nameOrId & 0x7FFFFFFF; }
    unsigned int GetNameOffset() const { return nameOrId & 0x7FFFFFFF; }
    bool IsDirectory() const { return (offsetToData & 0x80000000) != 0; }
    unsigned int GetOffsetToData() const { return offsetToData & 0x7FFFFFFF; }
};

struct PortableExecutableResourceDataEntry {
    unsigned int dataRva;
    unsigned int size;
    unsigned int codepage;
    unsigned int reserved;

    unsigned int DataRva() const { return dataRva; }
    unsigned int Size() const { return size; }
};

/******************************************************************************/
/*    Icon File Format Structures                                              */
/******************************************************************************/

struct IconDirectory {
    unsigned short reserved;    // Always 0
    unsigned short type;        // 1=ICO, 2=CUR
    unsigned short count;       // Number of images

    unsigned short Type() const { return type; }
    unsigned short Count() const { return count; }
};

struct IconDirectoryHeader {
    unsigned char  width;       // 0 means 256
    unsigned char  height;      // 0 means 256
    unsigned char  colorCount;  // Number of colors
    unsigned char  reserved;
    unsigned short planes;
    unsigned short bitCount;
    unsigned int   size;        // Size of image data

    int Width() const { return width ? width : 256; }
    int Height() const { return height ? height : 256; }
    unsigned short BitCount() const { return bitCount; }
    unsigned int Size() const { return size; }
};

struct IconDirectoryEntry {
    IconDirectoryHeader header;
    unsigned int offset;        // Offset to image data in file

    int Width() const { return header.Width(); }
    int Height() const { return header.Height(); }
    unsigned short BitCount() const { return header.BitCount(); }
    unsigned int Size() const { return header.Size(); }
    unsigned int Offset() const { return offset; }
};

struct GroupIconDirectoryEntry {
    IconDirectoryHeader header;
    unsigned short identifier; // Resource ID

    int Width() const { return header.Width(); }
    int Height() const { return header.Height(); }
    unsigned short BitCount() const { return header.BitCount(); }
    unsigned int Size() const { return header.Size(); }
    unsigned short Identifier() const { return identifier; }
};

/******************************************************************************/
/*    NE (New Executable) Format Structures for FON file parsing              */
/******************************************************************************/

struct NeHeader {
    unsigned short signature;           // 'NE' = 0x454E
    unsigned char  majorLinkerVersion;
    unsigned char  minorLinkerVersion;
    unsigned short entryTableOffset;
    unsigned short entryTableLength;
    unsigned int   fileCRC;
    unsigned short flags;
    unsigned short autoDataSegment;
    unsigned short initialHeapSize;
    unsigned short initialStackSize;
    unsigned short csIp;
    unsigned short sssSp;
    unsigned short segmentTableEntries;
    unsigned short moduleReferenceEntries;
    unsigned short nonResidentNameTableSize;
    unsigned short segmentTableOffset;
    unsigned short resourceTableOffset;
    unsigned short residentNameTableOffset;
    unsigned short moduleReferenceTableOffset;
    unsigned short importedNamesTableOffset;
    unsigned int   nonResidentNameTableOffset;
    unsigned short movableEntryPoints;
    unsigned short alignmentShiftCount;     // Shift count for resource alignment
    unsigned short resourceSegments;
    unsigned char  targetOS;
    unsigned char  otherFlags;
    unsigned short fastLoadAreaOffset;
    unsigned short fastLoadAreaLength;
    unsigned short reserved;
    unsigned short windowsVersion;

    unsigned short Signature() const { return signature; }
    unsigned short ResourceTableOffset() const { return resourceTableOffset; }
    unsigned short AlignmentShiftCount() const { return alignmentShiftCount; }
};

struct NeResourceTypeInfo {
    unsigned short typeId;              // Type ID (0x8008 = RT_FONT)
    unsigned short count;               // Number of resources of this type
    unsigned int   reserved;

    unsigned short TypeId() const { return typeId; }
    unsigned short Count() const { return count; }
};

struct NeResourceNameInfo {
    unsigned short offset;              // Offset to resource data (shifted)
    unsigned short length;              // Length of resource data (shifted)
    unsigned short flags;
    unsigned short id;                  // Resource ID
    unsigned int   reserved;

    unsigned short Offset() const { return offset; }
    unsigned short Length() const { return length; }
    unsigned short Id() const { return id; }
};

/******************************************************************************/
/*    FNT Font Header Structure (Windows 2.0/3.0 bitmap font format)          */
/******************************************************************************/

struct FntHeader {
    unsigned short dfVersion;           // 0x0200 or 0x0300
    unsigned int   dfSize;              // Total file size
    char           dfCopyright[60];     // Copyright string
    unsigned short dfType;              // 0 = raster, 1 = vector
    unsigned short dfPoints;            // Nominal point size
    unsigned short dfVertRes;           // Vertical resolution (dpi)
    unsigned short dfHorizRes;          // Horizontal resolution (dpi)
    unsigned short dfAscent;            // Ascent (baseline to top)
    unsigned short dfInternalLeading;   // Internal leading
    unsigned short dfExternalLeading;   // External leading
    unsigned char  dfItalic;            // 1 if italic
    unsigned char  dfUnderline;         // 1 if underline
    unsigned char  dfStrikeOut;         // 1 if strikeout
    unsigned short dfWeight;            // Weight (400=regular, 700=bold)
    unsigned char  dfCharSet;           // Character set (0=ANSI, 255=OEM)
    unsigned short dfPixWidth;          // Pixel width (0 = variable)
    unsigned short dfPixHeight;         // Pixel height
    unsigned char  dfPitchAndFamily;    // Pitch and family
    unsigned short dfAvgWidth;          // Average character width
    unsigned short dfMaxWidth;          // Maximum character width
    unsigned char  dfFirstChar;         // First character code
    unsigned char  dfLastChar;          // Last character code
    unsigned char  dfDefaultChar;       // Default character (relative to dfFirstChar)
    unsigned char  dfBreakChar;         // Break character (relative to dfFirstChar)
    unsigned short dfWidthBytes;        // Bytes per row (stride)
    unsigned int   dfDevice;            // Offset to device name
    unsigned int   dfFace;              // Offset to face name
    unsigned int   dfBitsPointer;       // Absolute pointer to bitmap data
    unsigned int   dfBitsOffset;        // Offset to bitmap data
    unsigned char  dfReserved;          // Reserved (V2.0)
    // V3.0 extensions follow...

    unsigned short Version() const { return dfVersion; }
    unsigned short Points() const { return dfPoints; }
    unsigned short PixHeight() const { return dfPixHeight; }
    unsigned short PixWidth() const { return dfPixWidth; }
    unsigned short Ascent() const { return dfAscent; }
    unsigned char  FirstChar() const { return dfFirstChar; }
    unsigned char  LastChar() const { return dfLastChar; }
    unsigned short AvgWidth() const { return dfAvgWidth; }
    unsigned short MaxWidth() const { return dfMaxWidth; }
    unsigned short Weight() const { return dfWeight; }
    unsigned char  Italic() const { return dfItalic; }
};

// Character width table entry
// V2.0: 4 bytes per entry (width: WORD, offset: WORD)
// V3.0: 6 bytes per entry (width: WORD, offset: DWORD)
struct FntCharEntryV2 {
    unsigned short width;               // Character width in pixels
    unsigned short offset;              // Offset to glyph bitmap (from start of FNT)
};

struct FntCharEntryV3 {
    unsigned short width;               // Character width in pixels
    unsigned int offset;                // Offset to glyph bitmap (from start of FNT)
};

#pragma pack(pop)

/******************************************************************************/
/*    Helper: Decode icon DIB data to 32-bit ARGB image                        */
/******************************************************************************/

static void DecodeIconDIB(const unsigned char* iconData, Int32 targetSize, Image& result)
{
    const BitmapInfoHeader* bmpHeader = reinterpret_cast<const BitmapInfoHeader*>(iconData);
    Int32 width = Int32(static_cast<int>(bmpHeader->Width()));
    Int32 height = Int32(static_cast<int>(bmpHeader->Height()) / 2);  // DIB height includes mask
    Int32 bitCount = Int32(static_cast<int>(bmpHeader->BitCount()));

    if (static_cast<int>(width) != static_cast<int>(targetSize) || static_cast<int>(height) != static_cast<int>(targetSize))
    {
        throw InvalidDataException("Icon DIB dimensions don't match expected size.");
    }

    // Get palette (if any)
    UInt32 paletteCount = UInt32(static_cast<unsigned int>(bmpHeader->UsedColors()));
    if (static_cast<unsigned int>(paletteCount) == 0 && static_cast<int>(bitCount) <= 8)
    {
        paletteCount = UInt32(1u << static_cast<int>(bitCount));
    }

    const unsigned char* paletteData = iconData + static_cast<unsigned int>(bmpHeader->HeaderSize());
    const unsigned char* xorMask = paletteData + static_cast<unsigned int>(paletteCount) * 4;

    // Calculate strides
    Int32 xorStride = Int32(((static_cast<int>(bitCount) * static_cast<int>(width) + 31) / 32) * 4);
    Int32 andStride = Int32(((static_cast<int>(width) + 31) / 32) * 4);
    const unsigned char* andMask = xorMask + static_cast<int>(xorStride) * static_cast<int>(height);

    unsigned int* outPixels = result.Data();

    // Decode based on bit depth
    if (static_cast<int>(bitCount) == 32)
    {
        // 32bpp BGRA with alpha
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                unsigned char b = row[static_cast<int>(x) * 4 + 0];
                unsigned char g = row[static_cast<int>(x) * 4 + 1];
                unsigned char r = row[static_cast<int>(x) * 4 + 2];
                unsigned char a = row[static_cast<int>(x) * 4 + 3];
                outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] =
                    (static_cast<unsigned int>(a) << 24) |
                    (static_cast<unsigned int>(r) << 16) |
                    (static_cast<unsigned int>(g) << 8) |
                    static_cast<unsigned int>(b);
            }
        }
    } else if (static_cast<int>(bitCount) == 24)
    {
        // 24bpp RGB, use AND mask for transparency
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char b = row[static_cast<int>(x) * 3 + 0];
                    unsigned char g = row[static_cast<int>(x) * 3 + 1];
                    unsigned char r = row[static_cast<int>(x) * 3 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 8)
    {
        // 8bpp indexed
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char index = row[static_cast<int>(x)];
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 4)
    {
        // 4bpp indexed
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    unsigned char byteVal = row[static_cast<int>(x) / 2];
                    unsigned char index = ((static_cast<int>(x) & 1) == 0) ? (byteVal >> 4) & 0x0F : byteVal & 0x0F;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    } else if (static_cast<int>(bitCount) == 1)
    {
        // 1bpp monochrome
        for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(height); y += 1)
        {
            const unsigned char* row = xorMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(xorStride);
            const unsigned char* maskRow = andMask + (static_cast<int>(height) - 1 - static_cast<int>(y)) * static_cast<int>(andStride);
            for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(width); x += 1)
            {
                Boolean transparent = Boolean(((maskRow[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                if (static_cast<bool>(transparent))
                {
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0x00000000;
                }
                else
                {
                    Boolean pixel = Boolean(((row[static_cast<int>(x) / 8] >> (7 - (static_cast<int>(x) & 7))) & 1) != 0);
                    unsigned char index = static_cast<bool>(pixel) ? 1 : 0;
                    unsigned char b = paletteData[index * 4 + 0];
                    unsigned char g = paletteData[index * 4 + 1];
                    unsigned char r = paletteData[index * 4 + 2];
                    outPixels[static_cast<int>(y) * static_cast<int>(width) + static_cast<int>(x)] = 0xFF000000 |
                        (static_cast<unsigned int>(r) << 16) |
                        (static_cast<unsigned int>(g) << 8) |
                        static_cast<unsigned int>(b);
                }
            }
        }
    }
    else
    {
        throw InvalidDataException("Unsupported icon bit depth.");
    }
}

/******************************************************************************/
/*    Image::FromIcon - Load icon from standalone .ico file                    */
/******************************************************************************/

Image Image::FromIcon(const char* path, const Size& size)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = Int32(static_cast<int>(size.width));
    if (static_cast<int>(targetSize) != 16 && static_cast<int>(targetSize) != 24 && static_cast<int>(targetSize) != 32 && static_cast<int>(targetSize) != 48)
    {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < static_cast<int>(sizeof(IconDirectory)))
    {
        throw InvalidDataException("File is too small to be a valid ICO.");
    }

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    const IconDirectory* dir = reinterpret_cast<const IconDirectory*>(fileData);
    if (dir->Type() != 1 || dir->Count() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("Invalid ICO file format.");
    }

    const IconDirectoryEntry* entries = reinterpret_cast<const IconDirectoryEntry*>(
        fileData + sizeof(IconDirectory));

    // Find matching size
    const IconDirectoryEntry* chosen = nullptr;
    for (UInt16 i = UInt16(0); static_cast<unsigned short>(i) < dir->Count(); i += 1)
    {
        if (entries[static_cast<unsigned short>(i)].Width() == static_cast<int>(targetSize) && entries[static_cast<unsigned short>(i)].Height() == static_cast<int>(targetSize))
        {
            chosen = &entries[static_cast<unsigned short>(i)];
            break;
        }
    }

    if (!chosen)
    {
        std::free(fileData);
        throw InvalidDataException("Requested icon size not found in file.");
    }

    Image result(targetSize, targetSize);
    const unsigned char* iconData = fileData + chosen->Offset();
    DecodeIconDIB(iconData, targetSize, result);

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    Image::FromIconLibrary - Load icon from PE-based icon library            */
/******************************************************************************/

Image Image::FromIconLibrary(const char* path, Int32 iconIndex, const Size& size)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_ICON = UInt32(3);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = Int32(static_cast<int>(size.width));
    if (static_cast<int>(targetSize) != 16 && static_cast<int>(targetSize) != 24 && static_cast<int>(targetSize) != 32 && static_cast<int>(targetSize) != 48)
    {
        throw ArgumentException("Icon size must be 16, 24, 32, or 48 pixels.");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("No resource section in file.");
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        throw InvalidDataException("Resource section not found.");
    }

    UInt32 rsrcRva = UInt32(rsrcSection->VirtualAddress());
    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - static_cast<unsigned int>(rsrcRva));

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDirectoryEntry* groupIconEntry = nullptr;
    const PortableExecutableResourceDirectoryEntry* iconEntry = nullptr;

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed())
        {
            if (rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
            {
                groupIconEntry = &rootEntries[static_cast<int>(i)];
            } else if (rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_ICON))
            {
                iconEntry = &rootEntries[static_cast<int>(i)];
            }
        }
    }

    if (!groupIconEntry || !iconEntry)
    {
        std::free(fileData);
        throw InvalidDataException("No icon resources found.");
    }

    // Navigate to the RT_GROUP_ICON directory
    const PortableExecutableResourceDirectory* groupIconDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + groupIconEntry->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* groupIconEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(groupIconDir) + sizeof(PortableExecutableResourceDirectory));

    Int32 idx = iconIndex;
    if (static_cast<int>(idx) < 0 || static_cast<int>(idx) >= groupIconDir->TotalEntries())
    {
        std::free(fileData);
        throw ArgumentException("Icon index out of range.");
    }

    // Get the specific icon group
    const PortableExecutableResourceDirectoryEntry* chosenGroup = &groupIconEntries[static_cast<int>(idx)];
    if (!chosenGroup->IsDirectory())
    {
        std::free(fileData);
        throw InvalidDataException("Invalid icon group entry.");
    }

    // Navigate to language level
    const PortableExecutableResourceDirectory* langDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + chosenGroup->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* langEntry =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(langDir) + sizeof(PortableExecutableResourceDirectory));

    if (langEntry->IsDirectory())
    {
        std::free(fileData);
        throw InvalidDataException("Invalid icon resource structure.");
    }

    // Get the data entry
    const PortableExecutableResourceDataEntry* dataEntry =
        reinterpret_cast<const PortableExecutableResourceDataEntry*>(
            rsrcBase + langEntry->GetOffsetToData());

    const unsigned char* groupData = fileData + static_cast<unsigned int>(rsrcOffset) + (dataEntry->DataRva() - static_cast<unsigned int>(rsrcRva));

    // Parse the GROUP_ICON directory
    const IconDirectory* iconDir = reinterpret_cast<const IconDirectory*>(groupData);
    if (iconDir->Type() != 1 || iconDir->Count() == 0)
    {
        std::free(fileData);
        throw InvalidDataException("Invalid GROUP_ICON format.");
    }

    const GroupIconDirectoryEntry* groupEntries =
        reinterpret_cast<const GroupIconDirectoryEntry*>(groupData + sizeof(IconDirectory));

    // Find matching size
    const GroupIconDirectoryEntry* chosenIcon = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < iconDir->Count(); i += 1)
    {
        if (groupEntries[static_cast<int>(i)].Width() == static_cast<int>(targetSize) && groupEntries[static_cast<int>(i)].Height() == static_cast<int>(targetSize))
        {
            chosenIcon = &groupEntries[static_cast<int>(i)];
            break;
        }
    }

    if (!chosenIcon)
    {
        std::free(fileData);
        throw InvalidDataException("Requested icon size not found.");
    }

    // Now find the RT_ICON with matching identifier
    const PortableExecutableResourceDirectory* iconTypeDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + iconEntry->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* iconTypeEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(iconTypeDir) + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDirectoryEntry* matchingIcon = nullptr;
    Int32 totalIconEntries = Int32(iconTypeDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalIconEntries); i += 1)
    {
        if (!iconTypeEntries[static_cast<int>(i)].IsNamed() && iconTypeEntries[static_cast<int>(i)].GetId() == chosenIcon->Identifier())
        {
            matchingIcon = &iconTypeEntries[static_cast<int>(i)];
            break;
        }
    }

    if (!matchingIcon)
    {
        std::free(fileData);
        throw InvalidDataException("Icon resource not found.");
    }

    // Navigate to language level for the icon
    const PortableExecutableResourceDirectory* iconLangDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(
            rsrcBase + matchingIcon->GetOffsetToData());
    const PortableExecutableResourceDirectoryEntry* iconLangEntry =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            reinterpret_cast<const unsigned char*>(iconLangDir) + sizeof(PortableExecutableResourceDirectory));

    const PortableExecutableResourceDataEntry* iconDataEntry =
        reinterpret_cast<const PortableExecutableResourceDataEntry*>(
            rsrcBase + iconLangEntry->GetOffsetToData());

    const unsigned char* iconData = fileData + static_cast<unsigned int>(rsrcOffset) + (iconDataEntry->DataRva() - static_cast<unsigned int>(rsrcRva));

    Image result(targetSize, targetSize);
    DecodeIconDIB(iconData, targetSize, result);

    std::free(fileData);
    return result;
}

/******************************************************************************/
/*    Image::GetIconLibraryCount - Get number of icon groups in library        */
/******************************************************************************/

Int32 Image::GetIconLibraryCount(const char* path)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    // Read file using File API
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    // Copy to raw buffer for processing
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    // Parse PE header
    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid PE signature.");
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        return Int32(0);  // No resources
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        return Int32(0);
    }

    UInt32 rsrcRva = UInt32(rsrcSection->VirtualAddress());
    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - static_cast<unsigned int>(rsrcRva));

    // Parse resource directory - find RT_GROUP_ICON
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed() && rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
        {
            // Found RT_GROUP_ICON, count entries
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[static_cast<int>(i)].GetOffsetToData());
            Int32 count = Int32(groupIconDir->TotalEntries());
            std::free(fileData);
            return count;
        }
    }

    std::free(fileData);
    return Int32(0);
}

/******************************************************************************/
/*    Image::GetIconLibraryNames - Get names of icons in PE library            */
/******************************************************************************/

// Helper: Read PE resource name (UTF-16LE length-prefixed string)
// Returns empty string for unnamed resources or on error
static String ReadResourceName(const unsigned char* rsrcBase, UInt32 nameOffset)
{
    // Name format: WORD length (characters), followed by UTF-16LE chars (no null)
    const unsigned char* namePtr = rsrcBase + static_cast<unsigned int>(nameOffset);
    UInt16 charCount = UInt16(*reinterpret_cast<const unsigned short*>(namePtr));

    if (static_cast<unsigned short>(charCount) == 0 || static_cast<unsigned short>(charCount) > 256)
    {
        return String();  // Empty or suspiciously long
    }

    // Convert UTF-16LE to ASCII (simple conversion, ignore high bytes)
    char* asciiName = static_cast<char*>(std::malloc(static_cast<unsigned short>(charCount) + 1));
    if (!asciiName) return String();

    const unsigned short* utf16Chars = reinterpret_cast<const unsigned short*>(namePtr + 2);
    for (UInt16 i = UInt16(0); static_cast<unsigned short>(i) < static_cast<unsigned short>(charCount); i += 1)
    {
        // Simple conversion - just take low byte (works for ASCII names)
        asciiName[static_cast<unsigned short>(i)] = static_cast<char>(utf16Chars[static_cast<unsigned short>(i)] & 0xFF);
    }
    asciiName[static_cast<unsigned short>(charCount)] = '\0';

    String result(asciiName);
    std::free(asciiName);
    return result;
}

Array<String> Image::GetIconLibraryNames(const char* path)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt32 PE_SIGNATURE = UInt32(0x00004550);
    const UInt32 RT_GROUP_ICON = UInt32(14);

    if (!path || path[0] == '\0')
    {
        return Array<String>(0);
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData) return Array<String>(0);

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse DOS/PE headers
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        return Array<String>(0);
    }

    const PortableExecutableNTHeaders* peHeaders = reinterpret_cast<const PortableExecutableNTHeaders*>(
        fileData + dosHeader->NewHeaderOffset());
    if (peHeaders->Signature() != static_cast<unsigned int>(PE_SIGNATURE))
    {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find resource directory
    const PortableExecutableDataDirectory& rsrcDir = peHeaders->GetOptionalHeader().GetDataDirectory(2);
    if (rsrcDir.VirtualAddress() == 0)
    {
        std::free(fileData);
        return Array<String>(0);
    }

    // Find .rsrc section
    const PortableExecutableSectionHeader* sections = reinterpret_cast<const PortableExecutableSectionHeader*>(
        fileData + dosHeader->NewHeaderOffset() + sizeof(unsigned int) +
        sizeof(PortableExecutableFileHeader) + peHeaders->GetFileHeader().OptionalHeaderSize());

    const PortableExecutableSectionHeader* rsrcSection = nullptr;
    for (Int32 i = Int32(0); static_cast<int>(i) < peHeaders->GetFileHeader().SectionCount(); i += 1)
    {
        if (rsrcDir.VirtualAddress() >= sections[static_cast<int>(i)].VirtualAddress() &&
            rsrcDir.VirtualAddress() < sections[static_cast<int>(i)].VirtualAddress() + sections[static_cast<int>(i)].VirtualSize())
            {
            rsrcSection = &sections[static_cast<int>(i)];
            break;
        }
    }

    if (!rsrcSection)
    {
        std::free(fileData);
        return Array<String>(0);
    }

    UInt32 rsrcOffset = UInt32(rsrcSection->RawDataPointer());
    const unsigned char* rsrcBase = fileData + static_cast<unsigned int>(rsrcOffset) + (rsrcDir.VirtualAddress() - rsrcSection->VirtualAddress());

    // Find RT_GROUP_ICON directory
    const PortableExecutableResourceDirectory* rootDir =
        reinterpret_cast<const PortableExecutableResourceDirectory*>(rsrcBase);
    const PortableExecutableResourceDirectoryEntry* rootEntries =
        reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
            rsrcBase + sizeof(PortableExecutableResourceDirectory));

    Int32 totalRootEntries = Int32(rootDir->TotalEntries());
    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(totalRootEntries); i += 1)
    {
        if (!rootEntries[static_cast<int>(i)].IsNamed() && rootEntries[static_cast<int>(i)].GetId() == static_cast<unsigned int>(RT_GROUP_ICON))
        {
            // Found RT_GROUP_ICON, get icon names
            const PortableExecutableResourceDirectory* groupIconDir =
                reinterpret_cast<const PortableExecutableResourceDirectory*>(
                    rsrcBase + rootEntries[static_cast<int>(i)].GetOffsetToData());
            const PortableExecutableResourceDirectoryEntry* iconEntries =
                reinterpret_cast<const PortableExecutableResourceDirectoryEntry*>(
                    reinterpret_cast<const unsigned char*>(groupIconDir) + sizeof(PortableExecutableResourceDirectory));

            Int32 count = Int32(groupIconDir->TotalEntries());
            Array<String> names(count);

            for (Int32 j = Int32(0); static_cast<int>(j) < static_cast<int>(count); j += 1)
            {
                if (iconEntries[static_cast<int>(j)].IsNamed())
                {
                    names[static_cast<int>(j)] = ReadResourceName(rsrcBase, UInt32(iconEntries[static_cast<int>(j)].GetNameOffset()));
                }
                else
                {
                    // ID-based entry - return empty string or the ID as string
                    names[static_cast<int>(j)] = String();
                }
            }

            std::free(fileData);
            return names;
        }
    }

    std::free(fileData);
    return Array<String>(0);
}

/******************************************************************************/
/*    Image::GetIconLibraryIndex - Find icon index by name                     */
/******************************************************************************/

Int32 Image::GetIconLibraryIndex(const char* path, const char* iconName)
{
    if (!path || !iconName || iconName[0] == '\0')
    {
        return Int32(-1);
    }

    Array<String> names = GetIconLibraryNames(path);
    String target(iconName);

    for (Int32 i = Int32(0); static_cast<int>(i) < names.Length(); i += 1)
    {
        if (names[static_cast<int>(i)].EqualsIgnoreCase(target))
        {
            return i;
        }
    }

    return Int32(-1);
}

/******************************************************************************/
/*    Image::FromIconLibrary (by name) - Load icon by name                     */
/******************************************************************************/

Image Image::FromIconLibrary(const char* path, const char* iconName, const Size& size)
{
    if (!iconName || iconName[0] == '\0')
    {
        throw ArgumentNullException("iconName");
    }

    Int32 index = GetIconLibraryIndex(path, iconName);
    if (static_cast<int>(index) < 0)
    {
        throw ArgumentException("Icon not found in library.");
    }

    return FromIconLibrary(path, index, size);
}

/******************************************************************************/
/*    Image::FromFile - Load image from file (auto-detect format)              */
/******************************************************************************/

Image Image::FromFile(const char* path)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    if (!IO::File::Exists(path))
    {
        throw System::FileNotFoundException(path);
    }

    // Check file extension to decide loader
    const char* ext = path;
    const char* p = path;
    while (*p)
    {
        if (*p == '.') ext = p;
        p++;
    }

    // Use native BMP loader for .bmp files
    if ((ext[1] == 'b' || ext[1] == 'B') &&
        (ext[2] == 'm' || ext[2] == 'M') &&
        (ext[3] == 'p' || ext[3] == 'P') &&
        ext[4] == '\0')
        {
        return FromBitmap(path);
    }

    // Use stb_image for PNG, JPEG, GIF, TGA, PSD, etc.
    Array<UInt8> fileData = IO::File::ReadAllBytes(path);
    int width, height, channels;

    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(&fileData[0]),
        fileData.Length(),
        &width, &height, &channels,
        4  // Request RGBA output
    );

    if (!pixels)
    {
        throw InvalidDataException("Failed to decode image file.");
    }

    // Create image and copy pixels
    Image img = Image(Int32(width), Int32(height));

    // stb_image returns RGBA, we need ARGB
    unsigned int* dest = img.Data();
    for (Int32 i = Int32(0); static_cast<int>(i) < width * height; i += 1)
    {
        unsigned char r = pixels[static_cast<int>(i) * 4 + 0];
        unsigned char g = pixels[static_cast<int>(i) * 4 + 1];
        unsigned char b = pixels[static_cast<int>(i) * 4 + 2];
        unsigned char a = pixels[static_cast<int>(i) * 4 + 3];
        dest[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) |
                  (static_cast<unsigned int>(r) << 16) |
                  (static_cast<unsigned int>(g) << 8) |
                  static_cast<unsigned int>(b);
    }

    stbi_image_free(pixels);
    return img;
}

/******************************************************************************/
/*    Image::FromPng - Load PNG image from file                                */
/******************************************************************************/

Image Image::FromPng(const char* path)
{
    // FromFile handles PNG via stb_image
    return FromFile(path);
}

/******************************************************************************/
/*    Image::FromJpeg - Load JPEG image from file                              */
/******************************************************************************/

Image Image::FromJpeg(const char* path)
{
    // FromFile handles JPEG via stb_image
    return FromFile(path);
}

/******************************************************************************/
/*    Image::ScaleTo - Scale image using bilinear interpolation                */
/******************************************************************************/

Image Image::ScaleTo(Int32 newWidth, Int32 newHeight) const {
    Int32 nw = newWidth;
    Int32 nh = newHeight;

    if (static_cast<int>(nw) <= 0 || static_cast<int>(nh) <= 0)
    {
        throw ArgumentException("New dimensions must be positive");
    }

    if (_width == 0 || _height == 0 || _data == nullptr)
    {
        return Image(newWidth, newHeight, Color::Black);
    }

    Image result = Image(newWidth, newHeight);
    unsigned int* dest = result.Data();
    const unsigned int* src = _data;

    // Fixed-point scaling factors (16.16 format)
    Int32 scaleX = Int32((_width << 16) / static_cast<int>(nw));
    Int32 scaleY = Int32((_height << 16) / static_cast<int>(nh));

    for (Int32 y = Int32(0); static_cast<int>(y) < static_cast<int>(nh); y += 1)
    {
        Int32 srcY = Int32((static_cast<int>(y) * static_cast<int>(scaleY)) >> 16);
        Int32 fracY = Int32((static_cast<int>(y) * static_cast<int>(scaleY)) & 0xFFFF);

        // Clamp source Y
        if (static_cast<int>(srcY) >= _height - 1)
        {
            srcY = Int32(_height - 2);
            fracY = Int32(0xFFFF);
        }
        if (static_cast<int>(srcY) < 0) srcY = Int32(0);

        for (Int32 x = Int32(0); static_cast<int>(x) < static_cast<int>(nw); x += 1)
        {
            Int32 srcX = Int32((static_cast<int>(x) * static_cast<int>(scaleX)) >> 16);
            Int32 fracX = Int32((static_cast<int>(x) * static_cast<int>(scaleX)) & 0xFFFF);

            // Clamp source X
            if (static_cast<int>(srcX) >= _width - 1)
            {
                srcX = Int32(_width - 2);
                fracX = Int32(0xFFFF);
            }
            if (static_cast<int>(srcX) < 0) srcX = Int32(0);

            // Get 4 neighboring pixels
            UInt32 p00 = UInt32(src[static_cast<int>(srcY) * _width + static_cast<int>(srcX)]);
            UInt32 p10 = UInt32(src[static_cast<int>(srcY) * _width + static_cast<int>(srcX) + 1]);
            UInt32 p01 = UInt32(src[(static_cast<int>(srcY) + 1) * _width + static_cast<int>(srcX)]);
            UInt32 p11 = UInt32(src[(static_cast<int>(srcY) + 1) * _width + static_cast<int>(srcX) + 1]);

            // Bilinear interpolation for each channel
            Int32 fx = Int32(static_cast<int>(fracX) >> 8);  // 0-255
            Int32 fy = Int32(static_cast<int>(fracY) >> 8);  // 0-255
            Int32 fx1 = Int32(256 - static_cast<int>(fx));
            Int32 fy1 = Int32(256 - static_cast<int>(fy));

            // Alpha channel
            Int32 a00 = Int32((static_cast<unsigned int>(p00) >> 24) & 0xFF);
            Int32 a10 = Int32((static_cast<unsigned int>(p10) >> 24) & 0xFF);
            Int32 a01 = Int32((static_cast<unsigned int>(p01) >> 24) & 0xFF);
            Int32 a11 = Int32((static_cast<unsigned int>(p11) >> 24) & 0xFF);
            Int32 a = Int32(((static_cast<int>(a00) * static_cast<int>(fx1) + static_cast<int>(a10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(a01) * static_cast<int>(fx1) + static_cast<int>(a11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Red channel
            Int32 r00 = Int32((static_cast<unsigned int>(p00) >> 16) & 0xFF);
            Int32 r10 = Int32((static_cast<unsigned int>(p10) >> 16) & 0xFF);
            Int32 r01 = Int32((static_cast<unsigned int>(p01) >> 16) & 0xFF);
            Int32 r11 = Int32((static_cast<unsigned int>(p11) >> 16) & 0xFF);
            Int32 r = Int32(((static_cast<int>(r00) * static_cast<int>(fx1) + static_cast<int>(r10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(r01) * static_cast<int>(fx1) + static_cast<int>(r11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Green channel
            Int32 g00 = Int32((static_cast<unsigned int>(p00) >> 8) & 0xFF);
            Int32 g10 = Int32((static_cast<unsigned int>(p10) >> 8) & 0xFF);
            Int32 g01 = Int32((static_cast<unsigned int>(p01) >> 8) & 0xFF);
            Int32 g11 = Int32((static_cast<unsigned int>(p11) >> 8) & 0xFF);
            Int32 g = Int32(((static_cast<int>(g00) * static_cast<int>(fx1) + static_cast<int>(g10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(g01) * static_cast<int>(fx1) + static_cast<int>(g11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            // Blue channel
            Int32 b00 = Int32(static_cast<unsigned int>(p00) & 0xFF);
            Int32 b10 = Int32(static_cast<unsigned int>(p10) & 0xFF);
            Int32 b01 = Int32(static_cast<unsigned int>(p01) & 0xFF);
            Int32 b11 = Int32(static_cast<unsigned int>(p11) & 0xFF);
            Int32 b = Int32(((static_cast<int>(b00) * static_cast<int>(fx1) + static_cast<int>(b10) * static_cast<int>(fx)) * static_cast<int>(fy1) + (static_cast<int>(b01) * static_cast<int>(fx1) + static_cast<int>(b11) * static_cast<int>(fx)) * static_cast<int>(fy)) >> 16);

            dest[static_cast<int>(y) * static_cast<int>(nw) + static_cast<int>(x)] = (static_cast<unsigned int>(a) << 24) |
                               (static_cast<unsigned int>(r) << 16) |
                               (static_cast<unsigned int>(g) << 8) |
                               static_cast<unsigned int>(b);
        }
    }

    return result;
}

Image Image::ScaleTo(const Size& newSize) const {
    return ScaleTo(newSize.width, newSize.height);
}

} // namespace System::Drawing
