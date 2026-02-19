/**
 * test_layout.cpp - Layout System Tests for WinDOS Forms
 *
 * Tests the flexbox-like layout system including:
 * - Measure pass (bottom-up size calculation)
 * - Arrange pass (top-down positioning)
 * - FlexGrow distribution
 * - JustifyContent positioning
 * - AlignItems alignment
 * - Floating controls (Windows)
 * - Nested layout
 * - Dirty flag optimization
 */

#include "test_framework.hpp"
#include "../src/System/Windows/Forms/Forms.hpp"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

/******************************************************************************/
/*    Test Helper: MockControl                                                 */
/******************************************************************************/

/**
 * A simple control for testing layout without graphics dependencies.
 * Reports a configurable preferred size.
 */
class MockControl : public Control {
private:
    Int32 _preferredWidth;
    Int32 _preferredHeight;

public:
    MockControl(Control* parent, const Rectangle& bounds,
                Int32 prefW = 0, Int32 prefH = 0)
        : Control(parent, bounds)
        , _preferredWidth(prefW)
        , _preferredHeight(prefH) {
        // If no preferred size specified, use bounds
        if (prefW == 0) _preferredWidth = bounds.width;
        if (prefH == 0) _preferredHeight = bounds.height;
    }

    void SetPreferredSize(Int32 w, Int32 h) {
        _preferredWidth = w;
        _preferredHeight = h;
    }

    virtual MeasureResult GetPreferredSize() const override {
        return MeasureResult(_preferredWidth, _preferredHeight);
    }

    // Suppress painting for tests
    virtual void OnPaint(PaintEventArgs& e) override { (void)e; }
};

/******************************************************************************/
/*    Test: Layout Properties Defaults                                         */
/******************************************************************************/

