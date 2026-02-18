#ifndef SYSTEM_WINDOWS_FORMS_HPP
#define SYSTEM_WINDOWS_FORMS_HPP

#include "../../Drawing/Drawing.hpp"
#include "../../Devices/Devices.hpp"
#include "../../Array.hpp"
#include "../../String.hpp"

namespace System::Windows::Forms
{

using namespace System::Drawing;
using namespace System::Devices;

// Forward declarations
class Control;
class Desktop;
class DesktopIcon;
class Window;
class TaskBar;
class TaskBarButton;
class Button;
class Picture;
class SpectrumControl;
class StartMenu;
class MenuItem;
class SpatialGrid;

// Click event handler type (C-style function pointer for DOS compatibility)
typedef void (*ClickEventHandler)(Button* sender, void* userData);

/******************************************************************************/
/*    System::Windows::Forms::ControlType                                     */
/******************************************************************************/

// Type identification for safe casting (similar to .NET GetType())
enum class ControlType
{
    Control,        // Base control
    Desktop,        // Desktop surface
    Window,         // Top-level window
    TaskBar,        // Taskbar
    TaskBarButton,  // Taskbar window button
    Button,         // Button control
    Picture,        // Picture/image control
    Spectrum,       // Spectrum gradient control (32-bit)
    StartMenu,      // Start menu popup
    MenuItem        // Menu item
};

/******************************************************************************/
/*    System::Windows::Forms::Layout Enums                                    */
/******************************************************************************/

/**
 * Direction for laying out child controls in a flex container.
 * Similar to CSS flex-direction.
 */
enum class FlexDirection : unsigned char
{
    Row,            // Horizontal layout (left to right)
    Column          // Vertical layout (top to bottom)
};

/**
 * Alignment of children along the main axis.
 * Similar to CSS justify-content.
 */
enum class JustifyContent : unsigned char
{
    Start,          // Pack items at start of main axis
    Center,         // Center items along main axis
    End,            // Pack items at end of main axis
    SpaceBetween,   // Evenly distribute; first at start, last at end
    SpaceAround     // Evenly distribute with equal space around each
};

/**
 * Alignment of children along the cross axis.
 * Similar to CSS align-items.
 */
enum class AlignItems : unsigned char
{
    Start,          // Align to start of cross axis
    Center,         // Center along cross axis
    End,            // Align to end of cross axis
    Stretch         // Stretch to fill cross axis
};

/**
 * How a control's size is determined.
 */
enum class SizeMode : unsigned char
{
    Auto,           // Size based on content/children
    Fixed,          // Size is explicitly set
    Fill            // Fill available space in parent
};

/******************************************************************************/
/*    System::Windows::Forms::LayoutProperties                                */
/******************************************************************************/

/**
 * Layout configuration for a control.
 * Embedded in each Control to configure flexbox-like layout behavior.
 */
struct LayoutProperties
{
    // Container properties (when this control has children)
    FlexDirection direction;        // Layout direction for children
    JustifyContent justifyContent;  // Main axis alignment
    AlignItems alignItems;          // Cross axis alignment
    Int32 gap;                      // Space between children

    // Self properties (when this control is inside a flex container)
    Int32 flexGrow;                 // 0 = don't grow, 1+ = proportional
    Int32 flexShrink;               // 0 = don't shrink, 1+ = proportional

    // Sizing
    SizeMode widthMode;             // How width is determined
    SizeMode heightMode;            // How height is determined
    Int32 minWidth, minHeight;      // Minimum size constraints
    Int32 maxWidth, maxHeight;      // Maximum size constraints

    // Spacing (top, right, bottom, left order)
    Int32 marginTop, marginRight, marginBottom, marginLeft;
    Int32 paddingTop, paddingRight, paddingBottom, paddingLeft;

    // Behavior flags
    bool participatesInLayout;      // false = floating (e.g., Windows)
    bool needsLayout;               // Dirty flag for optimization

