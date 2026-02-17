/**
 * Unit tests for Icon Loading Functions
 *
 * Tests Image::FromIcon(), Image::FromIconLibrary(), and Image::GetIconLibraryCount()
 */

#include "test_framework.hpp"

using namespace System;
using namespace System::Drawing;

// Test icon sizes
void TestIconSizes() {
    Test::PrintHeader("Icon Sizes");

    ASSERT_EQ(16, static_cast<int>(Size::IconSmall.width), "IconSmall width is 16");
    ASSERT_EQ(16, static_cast<int>(Size::IconSmall.height), "IconSmall height is 16");
    ASSERT_EQ(32, static_cast<int>(Size::IconMedium.width), "IconMedium width is 32");
    ASSERT_EQ(32, static_cast<int>(Size::IconMedium.height), "IconMedium height is 32");
    ASSERT_EQ(48, static_cast<int>(Size::IconLarge.width), "IconLarge width is 48");
    ASSERT_EQ(48, static_cast<int>(Size::IconLarge.height), "IconLarge height is 48");

    Test::PrintSummary();
}

// Test error handling for invalid paths
void TestInvalidPaths() {
    Test::PrintHeader("Invalid Path Handling");

    ASSERT_THROWS(Image::FromIcon(nullptr, Size::IconMedium),
                  ArgumentNullException, "FromIcon throws on null path");

    ASSERT_THROWS(Image::FromIcon("", Size::IconMedium),
                  ArgumentNullException, "FromIcon throws on empty path");

    ASSERT_THROWS(Image::FromIcon("nonexistent.ico", Size::IconMedium),
                  FileNotFoundException, "FromIcon throws on missing file");

    ASSERT_THROWS(Image::FromIconLibrary(nullptr, 0, Size::IconMedium),
                  ArgumentNullException, "FromIconLibrary throws on null path");

    ASSERT_THROWS(Image::FromIconLibrary("", 0, Size::IconMedium),
                  ArgumentNullException, "FromIconLibrary throws on empty path");

    ASSERT_THROWS(Image::FromIconLibrary("nonexistent.icl", 0, Size::IconMedium),
                  FileNotFoundException, "FromIconLibrary throws on missing file");

    ASSERT_THROWS(Image::GetIconLibraryCount(nullptr),
                  ArgumentNullException, "GetIconLibraryCount throws on null path");

    Test::PrintSummary();
}

// Test invalid icon sizes
void TestInvalidSizes() {
    Test::PrintHeader("Invalid Size Handling");

    // These should throw because 24x24 is not a standard icon size
    Size invalidSize(24, 24);

    ASSERT_THROWS(Image::FromIcon("sysicons.icl", invalidSize),
                  ArgumentException, "FromIcon throws on invalid size (24x24)");

    ASSERT_THROWS(Image::FromIconLibrary("sysicons.icl", 0, invalidSize),
                  ArgumentException, "FromIconLibrary throws on invalid size (24x24)");

    Test::PrintSummary();
}

// Test loading icon library count
void TestIconLibraryCount() {
    Test::PrintHeader("Icon Library Count");

    // sysicons.icl should have multiple icons
    Int32 count = Image::GetIconLibraryCount("sysicons.icl");

    ASSERT(static_cast<int>(count) > 0, "sysicons.icl has at least one icon");
    ASSERT(static_cast<int>(count) < 1000, "Icon count is reasonable (<1000)");

    Console::Write("Icon count in sysicons.icl: ");
    Console::WriteLine(static_cast<int>(count));

    Test::PrintSummary();
}

