/// @file Font.cpp
/// @brief Implementation of System.Drawing.Font class.

#include "Font.hpp"
#include "Image.hpp"
#include "../Exception.hpp"
#include "../IO/File.hpp"
#include "../../ThirdParty/stb_truetype.h"
#include <cstdlib>
#include <cstring>

namespace System::Drawing
{

/******************************************************************************/
/*    MZ/DOS Executable Header (needed for FON file parsing)                  */
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

#pragma pack(pop)

/******************************************************************************/
/*    NE (New Executable) Format Structures for FON file parsing              */
/******************************************************************************/

#pragma pack(push, 1)

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
/*    Font::FontData - Internal font data storage                              */
/******************************************************************************/

struct Font::FontData {
    String name;                    // Font face name
    Int32 pointSize;                // Nominal point size
    Int32 pixelHeight;              // Actual pixel height
    Int32 ascent;                   // Pixels above baseline
    FontStyle style;                // Font style flags
    Int32 firstChar;                // First character code
    Int32 lastChar;                 // Last character code
    Boolean isTrueType;             // True if TTF, false if FON

    // Character widths (256 entries, 0 for non-existent chars)
    unsigned short charWidths[256];

    // FON: Glyph offsets into bitmap data
    unsigned int charOffsets[256];

    // Raw font file data (FON bitmap or TTF file)
    unsigned char* bitmapData;
    unsigned int bitmapSize;

    // TTF: stbtt font info
    stbtt_fontinfo ttfInfo;
    Float32 ttfScale;               // Scale factor for pixel height

    // Glyph cache (lazily populated)
    mutable Image glyphCache[256];
    mutable Boolean glyphCached[256];

    FontData()
        : pointSize(Int32(0)), pixelHeight(Int32(0)), ascent(Int32(0)), style(FontStyle::Regular)
        , firstChar(Int32(0)), lastChar(Int32(0)), isTrueType(Boolean(false))
        , bitmapData(nullptr), bitmapSize(0), ttfScale(Float32(0.0f))
        {
        std::memset(charWidths, 0, sizeof(charWidths));
        std::memset(charOffsets, 0, sizeof(charOffsets));
        // Initialize Boolean array element by element (can't use memset on non-trivial types)
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
        {
            glyphCached[static_cast<int>(i)] = Boolean(false);
        }
        std::memset(&ttfInfo, 0, sizeof(ttfInfo));
    }

    ~FontData()
    {
        if (bitmapData)
        {
            std::free(bitmapData);
            bitmapData = nullptr;
        }
    }

    // Render a glyph to the cache
    void RenderGlyph(Int32 ch) const {
        if (static_cast<int>(ch) < 0 || static_cast<int>(ch) > 255 || static_cast<bool>(glyphCached[static_cast<int>(ch)])) return;

        if (static_cast<bool>(isTrueType))
        {
            RenderTrueTypeGlyph(ch);
        }
        else
        {
            RenderFonGlyph(ch);
        }
    }

    // Render FON (bitmap) glyph
    void RenderFonGlyph(Int32 ch) const {
        if (static_cast<int>(ch) < static_cast<int>(firstChar) || static_cast<int>(ch) > static_cast<int>(lastChar))
        {
            // Character not in font - create empty glyph
            glyphCache[static_cast<int>(ch)] = Image(Int32(1), pixelHeight, Color::Transparent);
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        Int32 width = Int32(charWidths[static_cast<int>(ch)]);
        Int32 height = pixelHeight;
        if (static_cast<int>(width) <= 0)
        {
            glyphCache[static_cast<int>(ch)] = Image(Int32(1), height, Color::Transparent);
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        // Create transparent glyph image
        glyphCache[static_cast<int>(ch)] = Image(width, height, Color::Transparent);

        // FON bitmap format: column-major by byte-columns (per FreeType winfnt.c)
        // Each byte-column (8 horizontal pixels) is stored as 'height' sequential bytes
        // Source layout: src[byteCol * height + row]
        // Bits: MSB is leftmost pixel within each byte
        const unsigned char* src = bitmapData + charOffsets[static_cast<int>(ch)];

        for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(height); row += 1)
        {
            for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(width); col += 1)
            {
                Int32 byteCol = Int32(static_cast<int>(col) / 8);
                Int32 bitIndex = Int32(7 - (static_cast<int>(col) % 8));  // MSB is leftmost pixel
                // Column-major access: byteCol * height + row
                unsigned char byte = src[static_cast<int>(byteCol) * static_cast<int>(height) + static_cast<int>(row)];
                Boolean pixel = Boolean(((byte >> static_cast<int>(bitIndex)) & 1) != 0);
                if (static_cast<bool>(pixel))
                {
                    glyphCache[static_cast<int>(ch)].SetPixel(col, row, Color::White);
                }
            }
        }

        glyphCached[static_cast<int>(ch)] = Boolean(true);
    }

