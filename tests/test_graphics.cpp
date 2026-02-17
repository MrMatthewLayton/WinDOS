// Graphics test suite for BCL
// Tests Point, Size, Rectangle, Color, Image, and basic Graphics operations

#include "test_framework.hpp"
#include "../src/System/Drawing/Drawing.hpp"
#include "../src/System/Devices/Devices.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Devices;

void TestColor() {
    Test::PrintHeader("Color");

    Color c1;
    ASSERT_EQ(0, static_cast<int>(c1.Value()), "Default color is black (0)");

    Color c2(5);
    ASSERT_EQ(5, static_cast<int>(c2.Value()), "Color from value");

    Color c3 = Color::Cyan;
    ASSERT_EQ(0x0B, static_cast<int>(c3.Value()), "Color::Cyan is 0x0B");

    Color c4 = c3;
    ASSERT(c4 == c3, "Color copy constructor");

    ASSERT(Color::Black != Color::White, "Black != White");
    ASSERT(Color::Red == Color::Red, "Red == Red");

    ASSERT_EQ(0x0F, static_cast<int>(Color::White.Value()), "Color::White is 0x0F");
    ASSERT_EQ(0xFF, static_cast<int>(Color::Transparent.Value()), "Color::Transparent is 0xFF");

    Test::PrintSummary();
}

void TestPoint() {
    Test::PrintHeader("Point");

    Point p1;
    ASSERT_EQ(0, p1.x, "Default point x is 0");
    ASSERT_EQ(0, p1.y, "Default point y is 0");

    Point p2(10, 20);
    ASSERT_EQ(10, p2.x, "Point x from constructor");
    ASSERT_EQ(20, p2.y, "Point y from constructor");

    Point p3 = p2;
    ASSERT(p3 == p2, "Point copy constructor");

    Point p4 = p2.Offset(5, -5);
    ASSERT_EQ(15, p4.x, "Point offset x");
    ASSERT_EQ(15, p4.y, "Point offset y");

    ASSERT(Point::Empty == Point(0, 0), "Point::Empty is (0,0)");

    Test::PrintSummary();
}

void TestSize() {
    Test::PrintHeader("Size");

    Size s1;
    ASSERT_EQ(0, s1.width, "Default size width is 0");
    ASSERT_EQ(0, s1.height, "Default size height is 0");
    ASSERT(s1.IsEmpty(), "Default size is empty");

    Size s2(100, 200);
    ASSERT_EQ(100, s2.width, "Size width from constructor");
    ASSERT_EQ(200, s2.height, "Size height from constructor");
    ASSERT(!s2.IsEmpty(), "Non-zero size is not empty");

    Size s3 = s2;
    ASSERT(s3 == s2, "Size copy constructor");

    Test::PrintSummary();
}

void TestRectangle() {
    Test::PrintHeader("Rectangle");

    Rectangle r1;
    ASSERT_EQ(0, r1.x, "Default rect x is 0");
    ASSERT_EQ(0, r1.y, "Default rect y is 0");
    ASSERT_EQ(0, r1.width, "Default rect width is 0");
    ASSERT_EQ(0, r1.height, "Default rect height is 0");

    Rectangle r2(10, 20, 100, 50);
    ASSERT_EQ(10, r2.x, "Rect x from constructor");
    ASSERT_EQ(20, r2.y, "Rect y from constructor");
    ASSERT_EQ(100, r2.width, "Rect width from constructor");
    ASSERT_EQ(50, r2.height, "Rect height from constructor");

    ASSERT_EQ(10, r2.Left(), "Rect Left()");
    ASSERT_EQ(20, r2.Top(), "Rect Top()");
    ASSERT_EQ(110, r2.Right(), "Rect Right()");
    ASSERT_EQ(70, r2.Bottom(), "Rect Bottom()");

    ASSERT(r2.Contains(50, 40), "Contains point inside");
    ASSERT(r2.Contains(10, 20), "Contains point on top-left corner");
    ASSERT(!r2.Contains(110, 70), "Does not contain point on bottom-right (exclusive)");
    ASSERT(!r2.Contains(5, 40), "Does not contain point outside left");

    Point loc = r2.Location();
    ASSERT_EQ(10, loc.x, "Location().x");
    ASSERT_EQ(20, loc.y, "Location().y");

    Size sz = r2.GetSize();
    ASSERT_EQ(100, sz.width, "GetSize().width");
    ASSERT_EQ(50, sz.height, "GetSize().height");

    Rectangle r3 = r2.Offset(5, 10);
    ASSERT_EQ(15, r3.x, "Offset rect x");
    ASSERT_EQ(30, r3.y, "Offset rect y");

    Test::PrintSummary();
}