    // Default constructor with sensible defaults
    LayoutProperties()
        : direction(FlexDirection::Column)
        , justifyContent(JustifyContent::Start)
        , alignItems(AlignItems::Stretch)
        , gap(0)
        , flexGrow(0)
        , flexShrink(1)
        , widthMode(SizeMode::Auto)
        , heightMode(SizeMode::Auto)
        , minWidth(0)
        , minHeight(0)
        , maxWidth(32767)  // Reasonable max for DOS
        , maxHeight(32767)
        , marginTop(0)
        , marginRight(0)
        , marginBottom(0)
        , marginLeft(0)
        , paddingTop(0)
        , paddingRight(0)
        , paddingBottom(0)
        , paddingLeft(0)
        , participatesInLayout(true)
        , needsLayout(true)
    {
    }

    // Convenience setters for fluent API
    LayoutProperties& SetDirection(FlexDirection dir)
    {
        direction = dir;
        return *this;
    }

    LayoutProperties& SetJustifyContent(JustifyContent jc)
    {
        justifyContent = jc;
        return *this;
    }

    LayoutProperties& SetAlignItems(AlignItems ai)
    {
        alignItems = ai;
        return *this;
    }

    LayoutProperties& SetGap(Int32 g)
    {
        gap = g;
        return *this;
    }

    LayoutProperties& SetFlexGrow(Int32 fg)
    {
        flexGrow = fg;
        return *this;
    }

    LayoutProperties& SetFlexShrink(Int32 fs)
    {
        flexShrink = fs;
        return *this;
    }

    LayoutProperties& SetWidthMode(SizeMode mode)
    {
        widthMode = mode;
        return *this;
    }

    LayoutProperties& SetHeightMode(SizeMode mode)
    {
        heightMode = mode;
        return *this;
    }

    LayoutProperties& SetMinSize(Int32 w, Int32 h)
    {
        minWidth = w;
        minHeight = h;
        return *this;
    }

    LayoutProperties& SetMaxSize(Int32 w, Int32 h)
    {
        maxWidth = w;
        maxHeight = h;
        return *this;
    }

    LayoutProperties& SetMargin(Int32 all)
    {
        marginTop = marginRight = marginBottom = marginLeft = all;
        return *this;
    }

    LayoutProperties& SetMargin(Int32 vertical, Int32 horizontal)
    {
        marginTop = marginBottom = vertical;
        marginRight = marginLeft = horizontal;
        return *this;
    }

    LayoutProperties& SetMargin(Int32 top, Int32 right, Int32 bottom, Int32 left)
    {
        marginTop = top;
        marginRight = right;
        marginBottom = bottom;
        marginLeft = left;
        return *this;
    }

    LayoutProperties& SetPadding(Int32 all)
    {
        paddingTop = paddingRight = paddingBottom = paddingLeft = all;
        return *this;
    }

    LayoutProperties& SetPadding(Int32 vertical, Int32 horizontal)
    {
        paddingTop = paddingBottom = vertical;
        paddingRight = paddingLeft = horizontal;
        return *this;
    }

    LayoutProperties& SetPadding(Int32 top, Int32 right, Int32 bottom, Int32 left)
    {
        paddingTop = top;
        paddingRight = right;
        paddingBottom = bottom;
        paddingLeft = left;
        return *this;
    }

