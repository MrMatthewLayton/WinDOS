// Extended Drawing tests for rtcorlib
// Tests additional Drawing features not covered in test_graphics.cpp

#include "test_framework.hpp"
#include "../src/System/Drawing/Drawing.hpp"

using namespace System;
using namespace System::Drawing;

void TestColorComponents() {
    Test::PrintHeader("Color Components");

    // Test constructors and component accessors
    Color c1(UInt8(255), UInt8(128), UInt8(64), UInt8(32));  // A=255, R=128, G=64, B=32
    ASSERT_EQ(255, static_cast<int>(c1.A()), "Alpha from ARGB constructor");
    ASSERT_EQ(128, static_cast<int>(c1.R()), "Red from ARGB constructor");
    ASSERT_EQ(64, static_cast<int>(c1.G()), "Green from ARGB constructor");
    ASSERT_EQ(32, static_cast<int>(c1.B()), "Blue from ARGB constructor");

    // RGB constructor (opaque)
    Color c2(UInt8(100), UInt8(150), UInt8(200));  // R=100, G=150, B=200
    ASSERT_EQ(255, static_cast<int>(c2.A()), "Alpha defaults to 255");
    ASSERT_EQ(100, static_cast<int>(c2.R()), "Red from RGB constructor");
    ASSERT_EQ(150, static_cast<int>(c2.G()), "Green from RGB constructor");
    ASSERT_EQ(200, static_cast<int>(c2.B()), "Blue from RGB constructor");

    // ARGB value constructor
    Color c3(0x80FF00FF);  // A=128, R=255, G=0, B=255 (semi-transparent magenta)
    ASSERT_EQ(128, static_cast<int>(c3.A()), "Alpha from ARGB value");
    ASSERT_EQ(255, static_cast<int>(c3.R()), "Red from ARGB value");
    ASSERT_EQ(0, static_cast<int>(c3.G()), "Green from ARGB value");
    ASSERT_EQ(255, static_cast<int>(c3.B()), "Blue from ARGB value");

    Test::PrintSummary();
}

void TestColorToVga() {
    Test::PrintHeader("Color::ToVgaIndex");

    // ToVgaIndex finds closest VGA palette color
    Color black(UInt8(0), UInt8(0), UInt8(0));
    ASSERT_EQ(0, static_cast<int>(black.ToVgaIndex()), "Black maps to VGA 0");

    Color white(UInt8(255), UInt8(255), UInt8(255));
    ASSERT_EQ(15, static_cast<int>(white.ToVgaIndex()), "White maps to VGA 15");

    // Standard VGA colors
    ASSERT_EQ(0, static_cast<int>(Color::Black.ToVgaIndex()), "Color::Black VGA index");
    ASSERT_EQ(15, static_cast<int>(Color::White.ToVgaIndex()), "Color::White VGA index");

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
    ASSERT(Color::Green == dst.GetPixel(5, 5), "CopyFrom (0,0) goes to (5,5)");
    ASSERT(Color::Blue == dst.GetPixel(10, 10), "CopyFrom center pixel (5+5)");

    // Original destination pixels outside copy area unchanged
    ASSERT(Color::Black == dst.GetPixel(0, 0), "CopyFrom doesn't affect (0,0)");
    ASSERT(Color::Black == dst.GetPixel(19, 19), "CopyFrom doesn't affect far corner");

    // Copy with Point overload
    Image dst2(20, 20, Color::White);
    dst2.CopyFrom(src, Point(3, 3));
    ASSERT(Color::Green == dst2.GetPixel(3, 3), "CopyFrom with Point");

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
    ASSERT(Color::Magenta == moved.GetPixel(25, 25), "Move constructor preserves pixels");

    // Source should be empty after move
    ASSERT_EQ(0, src.Width(), "Moved-from source width is 0");
    ASSERT_EQ(0, src.Height(), "Moved-from source height is 0");

    // Move assign
    Image another(30, 30, Color::Yellow);
    another = static_cast<Image&&>(moved);
    ASSERT_EQ(50, another.Width(), "Move assignment width");
    ASSERT(Color::Magenta == another.GetPixel(25, 25), "Move assignment preserves pixels");
    ASSERT_EQ(0, moved.Width(), "Moved-from width is 0 after assignment");

    Test::PrintSummary();
}

void TestImageGetRegionEdgeCases() {
    Test::PrintHeader("Image GetRegion Edge Cases");

    Image img(20, 20, Color::White);

    // Get region from corner
    Image corner = img.GetRegion(0, 0, 5, 5);
    ASSERT_EQ(5, corner.Width(), "Corner region width");
    ASSERT_EQ(5, corner.Height(), "Corner region height");

    // Get region from opposite corner
    Image farCorner = img.GetRegion(15, 15, 5, 5);
    ASSERT_EQ(5, farCorner.Width(), "Far corner region width");

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
    Test::PrintHeader("Color Constants (32-bit ARGB)");

    // Verify standard colors have correct ARGB values
    ASSERT_EQ(0xFF000000u, static_cast<unsigned int>(Color::Black.ToArgb()), "Black ARGB");
    ASSERT_EQ(0xFFFFFFFFu, static_cast<unsigned int>(Color::White.ToArgb()), "White ARGB");
    ASSERT_EQ(0x00000000u, static_cast<unsigned int>(Color::Transparent.ToArgb()), "Transparent ARGB");

    // Verify alpha channel
    ASSERT_EQ(255, static_cast<int>(Color::Black.A()), "Black is opaque");
    ASSERT_EQ(255, static_cast<int>(Color::White.A()), "White is opaque");
    ASSERT_EQ(0, static_cast<int>(Color::Transparent.A()), "Transparent has alpha 0");

    // Verify some color components
    ASSERT_EQ(0, static_cast<int>(Color::Black.R()), "Black R=0");
    ASSERT_EQ(0, static_cast<int>(Color::Black.G()), "Black G=0");
    ASSERT_EQ(0, static_cast<int>(Color::Black.B()), "Black B=0");

    ASSERT_EQ(255, static_cast<int>(Color::White.R()), "White R=255");
    ASSERT_EQ(255, static_cast<int>(Color::White.G()), "White G=255");
    ASSERT_EQ(255, static_cast<int>(Color::White.B()), "White B=255");

    Test::PrintSummary();
}

void TestColorLerp() {
    Test::PrintHeader("Color::Lerp");

    Color black = Color::Black;
    Color white = Color::White;

    // t=0 returns first color
    Color c0 = Color::Lerp(black, white, 0.0f);
    ASSERT(c0 == black, "Lerp t=0 returns first color");

    // t=1 returns second color
    Color c1 = Color::Lerp(black, white, 1.0f);
    ASSERT(c1 == white, "Lerp t=1 returns second color");

    // t=0.5 returns midpoint
    Color mid = Color::Lerp(black, white, 0.5f);
    // Should be approximately gray (127-128 for each channel)
    int r = static_cast<int>(mid.R());
    int g = static_cast<int>(mid.G());
    int b = static_cast<int>(mid.B());
    ASSERT(r >= 126 && r <= 129, "Lerp midpoint R is ~127");
    ASSERT(g >= 126 && g <= 129, "Lerp midpoint G is ~127");
    ASSERT(b >= 126 && b <= 129, "Lerp midpoint B is ~127");

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("rtcorlib Extended Drawing Test Suite");
    Console::WriteLine("================================");
    Console::ResetColor();
    Console::WriteLine();

    TestColorComponents();
    TestColorToVga();
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
    TestColorLerp();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All extended drawing tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