void TestLayoutPropertiesDefaults() {
    Test::PrintHeader("Layout Properties Defaults");

    LayoutProperties props;

    ASSERT(props.direction == FlexDirection::Column,
           "Default direction should be Column");
    ASSERT(props.justifyContent == JustifyContent::Start,
           "Default justifyContent should be Start");
    ASSERT(props.alignItems == AlignItems::Stretch,
           "Default alignItems should be Stretch");
    ASSERT_EQ(Int32(0), props.gap, "Default gap should be 0");
    ASSERT_EQ(Int32(0), props.flexGrow, "Default flexGrow should be 0");
    ASSERT_EQ(Int32(1), props.flexShrink, "Default flexShrink should be 1");
    ASSERT(props.widthMode == SizeMode::Auto,
           "Default widthMode should be Auto");
    ASSERT(props.heightMode == SizeMode::Auto,
           "Default heightMode should be Auto");
    ASSERT(props.participatesInLayout == true,
           "Default participatesInLayout should be true");
    ASSERT(props.needsLayout == true,
           "Default needsLayout should be true");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Layout Properties Fluent API                                       */
/******************************************************************************/

void TestLayoutPropertiesFluentAPI() {
    Test::PrintHeader("Layout Properties Fluent API");

    LayoutProperties props;
    props.SetDirection(FlexDirection::Row)
         .SetJustifyContent(JustifyContent::Center)
         .SetAlignItems(AlignItems::End)
         .SetGap(Int32(10))
         .SetFlexGrow(Int32(1))
         .SetMargin(Int32(5))
         .SetPadding(Int32(2), Int32(4));

    ASSERT(props.direction == FlexDirection::Row,
           "Fluent SetDirection works");
    ASSERT(props.justifyContent == JustifyContent::Center,
           "Fluent SetJustifyContent works");
    ASSERT(props.alignItems == AlignItems::End,
           "Fluent SetAlignItems works");
    ASSERT_EQ(Int32(10), props.gap, "Fluent SetGap works");
    ASSERT_EQ(Int32(1), props.flexGrow, "Fluent SetFlexGrow works");
    ASSERT_EQ(Int32(5), props.marginTop, "SetMargin(all) sets marginTop");
    ASSERT_EQ(Int32(5), props.marginLeft, "SetMargin(all) sets marginLeft");
    ASSERT_EQ(Int32(2), props.paddingTop, "SetPadding(v,h) sets paddingTop");
    ASSERT_EQ(Int32(4), props.paddingLeft, "SetPadding(v,h) sets paddingLeft");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Measure Pass - Single Control                                      */
/******************************************************************************/

void TestMeasureSingleControl() {
    Test::PrintHeader("Measure Pass - Single Control");

    MockControl root(nullptr, Rectangle(0, 0, 200, 100), Int32(150), Int32(80));

    // With Auto sizing, should return preferred size
    MeasureResult result = root.Measure(Int32(300), Int32(200));

    ASSERT_EQ(Int32(150), result.preferredWidth,
              "Single control measures to preferred width");
    ASSERT_EQ(Int32(80), result.preferredHeight,
              "Single control measures to preferred height");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Measure Pass - Fixed Size Mode                                     */
/******************************************************************************/

void TestMeasureFixedSize() {
    Test::PrintHeader("Measure Pass - Fixed Size Mode");

    MockControl root(nullptr, Rectangle(0, 0, 200, 100));
    root.Layout().widthMode = SizeMode::Fixed;
    root.Layout().heightMode = SizeMode::Fixed;

    MeasureResult result = root.Measure(Int32(300), Int32(200));

    ASSERT_EQ(Int32(200), result.preferredWidth,
              "Fixed width uses bounds width");
    ASSERT_EQ(Int32(100), result.preferredHeight,
              "Fixed height uses bounds height");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Measure Pass - Min/Max Constraints                                 */
/******************************************************************************/

void TestMeasureMinMaxConstraints() {
    Test::PrintHeader("Measure Pass - Min/Max Constraints");

    MockControl root(nullptr, Rectangle(0, 0, 100, 100), Int32(50), Int32(200));
    root.Layout().minWidth = 80;
    root.Layout().maxHeight = 150;

    MeasureResult result = root.Measure(Int32(300), Int32(300));

    ASSERT_EQ(Int32(80), result.preferredWidth,
              "Width clamped to minWidth");
    ASSERT_EQ(Int32(150), result.preferredHeight,
              "Height clamped to maxHeight");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Measure Pass - Column with Children                                */
/******************************************************************************/

void TestMeasureColumnWithChildren() {
    Test::PrintHeader("Measure Pass - Column with Children");

    MockControl root(nullptr, Rectangle(0, 0, 200, 300));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().gap = 5;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 40));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 80, 60));
    (void)child1;
    (void)child2;

    MeasureResult result = root.Measure(Int32(300), Int32(300));

    // Column: height = 40 + 5 (gap) + 60 = 105, width = max(100, 80) = 100
    ASSERT_EQ(Int32(100), result.preferredWidth,
              "Column width is max of children");
    ASSERT_EQ(Int32(105), result.preferredHeight,
              "Column height is sum of children + gaps");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Measure Pass - Row with Children                                   */
/******************************************************************************/

void TestMeasureRowWithChildren() {
    Test::PrintHeader("Measure Pass - Row with Children");

    MockControl root(nullptr, Rectangle(0, 0, 300, 100));
    root.Layout().direction = FlexDirection::Row;
    root.Layout().gap = 10;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 40));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 80, 60));
    (void)child1;
    (void)child2;

    MeasureResult result = root.Measure(Int32(400), Int32(200));

    // Row: width = 100 + 10 (gap) + 80 = 190, height = max(40, 60) = 60
    ASSERT_EQ(Int32(190), result.preferredWidth,
              "Row width is sum of children + gaps");
    ASSERT_EQ(Int32(60), result.preferredHeight,
              "Row height is max of children");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - Basic Positioning                                   */
/******************************************************************************/