    // Render TrueType glyph using stb_truetype
    void RenderTrueTypeGlyph(Int32 ch) const {
        // Get horizontal metrics (advance and left side bearing)
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&ttfInfo, static_cast<int>(ch), &advanceWidth, &lsb);

        // Get bitmap bounding box
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&ttfInfo, static_cast<int>(ch), static_cast<float>(ttfScale), static_cast<float>(ttfScale), &x0, &y0, &x1, &y1);

        Int32 glyphWidth = Int32(x1 - x0);
        Int32 glyphHeight = Int32(y1 - y0);

        // Scale lsb to pixels
        Int32 lsbPixels = Int32(static_cast<int>(lsb * static_cast<float>(ttfScale) + 0.5f));

        // Use advance width for image width (for proper character spacing)
        Int32 imageWidth = Int32(charWidths[static_cast<int>(ch)]);
        if (static_cast<int>(imageWidth) <= 0) imageWidth = Int32(1);
        Int32 imageHeight = pixelHeight;

        // Create glyph image
        glyphCache[static_cast<int>(ch)] = Image(imageWidth, imageHeight, Color::Transparent);

        if (static_cast<int>(glyphWidth) <= 0 || static_cast<int>(glyphHeight) <= 0)
        {
            // Empty glyph (e.g., space) - just return the transparent image
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        // Rasterize the glyph (8-bit grayscale)
        unsigned char* bitmap = static_cast<unsigned char*>(
            std::malloc(static_cast<int>(glyphWidth) * static_cast<int>(glyphHeight)));
        if (!bitmap)
        {
            glyphCached[static_cast<int>(ch)] = Boolean(true);
            return;
        }

        stbtt_MakeCodepointBitmap(&ttfInfo, bitmap, static_cast<int>(glyphWidth), static_cast<int>(glyphHeight),
                                   static_cast<int>(glyphWidth), static_cast<float>(ttfScale), static_cast<float>(ttfScale), static_cast<int>(ch));

        // Position glyph in image using the working example's approach:
        // - Horizontally: use lsb (left side bearing) scaled to pixels
        // - Vertically: use ascent + y0 for baseline alignment

        // Copy bitmap to image with anti-aliasing (store grayscale as alpha)
        for (Int32 row = Int32(0); static_cast<int>(row) < static_cast<int>(glyphHeight); row += 1)
        {
            Int32 destY = Int32(static_cast<int>(ascent) + y0 + static_cast<int>(row));
            if (static_cast<int>(destY) < 0 || static_cast<int>(destY) >= static_cast<int>(imageHeight)) continue;

            for (Int32 col = Int32(0); static_cast<int>(col) < static_cast<int>(glyphWidth); col += 1)
            {
                Int32 destX = Int32(static_cast<int>(lsbPixels) + static_cast<int>(col));
                if (static_cast<int>(destX) < 0 || static_cast<int>(destX) >= static_cast<int>(imageWidth)) continue;

                unsigned char gray = bitmap[static_cast<int>(row) * static_cast<int>(glyphWidth) + static_cast<int>(col)];
                if (gray > 0)
                {
                    // Store grayscale as alpha for anti-aliasing
                    // White color with variable alpha
                    glyphCache[static_cast<int>(ch)].SetPixel(destX, destY, Color(255, 255, 255, gray));
                }
            }
        }

        std::free(bitmap);
        glyphCached[static_cast<int>(ch)] = Boolean(true);
    }
};

/******************************************************************************/
/*    Font implementation                                                      */
/******************************************************************************/

