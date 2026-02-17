// Extended Drawing tests for BCL
// Tests additional Drawing features not covered in test_graphics.cpp

#include "test_framework.hpp"
#include "../bcl/System/Drawing.hpp"

using namespace System;
using namespace System::Drawing;

void TestColorFromRgb() {
    Test::PrintHeader("Color::FromRgb");

    // FromRgb finds the closest VGA color
    // Black (0, 0, 0) -> 0
    unsigned char black = Color::FromRgb(0, 0, 0);
    ASSERT_EQ(0, static_cast<int>(black), "FromRgb black");

    // White (255, 255, 255) -> 15
    unsigned char white = Color::FromRgb(255, 255, 255);
    ASSERT_EQ(15, static_cast<int>(white), "FromRgb white");

    // Red (255, 0, 0) -> should be close to 12 (bright red) or 4 (dark red)
    unsigned char red = Color::FromRgb(255, 0, 0);
    ASSERT(red == 12 || red == 4, "FromRgb red is red or dark red");

    // Green (0, 255, 0) -> should be close to 10 (bright green) or 2 (dark green)
    unsigned char green = Color::FromRgb(0, 255, 0);
    ASSERT(green == 10 || green == 2, "FromRgb green is green or dark green");

    // Blue (0, 0, 255) -> should be close to 9 (bright blue) or 1 (dark blue)
    unsigned char blue = Color::FromRgb(0, 0, 255);
    ASSERT(blue == 9 || blue == 1, "FromRgb blue is blue or dark blue");

    // Gray (128, 128, 128) -> should be 7 (gray) or 8 (dark gray)
    unsigned char gray = Color::FromRgb(128, 128, 128);
    ASSERT(gray == 7 || gray == 8, "FromRgb gray is gray or dark gray");

    // Yellow (255, 255, 0) -> should be 14 (yellow) or 6 (dark yellow/brown)
    unsigned char yellow = Color::FromRgb(255, 255, 0);
    ASSERT(yellow == 14 || yellow == 6, "FromRgb yellow");

    // Cyan (0, 255, 255) -> should be 11 (cyan) or 3 (dark cyan)
    unsigned char cyan = Color::FromRgb(0, 255, 255);
    ASSERT(cyan == 11 || cyan == 3, "FromRgb cyan");

    Test::PrintSummary();
}

void TestRectangleInflate() {
    Test::PrintHeader("Rectangle::Inflate");

    Rectangle r1(50, 50, 100, 100);

    // Inflate by positive values (grows)
    Rectangle r2 = r1.Inflate(10, 20);
    ASSERT_EQ(40, r2.x, "Inflate grows: x decreased");
    ASSERT_EQ(30, r2.y, "Inflate grows: y decreased");
    ASSERT_EQ(120, r2.width, "Inflate grows: width increased by 2*dx");
    ASSERT_EQ(140, r2.height, "Inflate grows: height increased by 2*dy");

    // Original unchanged
    ASSERT_EQ(50, r1.x, "Original x unchanged");
    ASSERT_EQ(100, r1.width, "Original width unchanged");

    // Inflate by negative values (shrinks)
    Rectangle r3 = r1.Inflate(-10, -10);
    ASSERT_EQ(60, r3.x, "Inflate shrinks: x increased");
    ASSERT_EQ(60, r3.y, "Inflate shrinks: y increased");
    ASSERT_EQ(80, r3.width, "Inflate shrinks: width decreased");
    ASSERT_EQ(80, r3.height, "Inflate shrinks: height decreased");

    // Inflate with zero
    Rectangle r4 = r1.Inflate(0, 0);
    ASSERT_EQ(50, r4.x, "Inflate zero: x same");
    ASSERT_EQ(100, r4.width, "Inflate zero: width same");

    Test::PrintSummary();
}