void TestArrangeBasicPositioning() {
    Test::PrintHeader("Arrange Pass - Basic Positioning");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 80, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    ASSERT_EQ(Int32(0), child1->Bounds().x, "First child X is 0");
    ASSERT_EQ(Int32(0), child1->Bounds().y, "First child Y is 0");
    ASSERT_EQ(Int32(0), child2->Bounds().x, "Second child X is 0");
    ASSERT_EQ(Int32(50), child2->Bounds().y, "Second child Y follows first");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - FlexGrow Distribution                               */
/******************************************************************************/

void TestArrangeFlexGrow() {
    Test::PrintHeader("Arrange Pass - FlexGrow Distribution");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    child1->Layout().flexGrow = 1;

    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    child2->Layout().flexGrow = 1;

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Extra space = 200 - 100 = 100, split evenly
    // Each child gets 50 + 50 = 100 height
    ASSERT_EQ(Int32(100), child1->Bounds().height,
              "First child grows with flexGrow");
    ASSERT_EQ(Int32(100), child2->Bounds().height,
              "Second child grows with flexGrow");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - JustifyContent Center                               */
/******************************************************************************/

void TestArrangeJustifyContentCenter() {
    Test::PrintHeader("Arrange Pass - JustifyContent Center");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().justifyContent = JustifyContent::Center;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Extra space = 200 - 50 = 150, centered = 75
    ASSERT_EQ(Int32(75), child->Bounds().y,
              "Child centered with JustifyContent::Center");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - JustifyContent End                                  */
/******************************************************************************/

void TestArrangeJustifyContentEnd() {
    Test::PrintHeader("Arrange Pass - JustifyContent End");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().justifyContent = JustifyContent::End;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Extra space = 200 - 50 = 150
    ASSERT_EQ(Int32(150), child->Bounds().y,
              "Child at end with JustifyContent::End");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - JustifyContent SpaceBetween                         */
/******************************************************************************/

void TestArrangeJustifyContentSpaceBetween() {
    Test::PrintHeader("Arrange Pass - JustifyContent SpaceBetween");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().justifyContent = JustifyContent::SpaceBetween;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // First at start, last at end
    ASSERT_EQ(Int32(0), child1->Bounds().y,
              "First child at start with SpaceBetween");
    // Second at 200 - 50 = 150
    ASSERT_EQ(Int32(150), child2->Bounds().y,
              "Last child at end with SpaceBetween");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - AlignItems Center                                   */
/******************************************************************************/

void TestArrangeAlignItemsCenter() {
    Test::PrintHeader("Arrange Pass - AlignItems Center");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().alignItems = AlignItems::Center;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Cross axis center: (200 - 100) / 2 = 50
    ASSERT_EQ(Int32(50), child->Bounds().x,
              "Child centered on cross axis");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - AlignItems End                                      */
/******************************************************************************/

void TestArrangeAlignItemsEnd() {
    Test::PrintHeader("Arrange Pass - AlignItems End");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().alignItems = AlignItems::End;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Cross axis end: 200 - 100 = 100
    ASSERT_EQ(Int32(100), child->Bounds().x,
              "Child at end of cross axis");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Arrange Pass - AlignItems Stretch                                  */
/******************************************************************************/

void TestArrangeAlignItemsStretch() {
    Test::PrintHeader("Arrange Pass - AlignItems Stretch");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().alignItems = AlignItems::Stretch;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Cross axis stretched to parent width
    ASSERT_EQ(Int32(200), child->Bounds().width,
              "Child stretched to parent width");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Floating Controls (participatesInLayout = false)                   */
/******************************************************************************/

void TestFloatingControls() {
    Test::PrintHeader("Floating Controls");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 50));

    // Floating control - should not affect layout
    MockControl* floating = new MockControl(&root, Rectangle(50, 50, 80, 80));
    floating->Layout().participatesInLayout = false;

    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Floating control keeps its original position
    ASSERT_EQ(Int32(50), floating->Bounds().x,
              "Floating control keeps original X");
    ASSERT_EQ(Int32(50), floating->Bounds().y,
              "Floating control keeps original Y");

    // Regular children laid out without considering floating
    ASSERT_EQ(Int32(0), child1->Bounds().y, "First child at Y=0");
    ASSERT_EQ(Int32(50), child2->Bounds().y, "Second child follows first");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Nested Layout                                                      */
/******************************************************************************/

void TestNestedLayout() {
    Test::PrintHeader("Nested Layout");

    // Root: column layout
    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    // Nested container: row layout
    MockControl* nested = new MockControl(&root, Rectangle(0, 0, 200, 100));
    nested->Layout().direction = FlexDirection::Row;
    nested->Layout().heightMode = SizeMode::Fixed;

    MockControl* inner1 = new MockControl(nested, Rectangle(0, 0, 60, 50));
    MockControl* inner2 = new MockControl(nested, Rectangle(0, 0, 80, 50));
    (void)inner1;
    (void)inner2;

    MockControl* sibling = new MockControl(&root, Rectangle(0, 0, 200, 50));
    sibling->Layout().heightMode = SizeMode::Fixed;

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Nested container at Y=0, sibling at Y=100
    ASSERT_EQ(Int32(0), nested->Bounds().y,
              "Nested container at Y=0");
    ASSERT_EQ(Int32(100), sibling->Bounds().y,
              "Sibling follows nested container");

    // Inner children arranged horizontally
    ASSERT_EQ(Int32(0), inner1->Bounds().x, "Inner1 at X=0");
    ASSERT_EQ(Int32(60), inner2->Bounds().x, "Inner2 follows inner1");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Padding Affects Content Area                                       */
/******************************************************************************/

void TestPaddingAffectsContentArea() {
    Test::PrintHeader("Padding Affects Content Area");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().paddingTop = 20;
    root.Layout().paddingLeft = 10;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Child positioned after padding
    ASSERT_EQ(Int32(10), child->Bounds().x,
              "Child offset by paddingLeft");
    ASSERT_EQ(Int32(20), child->Bounds().y,
              "Child offset by paddingTop");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Margin on Children                                                 */
/******************************************************************************/

void TestMarginOnChildren() {
    Test::PrintHeader("Margin on Children");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    child1->Layout().marginBottom = 10;

    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 100, 50));
    child2->Layout().marginTop = 5;

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Child2 Y = child1.height(50) + child1.marginBottom(10) + child2.marginTop(5) = 65
    ASSERT_EQ(Int32(65), child2->Bounds().y,
              "Margins create space between children");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: PerformLayout and InvalidateLayout                                 */
/******************************************************************************/

void TestPerformLayoutAndInvalidate() {
    Test::PrintHeader("PerformLayout and InvalidateLayout");

    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;

    MockControl* child = new MockControl(&root, Rectangle(0, 0, 100, 50));
    (void)child;

    // Initially dirty
    ASSERT(root.Layout().needsLayout == true,
           "Initially needsLayout is true");

    root.PerformLayout();

    ASSERT(root.Layout().needsLayout == false,
           "After PerformLayout, needsLayout is false");

    root.InvalidateLayout();

    ASSERT(root.Layout().needsLayout == true,
           "After InvalidateLayout, needsLayout is true");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Row Layout with Gap                                                */
/******************************************************************************/

void TestRowLayoutWithGap() {
    Test::PrintHeader("Row Layout with Gap");

    MockControl root(nullptr, Rectangle(0, 0, 300, 100));
    root.Layout().direction = FlexDirection::Row;
    root.Layout().gap = 15;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 80, 50));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 80, 50));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 80, 50));

    root.Measure(Int32(300), Int32(100));
    root.Arrange(Rectangle(0, 0, 300, 100));

    ASSERT_EQ(Int32(0), child1->Bounds().x, "First child at X=0");
    ASSERT_EQ(Int32(95), child2->Bounds().x, "Second child at X=80+15");
    ASSERT_EQ(Int32(190), child3->Bounds().x, "Third child at X=80+15+80+15");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Column Layout with Wrap                                            */
/******************************************************************************/

void TestColumnLayoutWithWrap() {
    Test::PrintHeader("Column Layout with Wrap");

    // Container: 200x200, children: 50x70 each
    // 200 / 70 = 2 children per column before wrap
    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().wrap = FlexWrap::Wrap;
    root.Layout().gap = 0;

    // Create 5 children, each 50x70
    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child4 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child5 = new MockControl(&root, Rectangle(0, 0, 50, 70));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Column layout wraps: first column (child1, child2), second (child3, child4), third (child5)
    // child1: Y=0, X=0
    // child2: Y=70, X=0
    // child3: Y=0, X=50 (wrapped to next column)
    // child4: Y=70, X=50
    // child5: Y=0, X=100

    ASSERT_EQ(Int32(0), child1->Bounds().y, "Child1 Y=0 (first column)");
    ASSERT_EQ(Int32(0), child1->Bounds().x, "Child1 X=0");

    ASSERT_EQ(Int32(70), child2->Bounds().y, "Child2 Y=70 (first column)");
    ASSERT_EQ(Int32(0), child2->Bounds().x, "Child2 X=0");

    ASSERT_EQ(Int32(0), child3->Bounds().y, "Child3 Y=0 (wrapped to second column)");
    ASSERT_EQ(Int32(50), child3->Bounds().x, "Child3 X=50");

    ASSERT_EQ(Int32(70), child4->Bounds().y, "Child4 Y=70 (second column)");
    ASSERT_EQ(Int32(50), child4->Bounds().x, "Child4 X=50");

    ASSERT_EQ(Int32(0), child5->Bounds().y, "Child5 Y=0 (wrapped to third column)");
    ASSERT_EQ(Int32(100), child5->Bounds().x, "Child5 X=100");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Row Layout with Wrap                                               */
/******************************************************************************/

void TestRowLayoutWithWrap() {
    Test::PrintHeader("Row Layout with Wrap");

    // Container: 200x200, children: 70x50 each
    // 200 / 70 = 2 children per row before wrap
    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Row;
    root.Layout().wrap = FlexWrap::Wrap;
    root.Layout().gap = 0;

    // Create 5 children, each 70x50
    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 70, 50));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 70, 50));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 70, 50));
    MockControl* child4 = new MockControl(&root, Rectangle(0, 0, 70, 50));
    MockControl* child5 = new MockControl(&root, Rectangle(0, 0, 70, 50));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // Row layout wraps: first row (child1, child2), second (child3, child4), third (child5)
    // child1: X=0, Y=0
    // child2: X=70, Y=0
    // child3: X=0, Y=50 (wrapped to next row)
    // child4: X=70, Y=50
    // child5: X=0, Y=100

    ASSERT_EQ(Int32(0), child1->Bounds().x, "Child1 X=0 (first row)");
    ASSERT_EQ(Int32(0), child1->Bounds().y, "Child1 Y=0");

    ASSERT_EQ(Int32(70), child2->Bounds().x, "Child2 X=70 (first row)");
    ASSERT_EQ(Int32(0), child2->Bounds().y, "Child2 Y=0");

    ASSERT_EQ(Int32(0), child3->Bounds().x, "Child3 X=0 (wrapped to second row)");
    ASSERT_EQ(Int32(50), child3->Bounds().y, "Child3 Y=50");

    ASSERT_EQ(Int32(70), child4->Bounds().x, "Child4 X=70 (second row)");
    ASSERT_EQ(Int32(50), child4->Bounds().y, "Child4 Y=50");

    ASSERT_EQ(Int32(0), child5->Bounds().x, "Child5 X=0 (wrapped to third row)");
    ASSERT_EQ(Int32(100), child5->Bounds().y, "Child5 Y=100");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Column Wrap with Gap                                               */