    LayoutProperties& SetParticipatesInLayout(bool participates)
    {
        participatesInLayout = participates;
        return *this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MeasureResult                                   */
/******************************************************************************/

/**
 * Result of measuring a control's preferred size.
 * Used during the Measure pass of layout.
 */
struct MeasureResult
{
    Int32 preferredWidth;   // Preferred width based on content
    Int32 preferredHeight;  // Preferred height based on content

    MeasureResult() : preferredWidth(0), preferredHeight(0)
    {
    }

    MeasureResult(Int32 w, Int32 h) : preferredWidth(w), preferredHeight(h)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::PaintEventArgs                                  */
/******************************************************************************/

class PaintEventArgs
{
public:
    Graphics* graphics;
    Rectangle bounds;

    PaintEventArgs() : graphics(nullptr), bounds()
    {
    }

    PaintEventArgs(Graphics* g, const Rectangle& b) : graphics(g), bounds(b)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MouseEventArgs                                  */
/******************************************************************************/

class MouseEventArgs
{
public:
    Int32 x;
    Int32 y;
    Boolean leftButton;
    Boolean rightButton;

    MouseEventArgs() : x(0), y(0), leftButton(false), rightButton(false)
    {
    }

    MouseEventArgs(Int32 x, Int32 y, Boolean left, Boolean right)
        : x(x), y(y), leftButton(left), rightButton(right)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::KeyboardEventArgs                               */
/******************************************************************************/

class KeyboardEventArgs
{
public:
    Char key;
    Boolean alt;
    Boolean ctrl;
    Boolean shift;

    KeyboardEventArgs() : key('\0'), alt(false), ctrl(false), shift(false)
    {
    }

    KeyboardEventArgs(Char k, Boolean a, Boolean c, Boolean s)
        : key(k), alt(a), ctrl(c), shift(s)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::SpatialGrid                                     */
/******************************************************************************/

/**
 * Grid-based spatial index for O(1) average hit testing.
 * Divides the screen into fixed-size cells and tracks which controls
 * overlap each cell for fast point-to-control queries.
 */
class SpatialGrid
{
public:
    static const int CELL_SIZE = 64;           // Pixels per cell
    static const int MAX_CELLS_X = 16;         // Up to 1024px wide
    static const int MAX_CELLS_Y = 12;         // Up to 768px tall
    static const int MAX_CONTROLS_PER_CELL = 16;

private:
    struct Cell
    {
        Control* controls[MAX_CONTROLS_PER_CELL];
        int count;

        Cell() : count(0)
        {
            for (int i = 0; i < MAX_CONTROLS_PER_CELL; i++)
            {
                controls[i] = nullptr;
            }
        }
    };

    Cell _cells[MAX_CELLS_Y][MAX_CELLS_X];
    Int32 _cellsX;
    Int32 _cellsY;
    Int32 _screenWidth;
    Int32 _screenHeight;

    // Get cell indices for a point
    void GetCellIndex(Int32 x, Int32 y, Int32& cellX, Int32& cellY) const;

    // Get cell range for a rectangle
    void GetCellRange(const Rectangle& bounds, Int32& minX, Int32& minY,
                      Int32& maxX, Int32& maxY) const;

public:
    SpatialGrid();

    // Initialize grid for screen dimensions
    void Initialize(Int32 screenWidth, Int32 screenHeight);

    // Clear all controls from grid
    void Clear();

    // Insert a control into all cells it overlaps
    void Insert(Control* control, const Rectangle& bounds);

    // Remove a control from all cells
    void Remove(Control* control);

    // Find the topmost control at a point (returns nullptr if none)
    // Checks controls in reverse insertion order for z-order
    Control* HitTest(Int32 x, Int32 y) const;
};

/******************************************************************************/
/*    System::Windows::Forms::Control                                         */
/******************************************************************************/

/**
 * Base class for all visual controls in the Windows Forms hierarchy.
 *
 * OWNERSHIP SEMANTICS:
 * --------------------
 * Parent controls OWN their children. When a parent is destroyed, it
 * automatically deletes all its children. This follows the composite
 * pattern used by Windows Forms and WPF.
 *
 * IMPORTANT RULES:
 * 1. Children MUST be heap-allocated using 'new'
 * 2. Do NOT allocate children on the stack - they will be double-freed
 * 3. Do NOT manually delete children - the parent handles this
 * 4. RemoveChild() releases ownership - caller becomes responsible for deletion
 *
 * CORRECT USAGE:
 *   Window* win = new Window(&desktop, bounds);  // Parent owns win
 *   Button* btn = new Button(win, btnBounds);    // Window owns btn
 *   // No need to delete - Desktop destructor cleans up everything
 *
 * INCORRECT USAGE:
 *   Window win(&desktop, bounds);  // WRONG! Stack allocation
 *   Button btn(win, btnBounds);    // WRONG! Will crash on destruction
 *
 * TRANSFERRING OWNERSHIP:
 *   parent->RemoveChild(child);  // Parent releases ownership
 *   delete child;                // Caller must now delete
 *   // OR
 *   newParent->AddChild(child);  // Transfer to new parent
 */
class Control
{
protected:
    Array<Control*> _children;
    Control* _parent;
    Rectangle _bounds;        // Relative to parent's client area
    Rectangle _clientBounds;  // Relative to this control's bounds
    bool _isInvalid;          // Internal flag - stays primitive
    LayoutProperties _layout; // Layout configuration
    MeasureResult _measuredSize;  // Cached measurement result

    // Calculate client bounds from bounds (subclasses override)
    virtual void UpdateClientBounds();

    // Layout helpers
    void ArrangeFlexChildren(const Rectangle& contentArea);

public:
    Control();
    Control(Control* parent, const Rectangle& bounds);
    virtual ~Control();

    // Properties
    Control* Parent() const
    {
        return _parent;
    }

    const Rectangle& Bounds() const
    {
        return _bounds;
    }

    const Rectangle& ClientBounds() const
    {
        return _clientBounds;
    }

    Int32 ChildCount() const
    {
        return Int32(_children.Length());
    }

    Control* GetChild(Int32 index) const;

    // Get absolute screen coordinates
    Rectangle ScreenBounds() const;
    Rectangle ScreenClientBounds() const;

    // Get visible bounds (clipped to parent's client area)
    Rectangle VisibleBounds() const;

    // Bounds manipulation (relative coordinates)
    void SetBounds(const Rectangle& bounds);
    void SetBounds(Int32 x, Int32 y, Int32 width, Int32 height);

    // Child management
    // AddChild: Takes ownership of child. Child will be deleted when parent is destroyed.
    virtual void AddChild(Control* child);
    // RemoveChild: Releases ownership. Caller becomes responsible for deleting the child.
    virtual void RemoveChild(Control* child);

    // Painting
    virtual void OnPaint(PaintEventArgs& e);
    virtual void OnPaintClient(PaintEventArgs& e);
    void Invalidate();
    void Update();

    // Input events (virtual - subclasses override)
    virtual void OnMouse(MouseEventArgs& e);
    virtual void OnKeyboard(KeyboardEventArgs& e);

    // Event propagation with hit-testing
    void NotifyMouse(MouseEventArgs& e);
    void NotifyKeyboard(KeyboardEventArgs& e);

    // Hit testing (uses screen coordinates)
    Boolean HitTest(Int32 x, Int32 y) const;

    // Type identification (similar to .NET GetType())
    virtual ControlType GetControlType() const
    {
        return ControlType::Control;
    }

    // Safe type casting (similar to .NET 'as' operator)
    // Returns nullptr if the control is not of the requested type
    virtual Window* AsWindow()
    {
        return nullptr;
    }

    virtual const Window* AsWindow() const
    {
        return nullptr;
    }

    virtual Button* AsButton()
    {
        return nullptr;
    }

    virtual const Button* AsButton() const
    {
        return nullptr;
    }

    virtual TaskBar* AsTaskBar()
    {
        return nullptr;
    }

    virtual const TaskBar* AsTaskBar() const
    {
        return nullptr;
    }

    virtual Picture* AsPicture()
    {
        return nullptr;
    }

    virtual const Picture* AsPicture() const
    {
        return nullptr;
    }

    // Type checking (similar to .NET 'is' operator)
    Boolean IsWindow() const
    {
        return GetControlType() == ControlType::Window;
    }

    Boolean IsButton() const
    {
        return GetControlType() == ControlType::Button;
    }

    Boolean IsTaskBar() const
    {
        return GetControlType() == ControlType::TaskBar;
    }

    Boolean IsPicture() const
    {
        return GetControlType() == ControlType::Picture;
    }

    Boolean IsDesktop() const
    {
        return GetControlType() == ControlType::Desktop;
    }

    // Layout system
    LayoutProperties& Layout()
    {
        return _layout;
    }

    const LayoutProperties& Layout() const
    {
        return _layout;
    }

    // Two-pass layout algorithm
    virtual MeasureResult Measure(Int32 availableWidth, Int32 availableHeight);
    virtual void Arrange(const Rectangle& finalBounds);

    // Trigger layout recalculation
    void PerformLayout();
    void InvalidateLayout();

    // Get preferred size (override in subclasses for content-based sizing)
    virtual MeasureResult GetPreferredSize() const;
};

/******************************************************************************/
/*    System::Windows::Forms::DesktopIcon                                     */
/******************************************************************************/

/**
 * Represents an icon displayed on the desktop.
 * Icons are displayed in a grid pattern with optional labels.
 */
struct DesktopIcon
{
    Image image;      // 32x32 icon image
    int x;            // X position on desktop
    int y;            // Y position on desktop

    DesktopIcon() : image(), x(0), y(0)
    {
    }

    DesktopIcon(const Image& img, int px, int py) : image(img), x(px), y(py)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Desktop                                         */
/******************************************************************************/

class Desktop : public Control
{
private:
    static const int CURSOR_SIZE = 24;  // Cursor size (24x24 icon)
    static const int ICON_SIZE = 32;    // Desktop icon size
    static const int ICON_SPACING_X = 75;  // Horizontal spacing between icons
    static const int ICON_SPACING_Y = 70;  // Vertical spacing between icons
    static const int ICON_MARGIN_X = 20;   // Left margin for icons
    static const int ICON_MARGIN_Y = 16;   // Top margin for icons

    Color _backgroundColor;
    Window* _focusedWindow;
    Window* _dragWindow;
    Int32 _dragOffsetX;
    Int32 _dragOffsetY;
    Int32 _dragStartX;
    Int32 _dragStartY;
    Image _dragBitmap;        // Captured window image during drag
    Image _cursorImage;       // Cursor icon (24x24)
    Array<DesktopIcon> _icons;  // Desktop icons
    bool _isDragging;         // Internal flag - stays primitive
    bool _running;            // Internal flag - stays primitive
    Int32 _cursorX;
    Int32 _cursorY;
    Int32 _prevCursorX;
    Int32 _prevCursorY;
    bool _cursorVisible;      // Internal flag - stays primitive
    bool _wasMouseDown;       // Internal flag - stays primitive
    unsigned int _cursorSave[CURSOR_SIZE * CURSOR_SIZE];  // Pixels under cursor (32-bit ARGB)
    bool _cursorSaved;        // Internal flag - stays primitive
    Int32 _screenWidth;       // Cached from Display
    Int32 _screenHeight;
    Int32 _nextIconX;         // Next icon position
    Int32 _nextIconY;

    // Spatial partitioning for fast hit testing
    SpatialGrid _spatialGrid;

    // References to special controls (not owned - they're in _children)
    TaskBar* _taskBar;
    StartMenu* _startMenu;

    void CheckForUpdates();
    void SaveUnderCursor();
    void RestoreCursor();
    void DrawCursor();
    void DrawIcons();
    void CaptureWindowBitmap(Window* win);
    void DrawDragBitmap();
    void UpdateSpatialGrid();

public:
    Desktop(const Color& backgroundColor = Color::Cyan);
    virtual ~Desktop();

    // Properties
    Window* FocusedWindow() const
    {
        return _focusedWindow;
    }

    void SetFocusedWindow(Window* window);

    Int32 ScreenWidth() const
    {
        return Int32(_screenWidth);
    }

    Int32 ScreenHeight() const
    {
        return Int32(_screenHeight);
    }

    Int32 IconCount() const
    {
        return Int32(_icons.Length());
    }

    // Cursor
    void SetCursor(const Image& cursorImage);
    void LoadCursorFromLibrary(const char* path, Int32 iconIndex);
    void LoadCursorFromLibrary(const char* path, const char* iconName);

    // Desktop icons
    void AddIcon(const Image& icon);
    void AddIconFromLibrary(const char* path, Int32 iconIndex);
    void AddIconFromLibrary(const char* path, const char* iconName);

    // TaskBar and StartMenu integration
    void SetTaskBar(TaskBar* taskBar)
    {
        _taskBar = taskBar;
    }

    TaskBar* GetTaskBar() const
    {
        return _taskBar;
    }

    void SetStartMenu(StartMenu* menu)
    {
        _startMenu = menu;
    }

    StartMenu* GetStartMenu() const
    {
        return _startMenu;
    }

    // Override child management to update spatial grid and taskbar
    virtual void AddChild(Control* child) override;
    virtual void RemoveChild(Control* child) override;

    // Override paint and input
    virtual void OnPaint(PaintEventArgs& e) override;
    virtual void OnKeyboard(KeyboardEventArgs& e) override;
    void HandleMouse(MouseEventArgs& e);  // Focus handling before child dispatch

    // Main event loop
    void Run();
    void Stop();

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::Desktop;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Window                                          */
/******************************************************************************/

class Window : public Control
{
private:
    static const int TITLE_BAR_HEIGHT = 20;  // Internal constant - stays primitive
    static const int FRAME_WIDTH = 3;
    bool _isFocused;
    String _title;             // Window title
    Font _font;                // Font for title rendering

protected:
    virtual void UpdateClientBounds() override;

public:
    Window(Control* parent, const Rectangle& bounds);
    virtual ~Window();

    // Title property
    const String& Title() const
    {
        return _title;
    }

    void SetTitle(const String& title)
    {
        _title = title;
        Invalidate();
    }

    void SetTitle(const char* title)
    {
        _title = String(title);
        Invalidate();
    }

    // Font property
    const Font& GetFont() const
    {
        return _font;
    }

    void SetFont(const Font& font)
    {
        _font = font;
        Invalidate();
    }

    Boolean IsFocused() const
    {
        return Boolean(_isFocused);
    }

    void SetFocused(Boolean focused)
    {
        _isFocused = static_cast<bool>(focused);
    }

    // Override paint
    virtual void OnPaint(PaintEventArgs& e) override;
    virtual void OnMouse(MouseEventArgs& e) override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::Window;
    }

    virtual Window* AsWindow() override
    {
        return this;
    }

    virtual const Window* AsWindow() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::TaskBar                                         */
/******************************************************************************/

class TaskBar : public Control
{
private:
    static const int WINDOW_BUTTON_WIDTH = 120;
    static const int WINDOW_BUTTON_HEIGHT = 20;
    static const int WINDOW_BUTTON_SPACING = 2;
    static const int WINDOW_BUTTON_START_X = 62;  // After Start button

    Button* _startButton;
    StartMenu* _startMenu;       // Reference to start menu (not owned)
    Desktop* _desktop;           // Back-reference to desktop
    Array<TaskBarButton*> _windowButtons;

    static void OnStartButtonClick(Button* sender, void* userData);

public:
    TaskBar(Control* parent, StartMenu* startMenu = nullptr);
    virtual ~TaskBar();

    // Override paint
    virtual void OnPaint(PaintEventArgs& e) override;

    Button* StartButton() const
    {
        return _startButton;
    }

    // Desktop back-reference (set by Desktop::SetTaskBar)
    void SetDesktop(Desktop* desktop)
    {
        _desktop = desktop;
    }

    Desktop* GetDesktop() const
    {
        return _desktop;
    }

    // Window button management
    void AddWindowButton(Window* window);
    void RemoveWindowButton(Window* window);
    void RefreshWindowButtons();  // Update pressed states based on focus
    TaskBarButton* FindButtonForWindow(Window* window) const;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::TaskBar;
    }

    virtual TaskBar* AsTaskBar() override
    {
        return this;
    }

    virtual const TaskBar* AsTaskBar() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Button                                          */
/******************************************************************************/

class Button : public Control
{
private:
    bool _isToggled;           // Persistent pressed state (e.g., active window, menu open)
    bool _isMouseDown;         // Temporary state during mouse click
    bool _wasMouseDown;        // For click detection
    ClickEventHandler _onClick;
    void* _onClickUserData;
    String _text;              // Button text
    Font _font;                // Font for text rendering

public:
    Button(Control* parent, const Rectangle& bounds);
    virtual ~Button();

    // Text property
    const String& Text() const
    {
        return _text;
    }

    void SetText(const String& text)
    {
        _text = text;
        Invalidate();
    }

    void SetText(const char* text)
    {
        _text = String(text);
        Invalidate();
    }

    // Font property
    const Font& GetFont() const
    {
        return _font;
    }

    void SetFont(const Font& font)
    {
        _font = font;
        Invalidate();
    }

    // Visual pressed state is toggled OR mouse down
    Boolean IsPressed() const
    {
        return Boolean(_isToggled || _isMouseDown);
    }

    // Set persistent pressed state (for TaskBarButton to show active window)
    void SetPressed(Boolean pressed)
    {
        _isToggled = static_cast<bool>(pressed);
        Invalidate();
    }

    // Click event handler
    void SetOnClick(ClickEventHandler handler, void* userData = nullptr);

    // Override paint and input
    virtual void OnPaint(PaintEventArgs& e) override;
    virtual void OnMouse(MouseEventArgs& e) override;

    // Layout: buttons report their bounds as preferred size by default
    virtual MeasureResult GetPreferredSize() const override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::Button;
    }

    virtual Button* AsButton() override
    {
        return this;
    }

    virtual const Button* AsButton() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Picture                                         */
/******************************************************************************/

class Picture : public Control
{
private:
    Image _image;

public:
    Picture(Control* parent, const Rectangle& bounds);
    Picture(Control* parent, const Rectangle& bounds, const Image& image);
    virtual ~Picture();

    void SetImage(const Image& image);

    const Image& GetImage() const
    {
        return _image;
    }

    // Override paint
    virtual void OnPaint(PaintEventArgs& e) override;

    // Layout: pictures prefer their image size
    virtual MeasureResult GetPreferredSize() const override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::Picture;
    }

    virtual Picture* AsPicture() override
    {
        return this;
    }

    virtual const Picture* AsPicture() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::SpectrumControl                                 */
/******************************************************************************/

/**
 * A control that displays a vertical color gradient:
 * - Top: 100% white
 * - Middle: base color
 * - Bottom: 100% black
 *
 * This control demonstrates 32-bit color rendering with smooth gradients.
 * The gradient is pre-computed and cached as an Image for efficient drawing.
 */
class SpectrumControl : public Control
{
private:
    Color _baseColor;
    Image _gradient;  // Cached gradient image

    void RegenerateGradient();

public:
    SpectrumControl(Control* parent, const Rectangle& bounds,
                    const Color& baseColor = Color::Red);
    virtual ~SpectrumControl();

    // Properties
    Color GetBaseColor() const
    {
        return _baseColor;
    }

    void SetBaseColor(const Color& color);

    // Get the interpolated color at a given Y position (0 = top, height-1 = bottom)
    Color GetColorAtY(Int32 y) const;

    // Override paint
    virtual void OnPaint(PaintEventArgs& e) override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::Spectrum;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::TaskBarButton                                   */
/******************************************************************************/

/**
 * A specialized button that represents an open window in the taskbar.
 * Shows as pressed/sunken when the associated window is focused.
 * When pressed, displays a checkerboard hatch pattern (Windows 95 style).
 */
class TaskBarButton : public Button
{
private:
    Window* _window;  // Reference to window (not owned)

    static void OnTaskBarButtonClick(Button* sender, void* userData);

public:
    TaskBarButton(Control* parent, const Rectangle& bounds, Window* window);
    virtual ~TaskBarButton();

    Window* GetWindow() const
    {
        return _window;
    }

    // Override paint to use hatch pattern when pressed
    virtual void OnPaint(PaintEventArgs& e) override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::TaskBarButton;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MenuItem                                        */
/******************************************************************************/

/**
 * A menu item that displays an icon and text.
 * Highlights on hover and fires click event when clicked.
 */
class MenuItem : public Control
{
private:
    static const int ICON_SIZE = 16;
    static const int ITEM_HEIGHT = 24;
    static const int ICON_MARGIN = 4;

    Image _icon;
    bool _isHighlighted;
    ClickEventHandler _onClick;
    void* _onClickUserData;
    int _itemIndex;  // Index in menu for icon selection

public:
    MenuItem(Control* parent, const Rectangle& bounds, int itemIndex);
    virtual ~MenuItem();

    void SetIcon(const Image& icon);
    void SetOnClick(ClickEventHandler handler, void* userData = nullptr);

    // Override paint and input
    virtual void OnPaint(PaintEventArgs& e) override;
    virtual void OnMouse(MouseEventArgs& e) override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::MenuItem;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::StartMenu                                       */
/******************************************************************************/

/**
 * Windows 95-style Start menu popup.
 * Displays a blue sidebar on the left with "Windows 95" text (rotated)
 * and 12 menu items with icons loaded from sysicons.icl.
 */
class StartMenu : public Control
{
private:
    static const int MENU_WIDTH = 160;
    static const int SIDEBAR_WIDTH = 24;
    static const int ITEM_COUNT = 12;
    static const int ITEM_HEIGHT = 24;

    Desktop* _desktop;  // Back-reference to desktop (not owned)
    bool _isVisible;
    Array<MenuItem*> _items;

    void LoadIcons();

public:
    StartMenu(Desktop* desktop);
    virtual ~StartMenu();

    Boolean IsVisible() const
    {
        return Boolean(_isVisible);
    }

    void Show();
    void Hide();
    void Toggle();

    // Override paint and input
    virtual void OnPaint(PaintEventArgs& e) override;
    virtual void OnMouse(MouseEventArgs& e) override;

    // Type identification
    virtual ControlType GetControlType() const override
    {
        return ControlType::StartMenu;
    }
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_HPP
