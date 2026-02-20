#include "Color.hpp"

namespace System::Drawing
{
    const struct
    {
        UInt8 r, g, b;
    } g_vgaPalette[16] = {
        {0x00, 0x00, 0x00}, // 0: Black
        {0x00, 0x00, 0xAA}, // 1: DarkBlue
        {0x00, 0xAA, 0x00}, // 2: DarkGreen
        {0x00, 0xAA, 0xAA}, // 3: DarkCyan
        {0xAA, 0x00, 0x00}, // 4: DarkRed
        {0xAA, 0x00, 0xAA}, // 5: DarkMagenta
        {0xAA, 0x55, 0x00}, // 6: DarkYellow (brown)
        {0xAA, 0xAA, 0xAA}, // 7: Gray
        {0x55, 0x55, 0x55}, // 8: DarkGray
        {0x55, 0x55, 0xFF}, // 9: Blue
        {0x55, 0xFF, 0x55}, // 10: Green
        {0x55, 0xFF, 0xFF}, // 11: Cyan
        {0xFF, 0x55, 0x55}, // 12: Red
        {0xFF, 0x55, 0xFF}, // 13: Magenta
        {0xFF, 0xFF, 0x55}, // 14: Yellow
        {0xFF, 0xFF, 0xFF} // 15: White
    };

    const Color Color::Black(0xFF000000);
    const Color Color::White(0xFFFFFFFF);
    const Color Color::Red(0xFFFF5555);
    const Color Color::Green(0xFF55FF55);
    const Color Color::Blue(0xFF5555FF);
    const Color Color::Cyan(0xFF55FFFF);
    const Color Color::Magenta(0xFFFF55FF);
    const Color Color::Yellow(0xFFFFFF55);
    const Color Color::Gray(0xFFAAAAAA);
    const Color Color::DarkGray(0xFF555555);
    const Color Color::DarkBlue(0xFF0000AA);
    const Color Color::DarkGreen(0xFF00AA00);
    const Color Color::DarkCyan(0xFF00AAAA);
    const Color Color::DarkRed(0xFFAA0000);
    const Color Color::DarkMagenta(0xFFAA00AA);
    const Color Color::DarkYellow(0xFFAA5500);
    const Color Color::Transparent(0x00000000);

    Color Color::Lerp(const Color &first, const Color &second, const Float32 factor)
    {
        if (factor <= 0.0f)
            return first;

        if (factor >= 1.0f)
            return second;

        const Float32 oneMinusT = 1.0f - factor;

        // TODO : Cleanup using .To* methods.

        const UInt8 a = UInt8(static_cast<unsigned char>(
            static_cast<float>(static_cast<unsigned char>(first.A())) * static_cast<float>(oneMinusT) +
            static_cast<float>(static_cast<unsigned char>(second.A())) * static_cast<float>(factor)));
        const UInt8 r = UInt8(static_cast<unsigned char>(
            static_cast<float>(static_cast<unsigned char>(first.R())) * static_cast<float>(oneMinusT) +
            static_cast<float>(static_cast<unsigned char>(second.R())) * static_cast<float>(factor)));
        const UInt8 g = UInt8(static_cast<unsigned char>(
            static_cast<float>(static_cast<unsigned char>(first.G())) * static_cast<float>(oneMinusT) +
            static_cast<float>(static_cast<unsigned char>(second.G())) * static_cast<float>(factor)));
        const UInt8 b = UInt8(static_cast<unsigned char>(
            static_cast<float>(static_cast<unsigned char>(first.B())) * static_cast<float>(oneMinusT) +
            static_cast<float>(static_cast<unsigned char>(second.B())) * static_cast<float>(factor)));

        return Color(r, g, b, a);
    }

    UInt8 Color::ToVgaIndex() const
    {
        return RgbToVgaIndex(R(), G(), B());
    }

    UInt8 Color::RgbToVgaIndex(const UInt8 red, const UInt8 green, const UInt8 blue)
    {
        UInt32 bestDist = UInt32::MaxValue;
        UInt8 bestIndex = UInt8::MinValue;

        const UInt8 rValue = red;
        const UInt8 gValue = green;
        const UInt8 bValue = blue;

        for (Int32 index = 0; index < 16; ++index)
        {
            Int32 rDist = (rValue - g_vgaPalette[index].r).ToInt32();
            Int32 gDist = (gValue - g_vgaPalette[index].g).ToInt32();
            Int32 bDist = (bValue - g_vgaPalette[index].b).ToInt32();

            if (UInt32 dist = (rDist * rDist + gDist * gDist + bDist * bDist).ToUInt32(); dist < bestDist)
            {
                bestDist = dist;
                bestIndex = index.ToUInt8();
            }
        }

        return bestIndex;
    }

    void Color::BuildVgaRemap(const UInt8 *paletteData, UInt32 paletteCount, UInt8 remap[16])
    {
        const UInt32 count = paletteCount;

        // For each palette entry, find the closest VGA color
        for (UInt32 index = 0; index < count && index < 16; ++index)
        {
            // BMP palette is BGRA format (4 bytes per entry)
            const UInt8 blue = paletteData[index * 4 + 0];
            const UInt8 green = paletteData[index * 4 + 1];
            const UInt8 red = paletteData[index * 4 + 2];

            remap[index] = RgbToVgaIndex(red, green, blue);
        }

        // Fill remaining entries with black
        for (UInt32 index = count; index < 16; ++index)
            remap[index] = 0;
    }
} // namespace System::Drawing