/******************************************************************************/

void TestColumnWrapWithGap() {
    Test::PrintHeader("Column Wrap with Gap");

    // Container: 200x200, children: 50x70 each, gap: 10
    // With gap, only 2 children fit per column (70 + 10 + 70 = 150 < 200)
    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().wrap = FlexWrap::Wrap;
    root.Layout().gap = 10;

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 50, 70));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // child1: Y=0, X=0
    // child2: Y=70+10=80, X=0
    // child3: Y=0, X=50+10=60 (wrapped, gap applies to cross axis too)

    ASSERT_EQ(Int32(0), child1->Bounds().y, "Child1 Y=0");
    ASSERT_EQ(Int32(0), child1->Bounds().x, "Child1 X=0");

    ASSERT_EQ(Int32(80), child2->Bounds().y, "Child2 Y=80 (70 + gap 10)");
    ASSERT_EQ(Int32(0), child2->Bounds().x, "Child2 X=0");

    ASSERT_EQ(Int32(0), child3->Bounds().y, "Child3 Y=0 (wrapped)");
    ASSERT_EQ(Int32(60), child3->Bounds().x, "Child3 X=60 (50 + gap 10)");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: Column Wrap with Padding                                           */
/******************************************************************************/

void TestColumnWrapWithPadding() {
    Test::PrintHeader("Column Wrap with Padding");

    // Container: 200x200, padding: 20 on all sides
    // Content area: 160x160
    MockControl root(nullptr, Rectangle(0, 0, 200, 200));
    root.Layout().direction = FlexDirection::Column;
    root.Layout().wrap = FlexWrap::Wrap;
    root.Layout().SetPadding(Int32(20));

    // Children 50x70 each - only 2 fit in 160 height (70+70=140 < 160)
    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 50, 70));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 50, 70));

    root.Measure(Int32(200), Int32(200));
    root.Arrange(Rectangle(0, 0, 200, 200));

    // All positions offset by padding (20, 20)
    ASSERT_EQ(Int32(20), child1->Bounds().y, "Child1 Y=20 (padding)");
    ASSERT_EQ(Int32(20), child1->Bounds().x, "Child1 X=20 (padding)");

    ASSERT_EQ(Int32(90), child2->Bounds().y, "Child2 Y=90 (20 + 70)");
    ASSERT_EQ(Int32(20), child2->Bounds().x, "Child2 X=20");

    ASSERT_EQ(Int32(20), child3->Bounds().y, "Child3 Y=20 (wrapped)");
    // Wrapped to next column: X = 20 (padding) + 50 (child width) + 0 (gap) = 70
    ASSERT_EQ(Int32(70), child3->Bounds().x, "Child3 X=70 (wrapped)");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Test: NoWrap is Default Behavior                                         */
