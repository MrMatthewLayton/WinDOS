// Minimal VBE Forms test - isolate crash in forms_demo
#include "../src/rtcorlib.hpp"
#include <cstdio>
#include <conio.h>

using namespace System;
using namespace System::Drawing;
using namespace System::IO::Devices;
using namespace System::Windows::Forms;

int main() {
    printf("=== VBE Forms Test ===\n\n");

    // Step 1: Initialize mouse
    printf("Step 1: Initializing mouse...\n");
    Mouse::Initialize();
    printf("  Done.\n");
    getch();

    // Step 2: Setup VBE mode
    printf("Step 2: Setting up VBE mode...\n");
    bool useVbe = false;
    int screenWidth = 640;
    int screenHeight = 480;

    if (static_cast<bool>(Display::IsVbeAvailable())) {
        Display vbeMode = Display::DetectVbeMode(UInt16(800), UInt16(600), UInt8(32));
        if (static_cast<bool>(vbeMode.IsVbeMode())) {
            Display::SetMode(vbeMode);
            void* lfb = Display::GetMappedLfb();
            if (lfb) {
                GraphicsBuffer::CreateFrameBuffer32(
                    Int32(vbeMode.Width()),
                    Int32(vbeMode.Height()),
                    vbeMode.VbeMode(),
                    lfb,
                    vbeMode.LfbPitch(),
                    vbeMode.BitsPerPixel()
                );
                useVbe = true;
                screenWidth = static_cast<int>(vbeMode.Width());
                screenHeight = static_cast<int>(vbeMode.Height());
                printf("  VBE mode set: %dx%d @ %dbpp\n", screenWidth, screenHeight,
                       static_cast<int>(vbeMode.BitsPerPixel()));
            }
        }
    }

    if (!useVbe) {
        printf("  VBE failed, using VGA fallback.\n");
        Display::SetMode(Display::VGA_640x480x4);
        GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
    }
    getch();

    // Step 3: Hide cursor, set bounds
    printf("Step 3: Setting up mouse bounds...\n");
    Mouse::HideCursor();
    Mouse::SetBounds(0, 0, screenWidth - 1, screenHeight - 1);
    printf("  Done.\n");
    getch();

    // Step 4: Create desktop only
    printf("Step 4: Creating desktop...\n");
    Desktop desktop(Color::Cyan);
    printf("  Desktop created.\n");
    getch();

    // Step 5: Try to flush/render
    printf("Step 5: Flushing framebuffer...\n");
    GraphicsBuffer::FlushFrameBuffer();
    printf("  Flushed. You should see cyan screen.\n");
    getch();

    // Step 6: Create a simple window
    printf("Step 6: Creating window...\n");
    Window* window1 = new Window(&desktop, Rectangle(50, 50, 200, 150));
    (void)window1;
    printf("  Window created.\n");
    getch();

    // Step 7: Flush again
    printf("Step 7: Flushing with window...\n");
    GraphicsBuffer::FlushFrameBuffer();
    printf("  Flushed. You should see window.\n");
    getch();

    // Step 8: Run brief event loop
    printf("Step 8: Running event loop (press ESC to exit)...\n");
    desktop.Run();

    // Cleanup
    GraphicsBuffer::DestroyFrameBuffer();
    Display::SetDefaultMode();

    printf("\n=== Test Complete ===\n");
    getch();

    return 0;
}
