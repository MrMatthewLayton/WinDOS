// Hatch Pattern Demo - Shows various hatch patterns
// Tests the HatchStyle class with FillRectangle

#include "../src/rtcorlib.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::IO::Devices;

int main() {
    // Initialize display - use VGA 640x480x16 for simplicity
    Display::SetMode(Display::VGA_640x480x4);
    GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);

    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (!fb) return 1;

    Graphics g(BufferMode::Single, Rectangle(0, 0, 640, 480));

    // Fill background
    g.FillRectangle(0, 0, 640, 480, Color::DarkCyan);

    // Define patterns to show
    struct PatternInfo {
        const HatchStyle* style;
        const char* name;
    };

    PatternInfo patterns[] = {
        { &HatchStyle::Horizontal, "Horizontal" },
        { &HatchStyle::Vertical, "Vertical" },
        { &HatchStyle::Cross, "Cross" },
        { &HatchStyle::ForwardDiagonal, "FwdDiag" },
        { &HatchStyle::BackwardDiagonal, "BkDiag" },
        { &HatchStyle::DiagonalCross, "DiagCross" },
        { &HatchStyle::Percent25, "25%" },
        { &HatchStyle::Percent50, "50%" },
        { &HatchStyle::Percent75, "75%" },
        { &HatchStyle::Brick, "Brick" },
        { &HatchStyle::Weave, "Weave" },
        { &HatchStyle::Trellis, "Trellis" },
        { &HatchStyle::LargeGrid, "LargeGrid" },
        { &HatchStyle::SmallGrid, "SmallGrid" },
        { &HatchStyle::DottedGrid, "DotGrid" },
        { &HatchStyle::ZigZag, "ZigZag" },
        { &HatchStyle::Wave, "Wave" },
        { &HatchStyle::Sphere, "Sphere" },
    };

    int numPatterns = sizeof(patterns) / sizeof(patterns[0]);

    // Draw patterns in a grid (6 columns, 3 rows)
    int cols = 6;
    int boxWidth = 100;
    int boxHeight = 140;
    int startX = 10;
    int startY = 20;

    for (int i = 0; i < numPatterns; i++) {
        int col = i % cols;
        int row = i / cols;

        int x = startX + col * boxWidth;
        int y = startY + row * boxHeight;

        // Draw pattern box
        g.FillRectangle(x, y, boxWidth - 10, boxHeight - 30,
                        *patterns[i].style, Color::White, Color::DarkBlue);

        // Draw border around the box
        g.DrawRectangle(x, y, boxWidth - 10, boxHeight - 30, Color::Black);
    }

    // Flush to screen
    GraphicsBuffer::FlushFrameBuffer();

    // Wait for any key
    Console::WriteLine("Press any key to exit...");
    while (!static_cast<bool>(Keyboard::IsKeyPressed())) {
        // Wait
    }

    // Cleanup
    GraphicsBuffer::DestroyFrameBuffer();
    Display::SetDefaultMode();

    return 0;
}
