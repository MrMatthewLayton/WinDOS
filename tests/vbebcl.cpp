// Minimal VBE test using rtcorlib library
// Tests VBE detection and mode setting without full Forms system

#include "../src/rtcorlib.hpp"
#include <cstdio>
#include <conio.h>

using namespace System;
using namespace System::Drawing;
using namespace System::Devices;

int main() {
    printf("=== VBE rtcorlib Test ===\n\n");

    printf("Step 1: Checking VBE availability...\n");
    getch();

    bool vbeAvail = static_cast<bool>(Display::IsVbeAvailable());
    printf("  VBE available: %s\n", vbeAvail ? "YES" : "NO");

    if (!vbeAvail) {
        printf("  VBE not available, exiting.\n");
        getch();
        return 1;
    }

    printf("\nStep 2: Detecting VBE mode 800x600x32...\n");
    getch();

    Display vbeMode = Display::DetectVbeMode(UInt16(800), UInt16(600), UInt8(32));
    bool isVbe = static_cast<bool>(vbeMode.IsVbeMode());
    printf("  Mode found: %s\n", isVbe ? "YES" : "NO");

    if (!isVbe) {
        printf("  800x600x32 not available, exiting.\n");
        getch();
        return 1;
    }

    printf("  Width: %d, Height: %d, BPP: %d\n",
           static_cast<int>(vbeMode.Width()),
           static_cast<int>(vbeMode.Height()),
           static_cast<int>(vbeMode.BitsPerPixel()));
    printf("  VBE Mode: 0x%X\n", static_cast<int>(vbeMode.VbeMode()));
    printf("  LFB Pitch: %d\n", static_cast<int>(vbeMode.LfbPitch()));

    printf("\nStep 3: Setting VBE mode...\n");
    getch();

    Display::SetMode(vbeMode);
    void* lfb = Display::GetMappedLfb();
    printf("  LFB mapped: %s (value: %p)\n", lfb ? "YES" : "NO", lfb);

    if (!lfb) {
        printf("  LFB mapping failed, reverting to text mode.\n");
        Display::SetDefaultMode();
        getch();
        return 1;
    }

    printf("\nStep 4: Creating 32-bit framebuffer...\n");
    getch();

    GraphicsBuffer::CreateFrameBuffer32(
        Int32(vbeMode.Width()),
        Int32(vbeMode.Height()),
        vbeMode.VbeMode(),
        lfb,
        vbeMode.LfbPitch(),
        vbeMode.BitsPerPixel()
    );

    printf("  Framebuffer created.\n");

    printf("\nStep 5: Drawing test pattern...\n");
    getch();

    // Get the Image32 and draw a simple pattern
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (fb && fb->GetImage32()) {
        Image32* img = fb->GetImage32();

        // Draw red rectangle
        for (int y = 100; y < 200; y++) {
            for (int x = 100; x < 300; x++) {
                img->SetPixel(x, y, Color32::Red);
            }
        }

        // Draw green rectangle
        for (int y = 200; y < 300; y++) {
            for (int x = 100; x < 300; x++) {
                img->SetPixel(x, y, Color32::Green);
            }
        }

        // Draw blue rectangle
        for (int y = 300; y < 400; y++) {
            for (int x = 100; x < 300; x++) {
                img->SetPixel(x, y, Color32::Blue);
            }
        }

        printf("  Pattern drawn to Image32.\n");
    }

    printf("\nStep 6: Flushing to LFB...\n");
    getch();

    GraphicsBuffer::FlushFrameBuffer();
    printf("  Flush complete. You should see colored rectangles.\n");

    printf("\nStep 7: Press any key to return to text mode...\n");
    getch();

    // Cleanup
    GraphicsBuffer::DestroyFrameBuffer();
    Display::SetDefaultMode();

    printf("\n=== VBE rtcorlib Test Complete ===\n");
    getch();

    return 0;
}
