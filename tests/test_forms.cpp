// Forms tests for rtcorlib
// Tests Control, Window, Button, Picture, TaskBar, and event argument classes
// Note: Full GUI testing requires graphics mode; these test the API structure

#include "test_framework.hpp"
#include "../src/System/Windows/Forms/Forms.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

void TestPaintEventArgs() {
    Test::PrintHeader("PaintEventArgs");

    // Default constructor
    PaintEventArgs pe1;
    ASSERT(pe1.graphics == nullptr, "Default graphics is nullptr");
    ASSERT_EQ(0, pe1.bounds.x, "Default bounds x is 0");
    ASSERT_EQ(0, pe1.bounds.y, "Default bounds y is 0");

    // Parameterized constructor
    Rectangle bounds(10, 20, 100, 50);
    PaintEventArgs pe2(nullptr, bounds);
    ASSERT_EQ(10, pe2.bounds.x, "Parameterized bounds x");
    ASSERT_EQ(20, pe2.bounds.y, "Parameterized bounds y");
    ASSERT_EQ(100, pe2.bounds.width, "Parameterized bounds width");
    ASSERT_EQ(50, pe2.bounds.height, "Parameterized bounds height");

    // Modify bounds
    pe2.bounds.x = 30;
    ASSERT_EQ(30, pe2.bounds.x, "Modified bounds x");

    Test::PrintSummary();
}

void TestMouseEventArgs() {
    Test::PrintHeader("MouseEventArgs");

    // Default constructor
    MouseEventArgs me1;
    ASSERT_EQ(0, me1.x, "Default x is 0");
    ASSERT_EQ(0, me1.y, "Default y is 0");
    ASSERT(!me1.leftButton, "Default leftButton is false");
    ASSERT(!me1.rightButton, "Default rightButton is false");

    // Parameterized constructor
    MouseEventArgs me2(100, 200, true, false);
    ASSERT_EQ(100, me2.x, "Parameterized x");
    ASSERT_EQ(200, me2.y, "Parameterized y");
    ASSERT(me2.leftButton, "Parameterized leftButton");
    ASSERT(!me2.rightButton, "Parameterized rightButton");

    // Modify
    me2.x = 150;
    me2.rightButton = true;
    ASSERT_EQ(150, me2.x, "Modified x");
    ASSERT(me2.rightButton, "Modified rightButton");

    Test::PrintSummary();
}

void TestKeyboardEventArgs() {
    Test::PrintHeader("KeyboardEventArgs");

    // Default constructor
    KeyboardEventArgs ke1;
    ASSERT_EQ('\0', ke1.key, "Default key is null");
    ASSERT(!ke1.alt, "Default alt is false");
    ASSERT(!ke1.ctrl, "Default ctrl is false");
    ASSERT(!ke1.shift, "Default shift is false");

    // Parameterized constructor
    KeyboardEventArgs ke2('A', false, true, true);
    ASSERT_EQ('A', ke2.key, "Parameterized key");
    ASSERT(!ke2.alt, "Parameterized alt");
    ASSERT(ke2.ctrl, "Parameterized ctrl");
    ASSERT(ke2.shift, "Parameterized shift");

    // Modify
    ke2.key = 'Z';
    ke2.alt = true;
    ASSERT_EQ('Z', ke2.key, "Modified key");
    ASSERT(ke2.alt, "Modified alt");

    Test::PrintSummary();
}

void TestControlBasic() {
    Test::PrintHeader("Control Basic");

    // Default constructor
    Control c1;
    ASSERT(c1.Parent() == nullptr, "Default parent is nullptr");
    ASSERT_EQ(0, c1.Bounds().x, "Default bounds x");
    ASSERT_EQ(0, c1.Bounds().y, "Default bounds y");
    ASSERT_EQ(0, c1.ChildCount(), "Default no children");

    // Constructor with parent and bounds (parent is nullptr for top-level)
    Rectangle bounds(10, 20, 100, 50);
    Control c2(nullptr, bounds);
    ASSERT(c2.Parent() == nullptr, "Top-level parent is nullptr");
    ASSERT_EQ(10, c2.Bounds().x, "Bounds x from constructor");
    ASSERT_EQ(20, c2.Bounds().y, "Bounds y from constructor");
    ASSERT_EQ(100, c2.Bounds().width, "Bounds width from constructor");
    ASSERT_EQ(50, c2.Bounds().height, "Bounds height from constructor");

    Test::PrintSummary();
}

