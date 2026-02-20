// Forms Demo - Windows 95 style WIMP system demo
// Tests Desktop, Window, TaskBar, Button, StartMenu, and layout system

#include "../src/rtcorlib.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::IO::Devices;
using namespace System::Windows::Forms;

int main()
{
    // Initialize mouse
    Mouse::Initialize();

    // Slow down mouse (default is horizontal=8, vertical=16; higher = slower)
    Mouse::SetSensitivity(16, 32);

    // Target 800x600x32 VBE mode
    int screenWidth = 800;
    int screenHeight = 600;
    bool useVbe = false;

    if (static_cast<bool>(Display::IsVbeAvailable()))
    {
        Display vbeMode = Display::DetectVbeMode(UInt16(800), UInt16(600), UInt8(32));
        if (static_cast<bool>(vbeMode.IsVbeMode()))
        {
            // VBE mode found - try to use it
            Display::SetMode(vbeMode);

            // Check if LFB was successfully mapped
            void *lfb = Display::GetMappedLfb();
            if (lfb)
            {
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
    if (!useVbe)
    {
        Display::SetMode(Display::VGA_640x480x4);
        GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
        screenWidth = 640;
        screenHeight = 480;
    }

    // Hide hardware cursor (we draw our own)
    Mouse::HideCursor();

    // Set mouse bounds to screen
    Mouse::SetBounds(0, 0, screenWidth - 1, screenHeight - 1);

    // Create desktop with teal background (Windows 95 style)
    Desktop desktop(Color(UInt8(1), UInt8(130), UInt8(129)));

    // Try to load wallpaper image
    const char *wallpaperPath = nullptr;
    if (IO::File::Exists("C:\\BOOT.PNG"))
    {
        wallpaperPath = "C:\\BOOT.PNG";
    } else if (IO::File::Exists("BOOT.PNG"))
    {
        wallpaperPath = "BOOT.PNG";
    } else if (IO::File::Exists("C:\\BOOT.JPG"))
    {
        wallpaperPath = "C:\\BOOT.JPG";
    } else if (IO::File::Exists("BOOT.JPG"))
    {
        wallpaperPath = "BOOT.JPG";
    }

    if (wallpaperPath)
    {
        try
        {
            Image wallpaper = Image::FromFile(wallpaperPath);
            desktop.SetWallpaper(wallpaper);
        } catch (...)
        {
            // If wallpaper loading fails, use default background color
        }
    }

    // Determine icon library path (try C: drive first for combo boot, then current dir)
    const char *iconLibPath = "C:\\SYSICONS.ICL";
    if (!IO::File::Exists(iconLibPath))
    {
        iconLibPath = "SYSICONS.ICL"; // Fall back to current directory
    }

    // Load icon library and set on desktop
    try
    {
        IconLibrary* iconLib = IconLibrary::FromFile(iconLibPath);
        desktop.SetIconLibrary(iconLib);
    }
    catch (...)
    {
        // If icon library loading fails, continue without it
    }

    // Load cursor from icon library
    try
    {
        desktop.LoadCursorFromLibrary(iconLibPath, "cursor-pointer");
    } catch (...)
    {
        // If cursor loading fails, desktop will use fallback cursor
    }

    // Add desktop icons (32x32 icons from sysicons.icl) with text labels
    try
    {
        desktop.AddIconFromLibrary(iconLibPath, "computer", String("Computer"));
        desktop.AddIconFromLibrary(iconLibPath, "bin-empty", String("Recycle Bin"));
        desktop.AddIconFromLibrary(iconLibPath, "folder-docs", String("Documents"));
        desktop.AddIconFromLibrary(iconLibPath, "drive-hdd", String("Hard Disk"));
        desktop.AddIconFromLibrary(iconLibPath, "store-cdrom", String("Disc"));
    } catch (...)
    {
        // If icon loading fails, continue without icons
    }

    // Create start menu FIRST (before taskbar)
    StartMenu *startMenu = new StartMenu(&desktop);
    desktop.SetStartMenu(startMenu);

    // Create taskbar with start menu reference
    // TaskBar uses row layout internally with gap=2, alignItems=Center
    TaskBar *taskBar = new TaskBar(&desktop, startMenu);
    desktop.SetTaskBar(taskBar);
    taskBar->SetDesktop(&desktop);
    taskBar->LoadIcons();

    // Load TTF font for window titles
    Font titleFont = Font::SystemFontBold(); // Default fallback

    // Try Tahoma Bold first (check C: drive first for combo boot, then current dir)
    const char *fontPath = nullptr;
    if (IO::File::Exists("C:\\TAHOMABD.TTF"))
    {
        fontPath = "C:\\TAHOMABD.TTF";
    } else if (IO::File::Exists("TAHOMABD.TTF"))
    {
        fontPath = "TAHOMABD.TTF";
    } else if (IO::File::Exists("C:\\TAHOMA.TTF"))
    {
        fontPath = "C:\\TAHOMA.TTF";
    } else if (IO::File::Exists("TAHOMA.TTF"))
    {
        fontPath = "TAHOMA.TTF";
    }

    if (fontPath)
    {
        try
        {
            titleFont = Font::FromTrueType(fontPath, 12); // 12pt like Windows
        } catch (...)
        {
            // Keep default FON font on error
        }
    }

    // Create windows - these are floating (participatesInLayout = false)
    // so they can be freely positioned and dragged
    // Window 1: 6 spectrum controls × 64px = 384px client area + borders (~4px each side)
    Window *window1 = new Window(&desktop, Rectangle(420, 60, 392, 300));
    window1->SetTitle(String("WinDOS 2026"));
    window1->SetFont(titleFont);
    window1->LoadButtonIcons();

    // Window 2: System icons viewer (640x480) with white background
    Window *window2 = new Window(&desktop, Rectangle(20, 20, 640, 480));
    window2->SetTitle(String("System Window"));
    window2->SetFont(titleFont);
    window2->SetBackColor(Color::White);
    window2->LoadButtonIcons();

    // Window 1: Row layout, no padding/margin, spectrums fill entire client area
    window1->Layout().SetDirection(FlexDirection::Row)
            .SetJustifyContent(JustifyContent::Start)
            .SetAlignItems(AlignItems::Stretch)
            .SetPadding(0);

    // Create 6 spectrum controls - each 64px wide, filling window height
    SpectrumControl *spectrum1 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Red);
    spectrum1->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    SpectrumControl *spectrum2 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Green);
    spectrum2->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    SpectrumControl *spectrum3 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Blue);
    spectrum3->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    SpectrumControl *spectrum4 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Yellow);
    spectrum4->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    SpectrumControl *spectrum5 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Cyan);
    spectrum5->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    SpectrumControl *spectrum6 = new SpectrumControl(window1,
                                                     Rectangle(0, 0, 64, 100), Color::Magenta);
    spectrum6->Layout().SetWidthMode(SizeMode::Fixed).SetHeightMode(SizeMode::Fill).SetMargin(0);

    // Perform layout on window1 to arrange spectrum controls
    window1->PerformLayout();

    // Window 2: Grid of all system icons from sysicons.icl
    // Each icon cell is 40x40 (32px icon centered)
    const int ICON_CELL_SIZE = 40;
    const int ICONS_PER_ROW = 15;  // ~600px / 40 = 15 columns
    const int NUM_ICONS = 98;      // Total icons in sysicons.icl

    try
    {
        for (int i = 0; i < NUM_ICONS; i++)
        {
            int col = i % ICONS_PER_ROW;
            int row = i / ICONS_PER_ROW;
            int x = col * ICON_CELL_SIZE + 4;  // 4px left margin
            int y = row * ICON_CELL_SIZE + 4;  // 4px top margin (inside client area)

            // Load icon at index i (32x32) using desktop's icon library
            IconLibrary* iconLib = desktop.GetIconLibrary();
            Image icon = iconLib ? iconLib->FromIndex(Int32(i), IconSize::Medium)
                                 : Image(Int32(32), Int32(32));

            // Create Picture control with icon centered in cell
            Picture *pic = new Picture(window2, Rectangle(x + 4, y + 4, 32, 32), icon);
            (void) pic;  // Suppress unused warning
        }
    }
    catch (...)
    {
        // If icon loading fails, continue without icons
    }

    // Suppress unused variable warnings
    (void) spectrum1;
    (void) spectrum2;
    (void) spectrum3;
    (void) spectrum4;
    (void) spectrum5;
    (void) spectrum6;

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
