/**
 * Icon Demo - Display icons from sysicons.icl in a grid
 *
 * This demo loads all icons from a Windows icon library file (.icl)
 * and displays them in a grid pattern on the desktop.
 */

#include "../src/rtcorlib.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::IO::Devices;
using namespace System::Windows::Forms;

// Icon grid control - displays icons in a grid pattern
class IconGrid : public Control {
private:
    Array<Image> _icons;
    int _iconSize;
    int _spacing;
    int _cols;

public:
    IconGrid(Control* parent, const Rectangle& bounds, int iconSize = 32, int spacing = 8)
        : Control(parent, bounds)
        , _icons()
        , _iconSize(iconSize)
        , _spacing(spacing)
        , _cols(1) {
        int bw = static_cast<int>(bounds.width);
        _cols = (bw - _spacing) / (_iconSize + _spacing);
        if (_cols < 1) _cols = 1;
    }

    virtual ~IconGrid() {}

    void AddIcon(const Image& icon) {
        int oldLen = _icons.Length();
        _icons.Resize(oldLen + 1);
        _icons[oldLen] = icon;
        Invalidate();
    }

    Int32 IconCount() const { return Int32(_icons.Length()); }

    virtual void OnPaint(PaintEventArgs& e) override {
        Rectangle screen = ScreenBounds();
        int sx = static_cast<int>(screen.x);
        int sy = static_cast<int>(screen.y);

        // Draw background
        e.graphics->FillRectangle(screen, Color::DarkCyan);

        // Draw each icon
        GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Image& fbImg = fb->GetImage();

        for (int i = 0; i < _icons.Length(); i++) {
            int col = i % _cols;
            int row = i / _cols;

            int iconX = sx + _spacing + col * (_iconSize + _spacing);
            int iconY = sy + _spacing + row * (_iconSize + _spacing);

            // Draw icon with alpha blending
            fbImg.CopyFromWithAlpha(_icons[i], iconX, iconY);
        }

        OnPaintClient(e);
    }
};

int main() {
    try {
        // Initialize display (VGA 640x480x4bpp)
        Display::SetMode(Display::VGA_640x480x4);
        GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);

        // Initialize input devices
        Mouse::Initialize();
        Mouse::SetBounds(0, 0, 639, 479);
        Mouse::SetPosition(320, 240);

        // Create desktop
        Desktop desktop(Color::DarkCyan);

        // Load cursor from icon library (Index 13 has a 24x24 arrow cursor)
        try {
            desktop.LoadCursorFromLibrary("sysicons.icl", 13);
        } catch (...) {
            // If cursor loading fails, desktop will use fallback cursor
        }

        // Create a window for the icon grid
        Window* iconWindow = new Window(&desktop, Rectangle(50, 30, 540, 400));

        // Create icon grid inside the window
        IconGrid* iconGrid = new IconGrid(
            iconWindow,
            Rectangle(5, 25, 525, 365),  // Inside window client area
            32,  // 32x32 icons
            12   // 12px spacing
        );

        // Load icons from sysicons.icl
        const char* iconLibPath = "sysicons.icl";
        Int32 iconCount = Image::GetIconLibraryCount(iconLibPath);

        Console::Write("Loading ");
        Console::Write(static_cast<int>(iconCount));
        Console::WriteLine(" icons from sysicons.icl...");

        for (int i = 0; i < static_cast<int>(iconCount); i++) {
            try {
                Image icon = Image::FromIconLibrary(iconLibPath, i, Size::IconMedium);
                iconGrid->AddIcon(icon);
            } catch (const Exception& e) {
                // Skip icons that fail to load
                Console::Write("Failed to load icon ");
                Console::Write(i);
                Console::Write(": ");
                Console::WriteLine(e.Message());
            }
        }

        Console::Write("Loaded ");
        Console::Write(static_cast<int>(iconGrid->IconCount()));
        Console::WriteLine(" icons successfully.");
        Console::WriteLine("Press ESC to exit.");

        // Run the desktop
        desktop.Run();

        // Cleanup
        GraphicsBuffer::DestroyFrameBuffer();
        Display::SetMode(Display::TextMode);

        Console::WriteLine("Icon demo completed.");
        return 0;

    } catch (const Exception& e) {
        // Return to text mode on error
        Display::SetMode(Display::TextMode);
        Console::SetForegroundColor(ConsoleColor::Red);
        Console::Write("Error: ");
        Console::WriteLine(e.Message());
        Console::ResetColor();
        return 1;
    }
}