void TestImageCopyFrom() {
    Test::PrintHeader("Image::CopyFrom");

    // Create source image with pattern
    Image src(10, 10, Color::Red);
    src.SetPixel(5, 5, Color::Blue);
    src.SetPixel(0, 0, Color::Green);

    // Create destination image
    Image dst(20, 20, Color::Black);

    // Copy source to destination at offset
    dst.CopyFrom(src, 5, 5);

    // Verify copied pixels
    ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(dst.GetPixel(5, 5)), "CopyFrom corner pixel");
    ASSERT_EQ(static_cast<int>(Color::Blue.Value()), static_cast<int>(dst.GetPixel(10, 10)), "CopyFrom center pixel (5+5)");
    ASSERT_EQ(static_cast<int>(Color::Green.Value()), static_cast<int>(dst.GetPixel(5, 5)), "CopyFrom (0,0) goes to (5,5)");

    // Original destination pixels outside copy area unchanged
    ASSERT_EQ(static_cast<int>(Color::Black.Value()), static_cast<int>(dst.GetPixel(0, 0)), "CopyFrom doesn't affect (0,0)");
    ASSERT_EQ(static_cast<int>(Color::Black.Value()), static_cast<int>(dst.GetPixel(19, 19)), "CopyFrom doesn't affect far corner");

    // Copy with Point overload
    Image dst2(20, 20, Color::White);
    dst2.CopyFrom(src, Point(3, 3));
    ASSERT_EQ(static_cast<int>(Color::Red.Value()), static_cast<int>(dst2.GetPixel(3, 3)), "CopyFrom with Point");

    Test::PrintSummary();
}

void TestImageMoveSemantics() {
    Test::PrintHeader("Image Move Semantics");

    // Create source image
    Image src(50, 50, Color::Cyan);
    src.SetPixel(25, 25, Color::Magenta);

    // Move construct
    Image moved(static_cast<Image&&>(src));
    ASSERT_EQ(50, moved.Width(), "Move constructor width");
    ASSERT_EQ(50, moved.Height(), "Move constructor height");
    ASSERT_EQ(static_cast<int>(Color::Magenta.Value()), static_cast<int>(moved.GetPixel(25, 25)), "Move constructor preserves pixels");

    // Source should be empty after move
    ASSERT_EQ(0, src.Width(), "Moved-from source width is 0");
    ASSERT_EQ(0, src.Height(), "Moved-from source height is 0");

    // Move assign
    Image another(30, 30, Color::Yellow);
    another = static_cast<Image&&>(moved);
    ASSERT_EQ(50, another.Width(), "Move assignment width");
    ASSERT_EQ(static_cast<int>(Color::Magenta.Value()), static_cast<int>(another.GetPixel(25, 25)), "Move assignment preserves pixels");
    ASSERT_EQ(0, moved.Width(), "Moved-from width is 0 after assignment");

    Test::PrintSummary();
}

void TestImageGetRegionEdgeCases() {
    Test::PrintHeader("Image GetRegion Edge Cases");

    Image img(20, 20, Color::White);
    for (int x = 0; x < 20; x++) {
        for (int y = 0; y < 20; y++) {
            img.SetPixel(x, y, Color((unsigned char)((x + y) % 16)));
        }
    }

    // Get region from corner
    Image corner = img.GetRegion(0, 0, 5, 5);
    ASSERT_EQ(5, corner.Width(), "Corner region width");
    ASSERT_EQ(5, corner.Height(), "Corner region height");
    ASSERT_EQ(static_cast<int>(img.GetPixel(0, 0)), static_cast<int>(corner.GetPixel(0, 0)), "Corner pixel matches");

    // Get region from opposite corner
    Image farCorner = img.GetRegion(15, 15, 5, 5);
    ASSERT_EQ(5, farCorner.Width(), "Far corner region width");
    ASSERT_EQ(static_cast<int>(img.GetPixel(15, 15)), static_cast<int>(farCorner.GetPixel(0, 0)), "Far corner pixel matches");

    // Get region with Rectangle overload
    Image rectRegion = img.GetRegion(Rectangle(5, 5, 10, 10));
    ASSERT_EQ(10, rectRegion.Width(), "Rectangle region width");
    ASSERT_EQ(10, rectRegion.Height(), "Rectangle region height");

    // Get entire image as region
    Image full = img.GetRegion(0, 0, 20, 20);
    ASSERT_EQ(20, full.Width(), "Full region width");
    ASSERT_EQ(20, full.Height(), "Full region height");

    // Get 1x1 region
    Image pixel = img.GetRegion(10, 10, 1, 1);
    ASSERT_EQ(1, pixel.Width(), "1x1 region width");
    ASSERT_EQ(1, pixel.Height(), "1x1 region height");

    Test::PrintSummary();
}

