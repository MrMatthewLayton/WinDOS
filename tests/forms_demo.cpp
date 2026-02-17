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

    // Determine icon library path (try C: drive first for combo boot, then current dir)
    const char* iconLibPath = "C:\\SYSICONS.ICL";
    if (!IO::File::Exists(iconLibPath)) {
        iconLibPath = SystemIcons::LibraryPath;  // Fall back to current directory
    }

    // Load cursor from icon library
    try {
        desktop.LoadCursorFromLibrary(iconLibPath, SystemIcons::CursorPointer);
    } catch (...) {
        // If cursor loading fails, desktop will use fallback cursor
    }

    // Add desktop icons (32x32 icons from sysicons.icl)
    try {
        desktop.AddIconFromLibrary(iconLibPath, SystemIcons::Computer);      // My Computer
        desktop.AddIconFromLibrary(iconLibPath, SystemIcons::BinEmpty);      // Recycle Bin
        desktop.AddIconFromLibrary(iconLibPath, SystemIcons::FolderLibrary); // Library
        desktop.AddIconFromLibrary(iconLibPath, SystemIcons::DriveHdd);      // Hard Drive
        desktop.AddIconFromLibrary(iconLibPath, SystemIcons::StoreCdrom);    // CD-ROM
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

    // Load TTF font for window titles
    Font titleFont = Font::SystemFontBold();  // Default fallback
    const char* fontStatus = "FON";

    // Try Tahoma Bold first (check C: drive first for combo boot, then current dir)
    const char* fontPath = nullptr;
    if (IO::File::Exists("C:\\TAHOMABD.TTF")) {
        fontPath = "C:\\TAHOMABD.TTF";
    } else if (IO::File::Exists("TAHOMABD.TTF")) {
        fontPath = "TAHOMABD.TTF";
    } else if (IO::File::Exists("C:\\TAHOMA.TTF")) {
        fontPath = "C:\\TAHOMA.TTF";
    } else if (IO::File::Exists("TAHOMA.TTF")) {
        fontPath = "TAHOMA.TTF";
    }

    if (fontPath) {
        try {
            titleFont = Font::FromTrueType(fontPath, 12);  // 12pt like Windows
            fontStatus = "TTF";
        } catch (...) {
            fontStatus = "ERR";
        }
    } else {
        fontStatus = "FON";
    }

    // Create windows - these are floating (participatesInLayout = false)
    // so they can be freely positioned and dragged
    Window* window1 = new Window(&desktop, Rectangle(80, 60, 320, 240));
    // Title shows: TTF (success), FON (fallback), ERR (parse error), 404 (not found)
    String title1 = String(fontStatus) + String(": Spectrum");
    window1->SetTitle(title1);
    window1->SetFont(titleFont);

    Window* window2 = new Window(&desktop, Rectangle(450, 120, 280, 200));
    String title2 = String(fontStatus) + String(": Window 2");
    window2->SetTitle(title2);
    window2->SetFont(titleFont);

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

    // Window 2: More spectrum controls (cyan and magenta)
    window2->Layout().SetDirection(FlexDirection::Row)
                     .SetJustifyContent(JustifyContent::SpaceAround)
                     .SetAlignItems(AlignItems::Stretch)
                     .SetPadding(10);

    SpectrumControl* spectrum5 = new SpectrumControl(window2,
        Rectangle(0, 0, 60, 100), Color32::Cyan);
    spectrum5->Layout().SetFlexGrow(1).SetMargin(5);

    SpectrumControl* spectrum6 = new SpectrumControl(window2,
        Rectangle(0, 0, 60, 100), Color32::Magenta);
    spectrum6->Layout().SetFlexGrow(1).SetMargin(5);

    // Perform layout on window2
    window2->PerformLayout();

    // Suppress unused variable warnings
    (void)spectrum1;
    (void)spectrum2;
    (void)spectrum3;
    (void)spectrum4;
    (void)spectrum5;
    (void)spectrum6;

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