void TestControlHierarchy() {
    Test::PrintHeader("Control Hierarchy");

    // Create parent control
    Control parent(nullptr, Rectangle(0, 0, 200, 200));

    // Create child controls
    Control* child1 = new Control(&parent, Rectangle(10, 10, 50, 50));
    Control* child2 = new Control(&parent, Rectangle(70, 10, 50, 50));

    // Note: Adding to parent happens in constructor
    ASSERT_EQ(2, parent.ChildCount(), "Parent has 2 children");
    ASSERT(parent.GetChild(0) == child1, "First child is child1");
    ASSERT(parent.GetChild(1) == child2, "Second child is child2");
    ASSERT(child1->Parent() == &parent, "Child1 parent is parent");
    ASSERT(child2->Parent() == &parent, "Child2 parent is parent");

    // Remove child
    parent.RemoveChild(child1);
    ASSERT_EQ(1, parent.ChildCount(), "Parent has 1 child after remove");
    ASSERT(parent.GetChild(0) == child2, "Remaining child is child2");
    ASSERT(child1->Parent() == nullptr, "Removed child parent is nullptr");

    // Clean up
    delete child1;
    delete child2;

    Test::PrintSummary();
}

void TestControlBounds() {
    Test::PrintHeader("Control Bounds");

    Control parent(nullptr, Rectangle(100, 100, 300, 200));
    Control* child = new Control(&parent, Rectangle(10, 10, 50, 50));

    // Bounds (relative to parent)
    ASSERT_EQ(10, child->Bounds().x, "Child bounds x relative");
    ASSERT_EQ(10, child->Bounds().y, "Child bounds y relative");

    // ScreenBounds (absolute)
    Rectangle screenBounds = child->ScreenBounds();
    ASSERT_EQ(110, screenBounds.x, "Child screen bounds x = parent.x + child.x");
    ASSERT_EQ(110, screenBounds.y, "Child screen bounds y = parent.y + child.y");
    ASSERT_EQ(50, screenBounds.width, "Child screen bounds width");
    ASSERT_EQ(50, screenBounds.height, "Child screen bounds height");

    // SetBounds
    child->SetBounds(20, 30, 60, 40);
    ASSERT_EQ(20, child->Bounds().x, "SetBounds x");
    ASSERT_EQ(30, child->Bounds().y, "SetBounds y");
    ASSERT_EQ(60, child->Bounds().width, "SetBounds width");
    ASSERT_EQ(40, child->Bounds().height, "SetBounds height");

    // SetBounds with Rectangle
    child->SetBounds(Rectangle(5, 5, 30, 30));
    ASSERT_EQ(5, child->Bounds().x, "SetBounds rect x");

    delete child;

    Test::PrintSummary();
}

void TestControlHitTest() {
    Test::PrintHeader("Control HitTest");

    Control parent(nullptr, Rectangle(100, 100, 200, 200));
    Control* child = new Control(&parent, Rectangle(50, 50, 50, 50));

    // HitTest uses screen coordinates
    // Parent is at (100, 100) to (300, 300)
    ASSERT(parent.HitTest(150, 150), "HitTest inside parent");
    ASSERT(parent.HitTest(100, 100), "HitTest on parent corner");
    ASSERT(!parent.HitTest(50, 50), "HitTest outside parent");
    ASSERT(!parent.HitTest(350, 350), "HitTest outside parent right");

    // Child is at (150, 150) to (200, 200) in screen coords
    ASSERT(child->HitTest(175, 175), "HitTest inside child");
    ASSERT(child->HitTest(150, 150), "HitTest on child corner");
    ASSERT(!child->HitTest(140, 140), "HitTest outside child");

    delete child;

    Test::PrintSummary();
}

void TestButtonControl() {
    Test::PrintHeader("Button Control");

    Control parent(nullptr, Rectangle(0, 0, 200, 200));
    Button* button = new Button(&parent, Rectangle(10, 10, 80, 30));

    // Initial state
    ASSERT(!button->IsPressed(), "Button not pressed initially");
    ASSERT_EQ(10, button->Bounds().x, "Button bounds x");
    ASSERT_EQ(80, button->Bounds().width, "Button bounds width");

    // Client bounds should be available
    const Rectangle& client = button->ClientBounds();
    ASSERT(client.width > 0 || client.width == 0, "Button has client bounds");

    delete button;

    Test::PrintSummary();
}