void TestBorderStyleEnum() {
    Test::PrintHeader("BorderStyle Enum");

    // Verify enum values exist and can be used
    BorderStyle none = BorderStyle::None;
    BorderStyle flat = BorderStyle::Flat;
    BorderStyle raised = BorderStyle::Raised;
    BorderStyle sunken = BorderStyle::Sunken;
    BorderStyle raisedDouble = BorderStyle::RaisedDouble;
    BorderStyle sunkenDouble = BorderStyle::SunkenDouble;
    BorderStyle window = BorderStyle::Window;

    // Comparison
    ASSERT(none != flat, "None != Flat");
    ASSERT(raised != sunken, "Raised != Sunken");
    ASSERT(none == BorderStyle::None, "None == None");
    ASSERT(window == BorderStyle::Window, "Window == Window");

    // Assignment
    BorderStyle style = BorderStyle::Raised;
    ASSERT(style == BorderStyle::Raised, "Assignment works");
    style = BorderStyle::Sunken;
    ASSERT(style == BorderStyle::Sunken, "Reassignment works");

    // Suppress unused variable warnings
    (void)none; (void)flat; (void)raised; (void)sunken;
    (void)raisedDouble; (void)sunkenDouble; (void)window;

    Test::PrintSummary();
}

void TestBufferModeEnum() {
    Test::PrintHeader("BufferMode Enum");

    BufferMode single = BufferMode::Single;
    BufferMode dbl = BufferMode::Double;

    ASSERT(single != dbl, "Single != Double");
    ASSERT(single == BufferMode::Single, "Single == Single");
    ASSERT(dbl == BufferMode::Double, "Double == Double");

    // Assignment
    BufferMode mode = BufferMode::Single;
    ASSERT(mode == BufferMode::Single, "Assignment works");
    mode = BufferMode::Double;
    ASSERT(mode == BufferMode::Double, "Reassignment works");

    Test::PrintSummary();
}

void TestPointArithmetic() {
    Test::PrintHeader("Point Arithmetic");

    Point p1(10, 20);
    Point p2(5, 10);

    // Offset (already tested, but more cases)
    Point offset1 = p1.Offset(0, 0);
    ASSERT_EQ(10, offset1.x, "Offset by zero x");
    ASSERT_EQ(20, offset1.y, "Offset by zero y");

    Point offset2 = p1.Offset(-20, -30);
    ASSERT_EQ(-10, offset2.x, "Offset negative x");
    ASSERT_EQ(-10, offset2.y, "Offset negative y");

    // Large values
    Point large(100000, 200000);
    Point largeOffset = large.Offset(50000, 50000);
    ASSERT_EQ(150000, largeOffset.x, "Large offset x");
    ASSERT_EQ(250000, largeOffset.y, "Large offset y");

    Test::PrintSummary();
}

void TestSizeEdgeCases() {
    Test::PrintHeader("Size Edge Cases");

    // Zero size
    Size zero(0, 0);
    ASSERT(zero.IsEmpty(), "Zero size is empty");

    // One dimension zero
    Size wideZero(100, 0);
    ASSERT(wideZero.IsEmpty(), "Width-only size is empty");

    Size tallZero(0, 100);
    ASSERT(tallZero.IsEmpty(), "Height-only size is empty");

    // Non-zero
    Size normal(100, 100);
    ASSERT(!normal.IsEmpty(), "Normal size not empty");

    // Large values
    Size large(65535, 65535);
    ASSERT_EQ(65535, large.width, "Large width");
    ASSERT_EQ(65535, large.height, "Large height");
    ASSERT(!large.IsEmpty(), "Large size not empty");

    Test::PrintSummary();
}