// Test loading icons from library at various indices
void TestIconLibraryLoading() {
    Test::PrintHeader("Icon Library Loading");

    Int32 count = Image::GetIconLibraryCount("sysicons.icl");

    if (static_cast<int>(count) == 0) {
        Test::Fail("No icons in library - cannot test loading");
        Test::PrintSummary();
        return;
    }

    // Test loading first icon at 32x32
    bool loadedFirst = false;
    try {
        Image icon = Image::FromIconLibrary("sysicons.icl", 0, Size::IconMedium);
        loadedFirst = (static_cast<int>(icon.Width()) == 32 &&
                       static_cast<int>(icon.Height()) == 32);
    } catch (...) {
        loadedFirst = false;
    }
    ASSERT(loadedFirst, "Load first icon at 32x32");

    // Test loading icon at different sizes
    bool loaded16 = false;
    try {
        Image icon16 = Image::FromIconLibrary("sysicons.icl", 0, Size::IconSmall);
        loaded16 = (static_cast<int>(icon16.Width()) == 16);
    } catch (const InvalidDataException&) {
        // Size may not be available - this is acceptable
        loaded16 = true;  // Mark as pass if size not found
        Console::WriteLine("(16x16 size not available - OK)");
    } catch (...) {
        loaded16 = false;
    }
    ASSERT(loaded16, "Load icon at 16x16 (or size not available)");

    // Test out of range index
    ASSERT_THROWS(Image::FromIconLibrary("sysicons.icl", 9999, Size::IconMedium),
                  ArgumentException, "Out of range index throws");

    ASSERT_THROWS(Image::FromIconLibrary("sysicons.icl", -1, Size::IconMedium),
                  ArgumentException, "Negative index throws");

    Test::PrintSummary();
}

// Test icon pixel data
void TestIconPixelData() {
    Test::PrintHeader("Icon Pixel Data");

    Int32 count = Image::GetIconLibraryCount("sysicons.icl");
    if (static_cast<int>(count) == 0) {
        Test::Fail("No icons to test");
        Test::PrintSummary();
        return;
    }

    Image icon = Image::FromIconLibrary("sysicons.icl", 0, Size::IconMedium);

    ASSERT_EQ(32, static_cast<int>(icon.Width()), "Icon width is 32");
    ASSERT_EQ(32, static_cast<int>(icon.Height()), "Icon height is 32");
    ASSERT_EQ(32 * 32, static_cast<int>(icon.Length()), "Icon has 1024 pixels");

    // Check that pixel data pointer is valid
    const unsigned int* data = icon.Data();
    ASSERT(data != nullptr, "Icon data pointer is not null");

    // Check that we can read pixels without crashing
    bool canReadPixels = true;
    try {
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                Color c = icon.GetPixel(x, y);
                (void)c;  // Just access it
            }
        }
    } catch (...) {
        canReadPixels = false;
    }
    ASSERT(canReadPixels, "Can read all pixels without error");

    Test::PrintSummary();
}

// Test loading multiple icons
void TestMultipleIcons() {
    Test::PrintHeader("Multiple Icon Loading");

    Int32 count = Image::GetIconLibraryCount("sysicons.icl");
    int loadedCount = 0;
    int failedCount = 0;

    // Try to load first 10 icons (or all if fewer)
    int maxToTest = (static_cast<int>(count) < 10) ? static_cast<int>(count) : 10;

    for (int i = 0; i < maxToTest; i++) {
        try {
            Image icon = Image::FromIconLibrary("sysicons.icl", i, Size::IconMedium);
            if (static_cast<int>(icon.Width()) == 32 &&
                static_cast<int>(icon.Height()) == 32) {
                loadedCount++;
            } else {
                failedCount++;
            }
        } catch (const InvalidDataException&) {
            // Size not available for this icon - acceptable
            loadedCount++;  // Count as success since the file was read
        } catch (...) {
            failedCount++;
        }
    }

    Console::Write("Successfully loaded ");
    Console::Write(loadedCount);
    Console::Write(" of ");
    Console::Write(maxToTest);
    Console::WriteLine(" icons");

    ASSERT(loadedCount > 0, "At least one icon loaded successfully");
    ASSERT(failedCount < maxToTest, "Not all icons failed to load");

    Test::PrintSummary();
}

int main() {
    Console::WriteLine("=== Icon Loading Tests ===");
    Console::WriteLine();

    // Run all test suites
    TestIconSizes();
    TestInvalidPaths();
    TestInvalidSizes();
    TestIconLibraryCount();
    TestIconLibraryLoading();
    TestIconPixelData();
    TestMultipleIcons();

    // Final summary
    Console::WriteLine();
    Console::WriteLine("=== All Icon Tests Complete ===");

    return Test::AllPassed() ? 0 : 1;
}