Font::Font() : _data(nullptr) {}

Font::Font(FontData* data) : _data(data) {}

Font::Font(const Font& other) : _data(nullptr)
{
    if (other._data)
    {
        _data = new FontData();
        _data->name = other._data->name;
        _data->pointSize = other._data->pointSize;
        _data->pixelHeight = other._data->pixelHeight;
        _data->ascent = other._data->ascent;
        _data->style = other._data->style;
        _data->firstChar = other._data->firstChar;
        _data->lastChar = other._data->lastChar;
        _data->isTrueType = other._data->isTrueType;
        _data->ttfScale = other._data->ttfScale;
        std::memcpy(_data->charWidths, other._data->charWidths, sizeof(_data->charWidths));
        std::memcpy(_data->charOffsets, other._data->charOffsets, sizeof(_data->charOffsets));

        if (other._data->bitmapData && other._data->bitmapSize > 0)
        {
            _data->bitmapData = static_cast<unsigned char*>(std::malloc(other._data->bitmapSize));
            if (_data->bitmapData)
            {
                std::memcpy(_data->bitmapData, other._data->bitmapData, other._data->bitmapSize);
                _data->bitmapSize = other._data->bitmapSize;

                // For TTF fonts, re-initialize ttfInfo to point to the new bitmapData
                if (static_cast<bool>(_data->isTrueType))
                {
                    int fontOffset = stbtt_GetFontOffsetForIndex(_data->bitmapData, 0);
                    stbtt_InitFont(&_data->ttfInfo, _data->bitmapData, fontOffset);
                }
            }
        }

        // Copy cached glyphs
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
        {
            if (static_cast<bool>(other._data->glyphCached[static_cast<int>(i)]))
            {
                _data->glyphCache[static_cast<int>(i)] = other._data->glyphCache[static_cast<int>(i)];
                _data->glyphCached[static_cast<int>(i)] = Boolean(true);
            }
        }
    }
}

Font::Font(Font&& other) noexcept : _data(other._data)
{
    other._data = nullptr;
}

Font::~Font()
{
    if (_data)
    {
        delete _data;
        _data = nullptr;
    }
}

Font& Font::operator=(const Font& other)
{
    if (this != &other)
    {
        if (_data)
        {
            delete _data;
            _data = nullptr;
        }
        if (other._data)
        {
            Font temp(other);
            _data = temp._data;
            temp._data = nullptr;
        }
    }
    return *this;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other)
    {
        if (_data)
        {
            delete _data;
        }
        _data = other._data;
        other._data = nullptr;
    }
    return *this;
}