/******************************************************************************/

void TestNoWrapDefault() {
    Test::PrintHeader("NoWrap is Default Behavior");

    LayoutProperties props;
    ASSERT(props.wrap == FlexWrap::NoWrap, "Default wrap should be NoWrap");

    // Container: 100x100, 3 children 50x50 each - without wrap, they overflow
    MockControl root(nullptr, Rectangle(0, 0, 100, 100));
    root.Layout().direction = FlexDirection::Column;
    // wrap is NoWrap by default

    MockControl* child1 = new MockControl(&root, Rectangle(0, 0, 50, 50));
    MockControl* child2 = new MockControl(&root, Rectangle(0, 0, 50, 50));
    MockControl* child3 = new MockControl(&root, Rectangle(0, 0, 50, 50));

    root.Measure(Int32(100), Int32(100));
    root.Arrange(Rectangle(0, 0, 100, 100));

    // Without wrap, all children stack vertically even if they overflow
    ASSERT_EQ(Int32(0), child1->Bounds().y, "Child1 Y=0");
    ASSERT_EQ(Int32(50), child2->Bounds().y, "Child2 Y=50");
    ASSERT_EQ(Int32(100), child3->Bounds().y, "Child3 Y=100 (overflows)");

    // All in same column (X=0)
    ASSERT_EQ(Int32(0), child1->Bounds().x, "Child1 X=0");
    ASSERT_EQ(Int32(0), child2->Bounds().x, "Child2 X=0");
    ASSERT_EQ(Int32(0), child3->Bounds().x, "Child3 X=0 (no wrap)");

    Test::PrintSummary();
}

