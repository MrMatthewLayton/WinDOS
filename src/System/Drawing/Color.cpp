#include "Color.hpp"

namespace System::Drawing
{

/******************************************************************************/
/*    VGA 16-color palette RGB values (used for dithering to 4bpp)            */
/******************************************************************************/

namespace
{
    const struct { unsigned char r, g, b; } g_vgaPalette[16] = {
        { 0x00, 0x00, 0x00 },  // 0: Black
        { 0x00, 0x00, 0xAA },  // 1: DarkBlue
        { 0x00, 0xAA, 0x00 },  // 2: DarkGreen
        { 0x00, 0xAA, 0xAA },  // 3: DarkCyan
        { 0xAA, 0x00, 0x00 },  // 4: DarkRed
        { 0xAA, 0x00, 0xAA },  // 5: DarkMagenta
        { 0xAA, 0x55, 0x00 },  // 6: DarkYellow (brown)
        { 0xAA, 0xAA, 0xAA },  // 7: Gray
        { 0x55, 0x55, 0x55 },  // 8: DarkGray
        { 0x55, 0x55, 0xFF },  // 9: Blue
        { 0x55, 0xFF, 0x55 },  // 10: Green
        { 0x55, 0xFF, 0xFF },  // 11: Cyan
        { 0xFF, 0x55, 0x55 },  // 12: Red
        { 0xFF, 0x55, 0xFF },  // 13: Magenta
        { 0xFF, 0xFF, 0x55 },  // 14: Yellow
        { 0xFF, 0xFF, 0xFF }   // 15: White
    };
} // anonymous namespace

/******************************************************************************/
/*    Color constants (unified 32-bit ARGB)                                   */
/******************************************************************************/

const Color Color::Black(0xFF000000);
const Color Color::White(0xFFFFFFFF);
const Color Color::Red(0xFFFF5555);       // Match VGA bright red
const Color Color::Green(0xFF55FF55);     // Match VGA bright green
const Color Color::Blue(0xFF5555FF);      // Match VGA bright blue
const Color Color::Cyan(0xFF55FFFF);      // Match VGA bright cyan
const Color Color::Magenta(0xFFFF55FF);   // Match VGA bright magenta
const Color Color::Yellow(0xFFFFFF55);    // Match VGA bright yellow
const Color Color::Gray(0xFFAAAAAA);      // Match VGA gray
const Color Color::DarkGray(0xFF555555);  // Match VGA dark gray
const Color Color::DarkBlue(0xFF0000AA);
const Color Color::DarkGreen(0xFF00AA00);
const Color Color::DarkCyan(0xFF00AAAA);
const Color Color::DarkRed(0xFFAA0000);
const Color Color::DarkMagenta(0xFFAA00AA);
const Color Color::DarkYellow(0xFFAA5500);// Brown
const Color Color::Transparent(0x00000000);

/******************************************************************************/
/*    Color methods                                                           */
/******************************************************************************/

Color Color::Lerp(const Color& c1, const Color& c2, Float32 t)
{
    if (t <= 0.0f) return c1;
    if (t >= 1.0f) return c2;

    Float32 oneMinusT = 1.0f - t;
    UInt8 a = UInt8(static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.A())) * static_cast<float>(oneMinusT) +
        static_cast<float>(static_cast<unsigned char>(c2.A())) * static_cast<float>(t)));
    UInt8 r = UInt8(static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.R())) * static_cast<float>(oneMinusT) +
        static_cast<float>(static_cast<unsigned char>(c2.R())) * static_cast<float>(t)));
    UInt8 g = UInt8(static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.G())) * static_cast<float>(oneMinusT) +
        static_cast<float>(static_cast<unsigned char>(c2.G())) * static_cast<float>(t)));
    UInt8 b = UInt8(static_cast<unsigned char>(
        static_cast<float>(static_cast<unsigned char>(c1.B())) * static_cast<float>(oneMinusT) +
        static_cast<float>(static_cast<unsigned char>(c2.B())) * static_cast<float>(t)));

    return Color(a, r, g, b);
}

UInt8 Color::ToVgaIndex() const
{
    return RgbToVgaIndex(R(), G(), B());
}

UInt8 Color::RgbToVgaIndex(UInt8 r, UInt8 g, UInt8 b)
{
    UInt32 bestDist = UInt32(0xFFFFFFFF);
    UInt8 bestIndex = UInt8(0);

    unsigned char rv = static_cast<unsigned char>(r);
    unsigned char gv = static_cast<unsigned char>(g);
    unsigned char bv = static_cast<unsigned char>(b);

    for (Int32 i = Int32(0); static_cast<int>(i) < 16; i += 1)
    {
        Int32 dr = Int32(static_cast<int>(rv) - g_vgaPalette[static_cast<int>(i)].r);
        Int32 dg = Int32(static_cast<int>(gv) - g_vgaPalette[static_cast<int>(i)].g);
        Int32 db = Int32(static_cast<int>(bv) - g_vgaPalette[static_cast<int>(i)].b);
        UInt32 dist = UInt32(static_cast<int>(dr) * static_cast<int>(dr) +
                             static_cast<int>(dg) * static_cast<int>(dg) +
                             static_cast<int>(db) * static_cast<int>(db));

        if (static_cast<unsigned int>(dist) < static_cast<unsigned int>(bestDist))
        {
            bestDist = dist;
            bestIndex = UInt8(static_cast<unsigned char>(i));
        }
    }
    return bestIndex;
}

void Color::BuildVgaRemap(const unsigned char* paletteData, UInt32 paletteCount, unsigned char remap[16])
{
    UInt32 count = paletteCount;
    // For each palette entry, find the closest VGA color
    for (UInt32 i = UInt32(0); static_cast<unsigned int>(i) < static_cast<unsigned int>(count) && static_cast<unsigned int>(i) < 16; i += 1)
    {
        // BMP palette is BGRA format (4 bytes per entry)
        unsigned char b = paletteData[static_cast<unsigned int>(i) * 4 + 0];
        unsigned char g = paletteData[static_cast<unsigned int>(i) * 4 + 1];
        unsigned char r = paletteData[static_cast<unsigned int>(i) * 4 + 2];
        remap[static_cast<unsigned int>(i)] = static_cast<unsigned char>(RgbToVgaIndex(r, g, b));
    }
    // Fill remaining entries with black
    for (UInt32 i = count; static_cast<unsigned int>(i) < 16; i += 1)
    {
        remap[static_cast<unsigned int>(i)] = 0;
    }
}

} // namespace System::Drawing