Font Font::FromFile(const char* path, Int32 size, FontStyle style)
{
    const UInt16 MZ_SIGNATURE = UInt16(0x5A4D);
    const UInt16 NE_SIGNATURE = UInt16(0x454E);
    const UInt16 RT_FONT = UInt16(0x8008);  // NE resource type for fonts

    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetSize = size;

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < 64)
    {
        throw InvalidDataException("File is too small to be a valid FON file.");
    }

    // Copy to raw buffer
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Parse MZ header to find NE header
    const MsDosExecutableHeader* dosHeader = reinterpret_cast<const MsDosExecutableHeader*>(fileData);
    if (dosHeader->Signature() != static_cast<unsigned short>(MZ_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid DOS executable header.");
    }

    UInt32 neOffset = UInt32(dosHeader->NewHeaderOffset());
    if (static_cast<unsigned int>(neOffset) >= static_cast<unsigned int>(fileSize) - sizeof(NeHeader))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid NE header offset.");
    }

    const NeHeader* neHeader = reinterpret_cast<const NeHeader*>(fileData + static_cast<unsigned int>(neOffset));
    if (neHeader->Signature() != static_cast<unsigned short>(NE_SIGNATURE))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid NE signature (not a FON file).");
    }

    // Parse resource table
    UInt32 rsrcTableOffset = UInt32(static_cast<unsigned int>(neOffset) + neHeader->ResourceTableOffset());
    if (static_cast<unsigned int>(rsrcTableOffset) >= static_cast<unsigned int>(fileSize))
    {
        std::free(fileData);
        throw InvalidDataException("Invalid resource table offset.");
    }

    // Resource table starts with alignment shift count
    const unsigned char* rsrcTable = fileData + static_cast<unsigned int>(rsrcTableOffset);
    UInt16 alignShift = UInt16(*reinterpret_cast<const unsigned short*>(rsrcTable));
    rsrcTable += 2;

    // Find RT_FONT resources
    const FntHeader* bestFont = nullptr;
    Int32 bestMatch = Int32(0x7FFFFFFF);
    Boolean isBold = Boolean((static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Bold)) != 0);
    Boolean isItalic = Boolean((static_cast<unsigned char>(style) & static_cast<unsigned char>(FontStyle::Italic)) != 0);

    while (true)
    {
        const NeResourceTypeInfo* typeInfo = reinterpret_cast<const NeResourceTypeInfo*>(rsrcTable);
        if (typeInfo->TypeId() == 0) break;  // End of resource table

        rsrcTable += sizeof(NeResourceTypeInfo);

        if (typeInfo->TypeId() == static_cast<unsigned short>(RT_FONT))
        {
            // Found font resources
            for (Int32 i = Int32(0); static_cast<int>(i) < typeInfo->Count(); i += 1)
            {
                const NeResourceNameInfo* nameInfo = reinterpret_cast<const NeResourceNameInfo*>(rsrcTable);
                rsrcTable += sizeof(NeResourceNameInfo);

                // Calculate actual offset
                UInt32 fontOffset = UInt32(static_cast<unsigned int>(nameInfo->Offset()) << static_cast<unsigned short>(alignShift));
                if (static_cast<unsigned int>(fontOffset) >= static_cast<unsigned int>(fileSize)) continue;

                const FntHeader* fntHeader = reinterpret_cast<const FntHeader*>(fileData + static_cast<unsigned int>(fontOffset));

                // Check if this font matches our criteria
                Int32 fontPoints = Int32(fntHeader->Points());
                Boolean fontBold = Boolean(fntHeader->Weight() >= 700);
                Boolean fontItalic = Boolean(fntHeader->Italic() != 0);

                // Calculate match score (lower is better)
                Int32 sizeDiff = Int32(static_cast<int>(fontPoints) > static_cast<int>(targetSize) ? static_cast<int>(fontPoints) - static_cast<int>(targetSize) : static_cast<int>(targetSize) - static_cast<int>(fontPoints));
                Int32 styleMatch = Int32(0);
                if (static_cast<bool>(fontBold) != static_cast<bool>(isBold)) styleMatch = Int32(static_cast<int>(styleMatch) + 100);
                if (static_cast<bool>(fontItalic) != static_cast<bool>(isItalic)) styleMatch = Int32(static_cast<int>(styleMatch) + 100);

                Int32 matchScore = Int32(static_cast<int>(sizeDiff) + static_cast<int>(styleMatch));
                if (static_cast<int>(matchScore) < static_cast<int>(bestMatch))
                {
                    bestMatch = matchScore;
                    bestFont = fntHeader;
                }
            }
        }
        else
        {
            // Skip resources of other types
            rsrcTable += typeInfo->Count() * sizeof(NeResourceNameInfo);
        }
    }

    if (!bestFont)
    {
        std::free(fileData);
        throw InvalidDataException("No font resources found in file.");
    }

    // Parse the selected font
    FontData* data = new FontData();
    data->pointSize = Int32(bestFont->Points());
    data->pixelHeight = Int32(bestFont->PixHeight());
    data->ascent = Int32(bestFont->Ascent());
    data->firstChar = Int32(bestFont->FirstChar());
    data->lastChar = Int32(bestFont->LastChar());

    // Use requested style (allows fake bold/italic even if font doesn't have it)
    // Combine with any inherent style from the font file
    data->style = style;
    if (bestFont->Weight() >= 700)
    {
        data->style = data->style | FontStyle::Bold;
    }
    if (bestFont->Italic())
    {
        data->style = data->style | FontStyle::Italic;
    }

    // Get face name
    UInt32 faceOffset = UInt32(bestFont->dfFace);
    const unsigned char* fontBase = reinterpret_cast<const unsigned char*>(bestFont);
    if (static_cast<unsigned int>(faceOffset) > 0 && static_cast<unsigned int>(faceOffset) < 0x10000)
    {
        const char* faceName = reinterpret_cast<const char*>(fontBase + static_cast<unsigned int>(faceOffset));
        data->name = String(faceName);
    }
    else
    {
        data->name = String("Unknown");
    }

    // Calculate character widths and offsets
    Boolean isV3 = Boolean(bestFont->Version() >= 0x0300);
    Int32 numChars = Int32(static_cast<int>(data->lastChar) - static_cast<int>(data->firstChar) + 1);

    // Character table follows FntHeader (per FreeType winfnt.c line 1026)
    // V2.0: 118 bytes
    // V3.0: 148 bytes
    Int32 headerSize = static_cast<bool>(isV3) ? Int32(148) : Int32(118);
    const unsigned char* charTable = fontBase + static_cast<int>(headerSize);

    if (static_cast<bool>(isV3))
    {
        // V3.0 format: 6-byte entries (2-byte width + 4-byte offset)
        const FntCharEntryV3* entries = reinterpret_cast<const FntCharEntryV3*>(charTable);

        for (Int32 i = Int32(0); static_cast<int>(i) <= static_cast<int>(numChars); i += 1)
        {  // +1 for sentinel
            Int32 charCode = Int32(static_cast<int>(data->firstChar) + static_cast<int>(i));
            if (static_cast<int>(charCode) >= 0 && static_cast<int>(charCode) < 256 && static_cast<int>(i) < static_cast<int>(numChars))
            {
                data->charWidths[static_cast<int>(charCode)] = entries[static_cast<int>(i)].width;
                data->charOffsets[static_cast<int>(charCode)] = entries[static_cast<int>(i)].offset;
            }
        }
    }
    else
    {
        // V2.0 format: 4-byte entries (2-byte width + 2-byte offset)
        const FntCharEntryV2* entries = reinterpret_cast<const FntCharEntryV2*>(charTable);

        for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(numChars); i += 1)
        {
            Int32 charCode = Int32(static_cast<int>(data->firstChar) + static_cast<int>(i));
            if (static_cast<int>(charCode) >= 0 && static_cast<int>(charCode) < 256)
            {
                data->charWidths[static_cast<int>(charCode)] = entries[static_cast<int>(i)].width;
                data->charOffsets[static_cast<int>(charCode)] = entries[static_cast<int>(i)].offset;
            }
        }
    }

    // Copy bitmap data
    // Each glyph bitmap size is: pitch * height (where pitch = ceil(width/8))
    UInt32 bitmapStart = UInt32(0);
    UInt32 bitmapEnd = UInt32(0);
    Int32 height = data->pixelHeight;

    if (static_cast<bool>(isV3))
    {
        // In V3, offsets are from start of FNT resource
        bitmapStart = UInt32(0xFFFFFFFF);
        for (Int32 i = data->firstChar; static_cast<int>(i) <= static_cast<int>(data->lastChar); i += 1)
        {
            if (data->charOffsets[static_cast<int>(i)] > 0 && data->charOffsets[static_cast<int>(i)] < static_cast<unsigned int>(bitmapStart))
            {
                bitmapStart = UInt32(data->charOffsets[static_cast<int>(i)]);
            }
            Int32 bytesPerRow = Int32((data->charWidths[static_cast<int>(i)] + 7) / 8);
            UInt32 charEnd = UInt32(data->charOffsets[static_cast<int>(i)] + static_cast<int>(bytesPerRow) * static_cast<int>(height));
            if (static_cast<unsigned int>(charEnd) > static_cast<unsigned int>(bitmapEnd))
            {
                bitmapEnd = charEnd;
            }
        }
    }
    else
    {
        // In V2, offsets are from start of FNT resource
        bitmapStart = UInt32(0xFFFFFFFF);
        for (Int32 i = data->firstChar; static_cast<int>(i) <= static_cast<int>(data->lastChar); i += 1)
        {
            if (data->charOffsets[static_cast<int>(i)] > 0 && data->charOffsets[static_cast<int>(i)] < static_cast<unsigned int>(bitmapStart))
            {
                bitmapStart = UInt32(data->charOffsets[static_cast<int>(i)]);
            }
            Int32 bytesPerRow = Int32((data->charWidths[static_cast<int>(i)] + 7) / 8);
            UInt32 charEnd = UInt32(data->charOffsets[static_cast<int>(i)] + static_cast<int>(bytesPerRow) * static_cast<int>(height));
            if (static_cast<unsigned int>(charEnd) > static_cast<unsigned int>(bitmapEnd))
            {
                bitmapEnd = charEnd;
            }
        }
    }

    if (static_cast<unsigned int>(bitmapEnd) > static_cast<unsigned int>(bitmapStart))
    {
        data->bitmapSize = static_cast<unsigned int>(bitmapEnd);  // Store entire font data to simplify offsets
        data->bitmapData = static_cast<unsigned char*>(std::malloc(data->bitmapSize));
        if (data->bitmapData)
        {
            std::memcpy(data->bitmapData, fontBase, data->bitmapSize);
        }
    }

    std::free(fileData);
    return Font(data);
}