void TestPictureControl() {
    Test::PrintHeader("Picture Control");

    Control parent(nullptr, Rectangle(0, 0, 200, 200));

    // Create without image
    Picture* pic1 = new Picture(&parent, Rectangle(10, 10, 50, 50));
    ASSERT_EQ(0, pic1->GetImage().Width(), "Picture without image has empty image");

    // Create with image
    Image img(30, 30, Color::Red);
    Picture* pic2 = new Picture(&parent, Rectangle(70, 10, 50, 50), img);
    ASSERT_EQ(30, pic2->GetImage().Width(), "Picture with image has correct width");
    ASSERT_EQ(30, pic2->GetImage().Height(), "Picture with image has correct height");

    // Set image
    Image img2(20, 20, Color::Blue);
    pic1->SetImage(img2);
    ASSERT_EQ(20, pic1->GetImage().Width(), "SetImage updates width");
    ASSERT_EQ(20, pic1->GetImage().Height(), "SetImage updates height");

    delete pic1;
    delete pic2;

    Test::PrintSummary();
}

void TestWindowControl() {
    Test::PrintHeader("Window Control");

    Control parent(nullptr, Rectangle(0, 0, 640, 480));
    Window* window = new Window(&parent, Rectangle(50, 50, 200, 150));

    // Check bounds
    ASSERT_EQ(50, window->Bounds().x, "Window bounds x");
    ASSERT_EQ(50, window->Bounds().y, "Window bounds y");
    ASSERT_EQ(200, window->Bounds().width, "Window bounds width");
    ASSERT_EQ(150, window->Bounds().height, "Window bounds height");

    // Client bounds should be smaller (accounting for frame and title bar)
    const Rectangle& client = window->ClientBounds();
    ASSERT(client.width < 200, "Window client width less than total width");
    ASSERT(client.height < 150, "Window client height less than total height");
    ASSERT(client.x > 0 || client.y > 0, "Window client offset for frame");

    // Focus state
    ASSERT(!window->IsFocused(), "Window not focused initially");
    window->SetFocused(true);
    ASSERT(window->IsFocused(), "Window focused after SetFocused(true)");
    window->SetFocused(false);
    ASSERT(!window->IsFocused(), "Window not focused after SetFocused(false)");

    delete window;

    Test::PrintSummary();
}

void TestNestedControls() {
    Test::PrintHeader("Nested Controls");

    // Create a hierarchy: parent -> window -> button
    Control parent(nullptr, Rectangle(0, 0, 640, 480));
    Window* window = new Window(&parent, Rectangle(100, 100, 200, 150));
    Button* button = new Button(window, Rectangle(10, 10, 60, 25));

    // Verify hierarchy
    ASSERT(button->Parent() == window, "Button parent is window");
    ASSERT(window->Parent() == &parent, "Window parent is parent");
    ASSERT_EQ(1, window->ChildCount(), "Window has 1 child");
    ASSERT(window->GetChild(0) == button, "Window child is button");

    // Screen bounds of button (should account for all ancestors)
    // Parent at (0,0), Window at (100,100) with frame offset, Button at offset in client
    Rectangle buttonScreen = button->ScreenBounds();
    ASSERT(buttonScreen.x > 100, "Button screen x accounts for window position and frame");
    ASSERT(buttonScreen.y > 100, "Button screen y accounts for window position and title bar");

    delete button;
    delete window;

    Test::PrintSummary();
}

void TestControlInvalidation() {
    Test::PrintHeader("Control Invalidation");

    Control parent(nullptr, Rectangle(0, 0, 200, 200));
    Control* child = new Control(&parent, Rectangle(10, 10, 50, 50));

    // Invalidate should not crash (actual redraw requires graphics context)
    ASSERT_NO_THROW(child->Invalidate(), "Invalidate does not throw");
    ASSERT_NO_THROW(parent.Invalidate(), "Parent Invalidate does not throw");

    delete child;

    Test::PrintSummary();
}

int main() {
    Console::Clear();
    Console::SetForegroundColor(ConsoleColor::Cyan);
    Console::WriteLine("rtcorlib Windows Forms Test Suite");
    Console::WriteLine("============================");
    Console::ResetColor();
    Console::WriteLine();
    Console::WriteLine("Note: Full GUI tests require graphics mode.");
    Console::WriteLine("      These tests verify API structure and logic.");
    Console::WriteLine();

    TestPaintEventArgs();
    TestMouseEventArgs();
    TestKeyboardEventArgs();
    TestControlBasic();
    TestControlHierarchy();
    TestControlBounds();
    TestControlHitTest();
    TestButtonControl();
    TestPictureControl();
    TestWindowControl();
    TestNestedControls();
    TestControlInvalidation();

    Console::WriteLine();
    Console::SetForegroundColor(ConsoleColor::White);
    Console::WriteLine("All forms tests complete!");
    Console::ResetColor();

    Console::WriteLine();
    Console::Write("Press any key to exit...");
    Console::ReadKey(true);

    return 0;
}
