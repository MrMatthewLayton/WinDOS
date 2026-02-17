// Graphics Demo - BCL VGA Mode 0x12 (640x480x4bpp)
// Loads a BMP background, draws a white mouse cursor with black border
// Optimized with dirty rectangles - only redraws changed regions

#include "../src/BCL.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Devices;

int main() {
    // Initialize mouse
    if (!Mouse::Initialize()) {
        Console::WriteLine("Mouse not available!");
        Console::WriteLine("Press any key to exit...");
        Console::ReadKey(true);
        return 1;
    }

    // Set VGA mode 0x12 (640x480, 16 colors)
    Display::SetMode(Display::VGA_640x480x4);

    // Create frame buffer for this display mode
    GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);

    // Create graphics context for the full screen (single buffer mode for speed)
    Graphics gfx(BufferMode::Single, 0, 0, 640, 480);

    // Load background image from BMP file
    Image background = Image::FromBitmap("c:\\assets\\test.bmp");
    if (background.Width() > 0) {
        gfx.DrawImage(background, 0, 0);
    } else {
        // Fallback to cyan if BMP fails to load
        gfx.Clear(Color::Cyan);
    }
    GraphicsBuffer::FlushFrameBuffer();

    // Hide hardware mouse cursor - we'll draw our own
    Mouse::HideCursor();

    // Set mouse bounds to screen dimensions
    Mouse::SetBounds(0, 0, 639, 479);
    Mouse::SetPosition(320, 240);  // Center of screen

    // Cursor properties
    const int CURSOR_SIZE = 16;
    int lastMouseX = -1;
    int lastMouseY = -1;

    // Main loop - no vsync wait, dirty rectangles handle efficiency
    bool running = true;
    while (running) {
        // Get mouse status
        MouseStatus mouse = Mouse::GetStatus();
        int mouseX = mouse.x;
        int mouseY = mouse.y;

        // Check for exit (right mouse button or keyboard)
        if (mouse.rightButton || Keyboard::IsKeyPressed()) {
            running = false;
            break;
        }

        // Check if mouse moved
        bool mouseMoved = (mouseX != lastMouseX || mouseY != lastMouseY);

        if (mouseMoved) {
            // Restore background at old cursor position
            if (lastMouseX >= 0 && lastMouseY >= 0) {
                if (background.Width() > 0) {
                    // Restore from background image
                    Image region = background.GetRegion(lastMouseX, lastMouseY, CURSOR_SIZE, CURSOR_SIZE);
                    gfx.DrawImage(region, lastMouseX, lastMouseY);
                } else {
                    // Fallback to cyan
                    gfx.FillRectangle(lastMouseX, lastMouseY, CURSOR_SIZE, CURSOR_SIZE, Color::Cyan);
                }
            }

            // Draw cursor at new position: white fill with black border
            gfx.FillRectangle(mouseX, mouseY, CURSOR_SIZE, CURSOR_SIZE, Color::White);
            gfx.DrawRectangle(mouseX, mouseY, CURSOR_SIZE, CURSOR_SIZE, Color::Black);

            // Update display - dirty rectangles only update changed areas
            GraphicsBuffer::FlushFrameBuffer();

            // Update last position
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    }

    // Cleanup: destroy frame buffer
    GraphicsBuffer::DestroyFrameBuffer();

    // Return to text mode
    Display::SetDefaultMode();

    Console::WriteLine("Graphics demo finished.");
    Console::WriteLine("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