Font Font::SystemFont()
{
    try {
        return FromFile("MSSANS.fon", 8);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::SystemFontBold()
{
    try {
        return FromFile("MSSANS.fon", 8, FontStyle::Bold);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FixedFont()
{
    try {
        return FromFile("FIXEDSYS.fon", 8);
    } catch (...)
    {
        // Return invalid font if file not found
        return Font();
    }
}

Font Font::FromTrueType(const char* path, Int32 pixelHeight, FontStyle style)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    Int32 targetHeight = pixelHeight;
    if (static_cast<int>(targetHeight) <= 0)
    {
        throw ArgumentException("pixelHeight must be positive.");
    }

    // Read file
    Array<UInt8> fileBytes = IO::File::ReadAllBytes(path);
    Int32 fileSize = Int32(fileBytes.Length());

    if (static_cast<int>(fileSize) < 12)
    {
        throw InvalidDataException("File is too small to be a valid TTF file.");
    }

    // Allocate and copy file data (stb_truetype requires persistent data)
    unsigned char* fileData = static_cast<unsigned char*>(std::malloc(static_cast<int>(fileSize)));
    if (!fileData)
    {
        throw InvalidOperationException("Failed to allocate memory.");
    }

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(fileSize); i += 1)
    {
        fileData[static_cast<int>(i)] = static_cast<unsigned char>(fileBytes[static_cast<int>(i)]);
    }

    // Initialize stb_truetype
    FontData* data = new FontData();
    data->bitmapData = fileData;
    data->bitmapSize = static_cast<int>(fileSize);
    data->isTrueType = Boolean(true);
    data->style = style;

    // Get font offset (handles font collections and validates TTF header)
    Int32 fontOffset = Int32(stbtt_GetFontOffsetForIndex(fileData, 0));
    if (static_cast<int>(fontOffset) < 0)
    {
        delete data;
        throw InvalidDataException("Invalid TTF file or font index.");
    }

    if (!stbtt_InitFont(&data->ttfInfo, fileData, static_cast<int>(fontOffset)))
    {
        delete data;
        throw InvalidDataException("Failed to parse TTF file.");
    }

    // Calculate scale for desired pixel height
    data->ttfScale = Float32(stbtt_ScaleForPixelHeight(&data->ttfInfo, static_cast<float>(targetHeight)));

    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&data->ttfInfo, &ascent, &descent, &lineGap);

    data->pixelHeight = targetHeight;
    data->ascent = Int32(static_cast<int>(ascent * static_cast<float>(data->ttfScale)));
    data->pointSize = targetHeight;  // Approximate
    data->firstChar = Int32(32);   // Space
    data->lastChar = Int32(126);   // Tilde

    // Pre-calculate character widths (round instead of truncate)
    for (Int32 ch = Int32(0); static_cast<int>(ch) < 256; ch += 1)
    {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&data->ttfInfo, static_cast<int>(ch), &advanceWidth, &leftSideBearing);
        // Add 0.5f for proper rounding to avoid accumulated spacing errors
        data->charWidths[static_cast<int>(ch)] = static_cast<unsigned short>(advanceWidth * static_cast<float>(data->ttfScale) + 0.5f);
    }

    // Get font name from TTF name table (simplified - just use filename)
    const char* nameStart = path;
    const char* p = path;
    while (*p)
    {
        if (*p == '/' || *p == '\\') nameStart = p + 1;
        p++;
    }
    // Remove extension
    char nameBuf[64];
    Int32 nameLen = Int32(0);
    for (const char* q = nameStart; *q && *q != '.' && static_cast<int>(nameLen) < 63; q++)
    {
        nameBuf[static_cast<int>(nameLen)] = *q;
        nameLen += 1;
    }
    nameBuf[static_cast<int>(nameLen)] = '\0';
    data->name = String(nameBuf);

    return Font(data);
}