void TestImage() {
    Test::PrintHeader("Image");

    Image img1;
    ASSERT_EQ(0, img1.Width(), "Default image width is 0");
    ASSERT_EQ(0, img1.Height(), "Default image height is 0");

    Image img2(10, 20, Color::Blue);
    ASSERT_EQ(10, img2.Width(), "Image width from constructor");
    ASSERT_EQ(20, img2.Height(), "Image height from constructor");
    ASSERT_EQ(200, img2.Length(), "Image length is width * height");
    ASSERT_EQ(static_cast<int>(Color::Blue.Value()), static_cast<int>(img2.GetPixel(0, 0)), "Filled with blue");
    ASSERT_EQ(static_cast<int>(Color::Blue.Value()), static_cast<int>(img2.GetPixel(9, 19)), "Last pixel is blue");

    img2.SetPixel(5, 5, Color::Red);
    ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(img2.GetPixel(5, 5)), "SetPixel works");

    // Out of bounds returns transparent
    ASSERT_EQ(static_cast<int>(Color::Transparent.Value()), static_cast<int>(img2.GetPixel(-1, 0)), "Out of bounds returns transparent");
    ASSERT_EQ(static_cast<int>(Color::Transparent.Value()), static_cast<int>(img2.GetPixel(100, 0)), "Out of bounds returns transparent (right)");

    Image img3 = img2;
    ASSERT_EQ(10, img3.Width(), "Copy constructor width");
    ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(img3.GetPixel(5, 5)), "Copy constructor copies pixels");

    img3.Clear(Color::Green);
    ASSERT_EQ(static_cast<int>(Color::Green.Value()), static_cast<int>(img3.GetPixel(0, 0)), "Clear sets all pixels");
    ASSERT_EQ(static_cast<int>(Color::Green.Value()), static_cast<int>(img3.GetPixel(5, 5)), "Clear sets all pixels");

    // Test GetRegion
    Image img4(20, 20, Color::Yellow);
    img4.SetPixel(5, 5, Color::Red);
    img4.SetPixel(6, 6, Color::Blue);
    Image region = img4.GetRegion(4, 4, 5, 5);
    ASSERT_EQ(5, region.Width(), "Region width");
    ASSERT_EQ(5, region.Height(), "Region height");
    ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(region.GetPixel(1, 1)), "Region contains copied pixels");

    Test::PrintSummary();
}

void TestDisplay() {
    Test::PrintHeader("Display");

    Display d = Display::VGA_640x480x4;
    ASSERT_EQ(0x12, static_cast<int>(d.Mode()), "VGA_640x480x4 mode is 0x12");
    ASSERT_EQ(4, static_cast<int>(d.BitsPerPixel()), "VGA_640x480x4 is 4bpp");
    ASSERT_EQ(640, static_cast<int>(d.Width()), "VGA_640x480x4 width is 640");
    ASSERT_EQ(480, static_cast<int>(d.Height()), "VGA_640x480x4 height is 480");

    Display d2 = Display::VGA_320x200x8;
    ASSERT_EQ(0x13, static_cast<int>(d2.Mode()), "VGA_320x200x8 mode is 0x13");
    ASSERT_EQ(8, static_cast<int>(d2.BitsPerPixel()), "VGA_320x200x8 is 8bpp");

    Display d3 = Display::TextMode;
    ASSERT_EQ(0x03, static_cast<int>(d3.Mode()), "TextMode mode is 0x03");

    Test::PrintSummary();
}

void TestGraphicsBuffer() {
    Test::PrintHeader("GraphicsBuffer");

    // Note: We can't fully test graphics buffer without setting video mode
    // Just test that the API works without crashing

    GraphicsBuffer::CreateFrameBuffer(100, 100, 0x12);
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    ASSERT(fb != nullptr, "CreateFrameBuffer creates a buffer");

    Rectangle bounds = fb->Bounds();
    ASSERT_EQ(0, bounds.x, "Frame buffer x is 0");
    ASSERT_EQ(0, bounds.y, "Frame buffer y is 0");
    ASSERT_EQ(100, bounds.width, "Frame buffer width matches");
    ASSERT_EQ(100, bounds.height, "Frame buffer height matches");

    Image& img = fb->GetImage();
    ASSERT_EQ(100, img.Width(), "Frame buffer image width");
    ASSERT_EQ(100, img.Height(), "Frame buffer image height");

    GraphicsBuffer::DestroyFrameBuffer();
    fb = GraphicsBuffer::GetFrameBuffer();
    ASSERT(fb == nullptr, "DestroyFrameBuffer clears the buffer");

    Test::PrintSummary();
}

void TestGraphics() {
    Test::PrintHeader("Graphics");

    // Create a small test buffer
    GraphicsBuffer::CreateFrameBuffer(100, 100, 0x12);

    Graphics gfx(BufferMode::Single, 0, 0, 100, 100);

    Rectangle bounds = gfx.Bounds();
    ASSERT_EQ(100, bounds.width, "Graphics bounds width");
    ASSERT_EQ(100, bounds.height, "Graphics bounds height");

    // Test operations don't crash
    gfx.Clear(Color::Black);
    gfx.DrawPixel(50, 50, Color::White);
    gfx.DrawLine(0, 0, 99, 99, Color::Red);
    gfx.DrawRectangle(10, 10, 20, 20, Color::Green);
    gfx.FillRectangle(30, 30, 10, 10, Color::Blue);

    // Verify pixels in frame buffer
    GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
    if (fb) {
        Image& img = fb->GetImage();
        ASSERT_EQ(static_cast<int>(Color::White.Value()), static_cast<int>(img.GetPixel(50, 50)), "DrawPixel works");
        ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(img.GetPixel(0, 0)), "DrawLine starts at origin");
        ASSERT_EQ(static_cast<int>(Color::Blue.Value()), static_cast<int>(img.GetPixel(35, 35)), "FillRectangle fills interior");
    }

    GraphicsBuffer::DestroyFrameBuffer();

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Graphics Test Suite");
    Console::WriteLine("=======================");
    Console::ResetColor();
    Console::WriteLine();

    TestColor();
    TestPoint();
    TestSize();
    TestRectangle();
    TestImage();
    TestDisplay();
    TestGraphicsBuffer();
    TestGraphics();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All graphics tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