void TestRectangleContainsEdgeCases() {
    Test::PrintHeader("Rectangle Contains Edge Cases");

    Rectangle r(10, 10, 100, 100);

    // All four corners
    ASSERT(r.Contains(10, 10), "Contains top-left corner");
    ASSERT(r.Contains(109, 10), "Contains just inside right edge");
    ASSERT(r.Contains(10, 109), "Contains just inside bottom edge");
    ASSERT(r.Contains(109, 109), "Contains just inside bottom-right");

    // Just outside (Right and Bottom are exclusive)
    ASSERT(!r.Contains(110, 10), "Not contains on right edge");
    ASSERT(!r.Contains(10, 110), "Not contains on bottom edge");
    ASSERT(!r.Contains(110, 110), "Not contains on bottom-right corner");

    // Just before left/top
    ASSERT(!r.Contains(9, 10), "Not contains just left of left edge");
    ASSERT(!r.Contains(10, 9), "Not contains just above top edge");

    // Center
    ASSERT(r.Contains(60, 60), "Contains center");

    // Zero-size rectangle
    Rectangle zero(10, 10, 0, 0);
    ASSERT(!zero.Contains(10, 10), "Zero-size contains nothing");

    Test::PrintSummary();
}

void TestColorConstants() {
    Test::PrintHeader("Color Constants");

    // Verify all 16 colors have correct values
    ASSERT_EQ(0, static_cast<int>(Color::Black.Value()), "Black is 0");
    ASSERT_EQ(1, static_cast<int>(Color::DarkBlue.Value()), "DarkBlue is 1");
    ASSERT_EQ(2, static_cast<int>(Color::DarkGreen.Value()), "DarkGreen is 2");
    ASSERT_EQ(3, static_cast<int>(Color::DarkCyan.Value()), "DarkCyan is 3");
    ASSERT_EQ(4, static_cast<int>(Color::DarkRed.Value()), "DarkRed is 4");
    ASSERT_EQ(5, static_cast<int>(Color::DarkMagenta.Value()), "DarkMagenta is 5");
    ASSERT_EQ(6, static_cast<int>(Color::DarkYellow.Value()), "DarkYellow is 6");
    ASSERT_EQ(7, static_cast<int>(Color::Gray.Value()), "Gray is 7");
    ASSERT_EQ(8, static_cast<int>(Color::DarkGray.Value()), "DarkGray is 8");
    ASSERT_EQ(9, static_cast<int>(Color::Blue.Value()), "Blue is 9");
    ASSERT_EQ(10, static_cast<int>(Color::Green.Value()), "Green is 10");
    ASSERT_EQ(11, static_cast<int>(Color::Cyan.Value()), "Cyan is 11");
    ASSERT_EQ(12, static_cast<int>(Color::Red.Value()), "Red is 12");
    ASSERT_EQ(13, static_cast<int>(Color::Magenta.Value()), "Magenta is 13");
    ASSERT_EQ(14, static_cast<int>(Color::Yellow.Value()), "Yellow is 14");
    ASSERT_EQ(15, static_cast<int>(Color::White.Value()), "White is 15");
    ASSERT_EQ(255, static_cast<int>(Color::Transparent.Value()), "Transparent is 255");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("BCL Extended Drawing Test Suite");
    Console::WriteLine("================================");
    Console::ResetColor();
    Console::WriteLine();

    TestColorFromRgb();
    TestRectangleInflate();
    TestImageCopyFrom();
    TestImageMoveSemantics();
    TestImageGetRegionEdgeCases();
    TestBorderStyleEnum();
    TestBufferModeEnum();
    TestPointArithmetic();
    TestSizeEdgeCases();
    TestRectangleContainsEdgeCases();
    TestColorConstants();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All extended drawing tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