String Font::Name() const {
    return _data ? _data->name : String();
}

Int32 Font::Size() const {
    return _data ? _data->pointSize : Int32(0);
}

Int32 Font::Height() const {
    return _data ? _data->pixelHeight : Int32(0);
}

Int32 Font::Ascent() const {
    return _data ? _data->ascent : Int32(0);
}

FontStyle Font::Style() const {
    return _data ? _data->style : FontStyle::Regular;
}

Boolean Font::IsValid() const {
    return Boolean(_data != nullptr && static_cast<int>(_data->pixelHeight) > 0);
}

Boolean Font::IsTrueType() const {
    return Boolean(_data != nullptr && static_cast<bool>(_data->isTrueType));
}

void* Font::GetTTFInfo() const {
    if (!_data || !static_cast<bool>(_data->isTrueType)) return nullptr;
    return const_cast<stbtt_fontinfo*>(&_data->ttfInfo);
}

float Font::GetTTFScale() const {
    if (!_data || !static_cast<bool>(_data->isTrueType)) return 0.0f;
    return static_cast<float>(_data->ttfScale);
}

Int32 Font::GetCharWidth(Char c) const {
    if (!_data) return Int32(0);
    Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(c)));
    return Int32(_data->charWidths[static_cast<int>(ch)]);
}

