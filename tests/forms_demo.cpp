// Forms Demo - Windows 95 style WIMP system demo
// Tests Desktop, Window, TaskBar, Button, StartMenu, and layout system

#include "../src/BCL.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Devices;
using namespace System::Windows::Forms;

int main() {
    // Initialize mouse
    Mouse::Initialize();

    // Slow down mouse (default is horizontal=8, vertical=16; higher = slower)
    Mouse::SetSensitivity(16, 32);

    // Target 800x600x32 VBE mode
    int screenWidth = 800;
    int screenHeight = 600;
    bool useVbe = false;

    if (static_cast<bool>(Display::IsVbeAvailable())) {
        Display vbeMode = Display::DetectVbeMode(UInt16(800), UInt16(600), UInt8(32));
        if (static_cast<bool>(vbeMode.IsVbeMode())) {
            // VBE mode found - try to use it
            Display::SetMode(vbeMode);

            // Check if LFB was successfully mapped
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
            }
        }
    }

    // Fall back to VGA mode if VBE not available or failed
    if (!useVbe) {
        Display::SetMode(Display::VGA_640x480x4);
        GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
        screenWidth = 640;
        screenHeight = 480;
    }

    // Hide hardware cursor (we draw our own)
    Mouse::HideCursor();

    // Set mouse bounds to screen
    Mouse::SetBounds(0, 0, screenWidth - 1, screenHeight - 1);

    // Create desktop with cyan background
    Desktop desktop(Color::Cyan);

    // Load cursor from icon library (Index 13 has a 24x24 arrow cursor)
    try {
        desktop.LoadCursorFromLibrary("sysicons.icl", 13);
    } catch (...) {
        // If cursor loading fails, desktop will use fallback cursor
    }

    // Add desktop icons (32x32 icons from sysicons.icl)
    try {
        desktop.AddIconFromLibrary("sysicons.icl", 8);   // Icon 1
        desktop.AddIconFromLibrary("sysicons.icl", 7);   // Icon 2
        desktop.AddIconFromLibrary("sysicons.icl", 55);  // Icon 3
        desktop.AddIconFromLibrary("sysicons.icl", 12);  // Icon 4
    } catch (...) {
        // If icon loading fails, continue without icons
    }

    // Create start menu FIRST (before taskbar)
    StartMenu* startMenu = new StartMenu(&desktop);
    desktop.SetStartMenu(startMenu);

    // Create taskbar with start menu reference
    // TaskBar uses row layout internally with gap=2, alignItems=Center
    TaskBar* taskBar = new TaskBar(&desktop, startMenu);
    desktop.SetTaskBar(taskBar);
    taskBar->SetDesktop(&desktop);

    // Create windows - these are floating (participatesInLayout = false)
    // so they can be freely positioned and dragged
    Window* window1 = new Window(&desktop, Rectangle(80, 60, 320, 240));
    Window* window2 = new Window(&desktop, Rectangle(450, 120, 280, 200));

    // Window 1: Use column layout for spectrum controls
    window1->Layout().SetDirection(FlexDirection::Row)
                     .SetJustifyContent(JustifyContent::SpaceAround)
                     .SetAlignItems(AlignItems::Stretch)
                     .SetPadding(10);

    // Create spectrum controls - they'll be arranged by the layout system
    SpectrumControl* spectrum1 = new SpectrumControl(window1,
        Rectangle(0, 0, 60, 100), Color32::Red);
    spectrum1->Layout().SetFlexGrow(1).SetMargin(5);

    SpectrumControl* spectrum2 = new SpectrumControl(window1,
        Rectangle(0, 0, 60, 100), Color32::Green);
    spectrum2->Layout().SetFlexGrow(1).SetMargin(5);

    SpectrumControl* spectrum3 = new SpectrumControl(window1,
        Rectangle(0, 0, 60, 100), Color32::Blue);
    spectrum3->Layout().SetFlexGrow(1).SetMargin(5);

    SpectrumControl* spectrum4 = new SpectrumControl(window1,
        Rectangle(0, 0, 60, 100), Color32::Yellow);
    spectrum4->Layout().SetFlexGrow(1).SetMargin(5);

    // Perform layout on window1 to arrange spectrum controls
    window1->PerformLayout();

    // Window 2: Display a test bitmap
    // Load test bitmap and add Picture to window2's client area
    Image testBitmap = Image::FromBitmap("test.bmp");

    // Picture fills the window's client area using layout
    window2->Layout().SetDirection(FlexDirection::Column)
                     .SetAlignItems(AlignItems::Stretch)
                     .SetPadding(5);

    Picture* picture = new Picture(window2, Rectangle(0, 0, 100, 100), testBitmap);
    picture->Layout().SetFlexGrow(1);  // Fill available space

    // Perform layout on window2
    window2->PerformLayout();

    // Suppress unused variable warnings
    (void)spectrum1;
    (void)spectrum2;
    (void)spectrum3;
    (void)spectrum4;
    (void)picture;

    // Run the event loop (ESC to exit)
    // - Click Start button to open/close start menu
    // - Hover over menu items to see blue highlight
    // - Click on windows to focus them (taskbar buttons update)
    // - Drag windows by title bar
    // - Click outside start menu to close it
    desktop.Run();

    // Cleanup
    GraphicsBuffer::DestroyFrameBuffer();
    Display::SetDefaultMode();

    return 0;
}