/******************************************************************************/
/*    Main Entry Point                                                         */
/******************************************************************************/

int main() {
    Console::WriteLine("========================================");
    Console::WriteLine("  WinDOS Layout System Tests");
    Console::WriteLine("========================================");

    // Layout Properties Tests
    TestLayoutPropertiesDefaults();
    TestLayoutPropertiesFluentAPI();

    // Measure Pass Tests
    TestMeasureSingleControl();
    TestMeasureFixedSize();
    TestMeasureMinMaxConstraints();
    TestMeasureColumnWithChildren();
    TestMeasureRowWithChildren();

    // Arrange Pass Tests
    TestArrangeBasicPositioning();
    TestArrangeFlexGrow();
    TestArrangeJustifyContentCenter();
    TestArrangeJustifyContentEnd();
    TestArrangeJustifyContentSpaceBetween();
    TestArrangeAlignItemsCenter();
    TestArrangeAlignItemsEnd();
    TestArrangeAlignItemsStretch();

    // Advanced Tests
    TestFloatingControls();
    TestNestedLayout();
    TestPaddingAffectsContentArea();
    TestMarginOnChildren();
    TestPerformLayoutAndInvalidate();
    TestRowLayoutWithGap();

    // Wrap Tests
    TestColumnLayoutWithWrap();
    TestRowLayoutWithWrap();
    TestColumnWrapWithGap();
    TestColumnWrapWithPadding();
    TestNoWrapDefault();

    Console::WriteLine();
    Console::WriteLine("========================================");
    if (Test::AllPassed()) {
        Console::SetForegroundColor(ConsoleColor::Green);
        Console::WriteLine("  All layout tests passed!");
    } else {
        Console::SetForegroundColor(ConsoleColor::Red);
        Console::WriteLine("  Some layout tests failed!");
    }
    Console::ResetColor();
    Console::WriteLine("========================================");

    return Test::AllPassed() ? 0 : 1;
}