Drawing::Size Font::MeasureString(const String& text) const {
    return MeasureString(text.GetRawString());
}

Drawing::Size Font::MeasureString(const char* text) const {
    if (!_data || !text) return Drawing::Size(Int32(0), Int32(0));

    // Check if font style includes bold (adds 1 pixel per character)
    Boolean isBold = Boolean((static_cast<unsigned char>(_data->style) &
                   static_cast<unsigned char>(FontStyle::Bold)) != 0);

    Int32 maxWidth = Int32(0);
    Int32 currentWidth = Int32(0);
    Int32 lines = Int32(1);
    Int32 charsOnLine = Int32(0);

    for (const char* p = text; *p; p++)
    {
        if (*p == '\n')
        {
            // Add extra pixels for bold characters
            if (static_cast<bool>(isBold) && static_cast<int>(charsOnLine) > 0)
            {
                currentWidth = Int32(static_cast<int>(currentWidth) + static_cast<int>(charsOnLine));
            }
            if (static_cast<int>(currentWidth) > static_cast<int>(maxWidth)) maxWidth = currentWidth;
            currentWidth = Int32(0);
            charsOnLine = Int32(0);
            lines += 1;
        }
        else
        {
            Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(*p)));
            currentWidth = Int32(static_cast<int>(currentWidth) + _data->charWidths[static_cast<int>(ch)]);
            charsOnLine += 1;
        }
    }

    // Add extra pixels for bold characters on last line
    if (static_cast<bool>(isBold) && static_cast<int>(charsOnLine) > 0)
    {
        currentWidth = Int32(static_cast<int>(currentWidth) + static_cast<int>(charsOnLine));
    }
    if (static_cast<int>(currentWidth) > static_cast<int>(maxWidth)) maxWidth = currentWidth;
    return Drawing::Size(maxWidth, Int32(static_cast<int>(lines) * static_cast<int>(_data->pixelHeight)));
}

const Image& Font::GetGlyph(Char c) const {
    static Image emptyGlyph(Int32(1), Int32(1), Color::Transparent);
    if (!_data) return emptyGlyph;

    Int32 ch = Int32(static_cast<int>(static_cast<unsigned char>(c)));
    if (!static_cast<bool>(_data->glyphCached[static_cast<int>(ch)]))
    {
        _data->RenderGlyph(ch);
    }
    return _data->glyphCache[static_cast<int>(ch)];
}

} // namespace System::Drawing
