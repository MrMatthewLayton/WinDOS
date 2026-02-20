#ifndef SYSTEM_WINDOWS_FORMS_HPP
#define SYSTEM_WINDOWS_FORMS_HPP

/// @file Forms.hpp
/// @brief Windows Forms GUI framework for DOS
/// @details Provides a Windows 95-inspired graphical user interface system
/// implementing the WIMP (Windows, Icons, Menus, Pointer) paradigm.
/// This framework uses a .NET-style API design and runs in DOS protected mode.

#include "../../Drawing/Drawing.hpp"
#include "../../IO/Devices/Display.hpp"
#include "../../IO/Devices/Mouse.hpp"
#include "../../IO/Devices/Keyboard.hpp"
#include "../../Array.hpp"
#include "../../String.hpp"
#include "ControlTypes.hpp"
#include "LayoutEnums.hpp"
#include "LayoutProperties.hpp"
#include "EventArgs.hpp"

namespace System::Windows::Forms
{

using namespace System::Drawing;
using namespace System::IO::Devices;

// Forward declarations
class Control;
class Desktop;
class DesktopIcon;
class DesktopIconControl;
class Window;
class TaskBar;
class TaskBarButton;
class TaskTray;
class Button;
class Picture;
class SpectrumControl;
class StartMenu;
class MenuItem;
class SpatialGrid;

/// @brief Function pointer type for button click event handlers.
/// @details Uses C-style function pointers for DOS compatibility.
/// @param sender Pointer to the Button that was clicked.
/// @param userData User-provided context data passed to the handler.
typedef void (*ClickEventHandler)(Button* sender, void* userData);

/******************************************************************************/
/*    System::Windows::Forms::SpatialGrid                                     */
/******************************************************************************/

/// @brief Grid-based spatial index for O(1) average hit testing.
/// @details Divides the screen into fixed-size cells and tracks which controls
/// overlap each cell. Enables fast point-to-control queries without iterating
/// through all controls. Used by Desktop for efficient mouse event routing.
///
/// The grid uses 64x64 pixel cells and supports screens up to 1024x768.
/// When a point is tested, only controls in the relevant cell are checked.
class SpatialGrid
{
public:
    static const int CELL_SIZE = 64;           ///< Pixels per grid cell
    static const int MAX_CELLS_X = 16;         ///< Maximum horizontal cells (1024px)
    static const int MAX_CELLS_Y = 12;         ///< Maximum vertical cells (768px)
    static const int MAX_CONTROLS_PER_CELL = 16; ///< Max controls per cell

private:
    /// @brief Internal cell structure containing control references.
    struct Cell
    {
        Control* controls[MAX_CONTROLS_PER_CELL]; ///< Controls overlapping this cell
        int count;                                 ///< Number of controls in cell

        /// @brief Default constructor initializing empty cell.
        Cell() : count(0)
        {
            for (int i = 0; i < MAX_CONTROLS_PER_CELL; i++)
            {
                controls[i] = nullptr;
            }
        }
    };

    Cell _cells[MAX_CELLS_Y][MAX_CELLS_X];  ///< 2D grid of cells
    Int32 _cellsX;                           ///< Active cells horizontally
    Int32 _cellsY;                           ///< Active cells vertically
    Int32 _screenWidth;                      ///< Screen width for bounds checking
    Int32 _screenHeight;                     ///< Screen height for bounds checking

    /// @brief Get cell indices for a screen point.
    /// @param x Screen X coordinate.
    /// @param y Screen Y coordinate.
    /// @param cellX Output cell X index.
    /// @param cellY Output cell Y index.
    void GetCellIndex(Int32 x, Int32 y, Int32& cellX, Int32& cellY) const;

    /// @brief Get cell range that a rectangle overlaps.
    /// @param bounds Rectangle in screen coordinates.
    /// @param minX Output minimum cell X index.
    /// @param minY Output minimum cell Y index.
    /// @param maxX Output maximum cell X index.
    /// @param maxY Output maximum cell Y index.
    void GetCellRange(const Rectangle& bounds, Int32& minX, Int32& minY,
                      Int32& maxX, Int32& maxY) const;

public:
    /// @brief Default constructor.
    SpatialGrid();

    /// @brief Initialize the grid for given screen dimensions.
    /// @param screenWidth Screen width in pixels.
    /// @param screenHeight Screen height in pixels.
    void Initialize(Int32 screenWidth, Int32 screenHeight);

    /// @brief Clear all controls from the grid.
    void Clear();

    /// @brief Insert a control into all cells it overlaps.
    /// @param control Pointer to the control to insert.
    /// @param bounds Control bounds in screen coordinates.
    void Insert(Control* control, const Rectangle& bounds);

    /// @brief Remove a control from all cells.
    /// @param control Pointer to the control to remove.
    void Remove(Control* control);

    /// @brief Find the topmost control at a point.
    /// @param x Screen X coordinate.
    /// @param y Screen Y coordinate.
    /// @return Pointer to topmost control at point, or nullptr if none.
    /// @note Checks controls in reverse insertion order for z-order.
    Control* HitTest(Int32 x, Int32 y) const;
};

/******************************************************************************/
/*    System::Windows::Forms::Control                                         */
/******************************************************************************/

/// @brief Base class for all visual controls in the Windows Forms hierarchy.
/// @details Provides the foundation for all UI elements including bounds
/// management, parent-child relationships, painting, and event handling.
/// Similar to System.Windows.Forms.Control in .NET Framework.
///
/// @note OWNERSHIP SEMANTICS:
/// Parent controls OWN their children. When a parent is destroyed, it
/// automatically deletes all its children. This follows the composite
/// pattern used by Windows Forms and WPF.
///
/// @note IMPORTANT RULES:
/// - Children MUST be heap-allocated using 'new'
/// - Do NOT allocate children on the stack - they will be double-freed
/// - Do NOT manually delete children - the parent handles this
/// - RemoveChild() releases ownership - caller becomes responsible for deletion
///
/// @par Correct Usage:
/// @code
/// Window* win = new Window(&desktop, bounds);  // Parent owns win
/// Button* btn = new Button(win, btnBounds);    // Window owns btn
/// // No need to delete - Desktop destructor cleans up everything
/// @endcode
///
/// @par Incorrect Usage:
/// @code
/// Window win(&desktop, bounds);  // WRONG! Stack allocation
/// Button btn(win, btnBounds);    // WRONG! Will crash on destruction
/// @endcode
///
/// @par Transferring Ownership:
/// @code
/// parent->RemoveChild(child);  // Parent releases ownership
/// delete child;                // Caller must now delete
/// // OR
/// newParent->AddChild(child);  // Transfer to new parent
/// @endcode
class Control
{
protected:
    Array<Control*> _children;    ///< Child controls (owned by this control)
    Control* _parent;             ///< Parent control (not owned, may be nullptr)
    Rectangle _bounds;            ///< Bounds relative to parent's client area
    Rectangle _clientBounds;      ///< Client bounds relative to this control's bounds
    bool _isInvalid;              ///< Dirty flag indicating repaint needed
    bool _visible;                ///< True if control is visible
    LayoutProperties _layout;     ///< Layout configuration for flexbox behavior
    MeasureResult _measuredSize;  ///< Cached measurement result

    /// @brief Calculate client bounds from control bounds.
    /// @details Override in subclasses to account for borders, title bars, etc.
    virtual void UpdateClientBounds();

    /// @brief Arrange children using flexbox layout algorithm.
    /// @param contentArea The area available for laying out children.
    void ArrangeFlexChildren(const Rectangle& contentArea);

public:
    /// @brief Default constructor creating unparented control at origin.
    Control();

    /// @brief Constructor with parent and bounds.
    /// @param parent Parent control that will own this control (or nullptr).
    /// @param bounds Initial bounds relative to parent's client area.
    /// @note If parent is non-null, this control is automatically added as a child.
    Control(Control* parent, const Rectangle& bounds);

    /// @brief Virtual destructor that deletes all child controls.
    /// @note All children are deleted recursively.
    virtual ~Control();

    /// @brief Get the parent control.
    /// @return Pointer to parent control, or nullptr if this is a root control.
    Control* Parent() const
    {
        return _parent;
    }

    /// @brief Get the control bounds relative to parent.
    /// @return Rectangle representing position and size within parent.
    const Rectangle& Bounds() const
    {
        return _bounds;
    }

    /// @brief Get the client area bounds.
    /// @return Rectangle representing the client area within this control.
    /// @note Client area excludes borders, title bars, etc.
    const Rectangle& ClientBounds() const
    {
        return _clientBounds;
    }

    /// @brief Get the number of child controls.
    /// @return Number of children as Int32.
    Int32 ChildCount() const
    {
        return Int32(_children.Length());
    }

    /// @brief Get a child control by index.
    /// @param index Zero-based index of the child.
    /// @return Pointer to the child control.
    /// @throws IndexOutOfRangeException if index is invalid.
    Control* GetChild(Int32 index) const;

    /// @brief Get bounds in absolute screen coordinates.
    /// @return Rectangle in screen space.
    Rectangle ScreenBounds() const;

    /// @brief Get client bounds in absolute screen coordinates.
    /// @return Rectangle representing client area in screen space.
    Rectangle ScreenClientBounds() const;

    /// @brief Get visible bounds clipped to parent's client area.
    /// @return Rectangle representing the visible portion of this control.
    Rectangle VisibleBounds() const;

    /// @brief Set the control bounds.
    /// @param bounds New bounds relative to parent's client area.
    void SetBounds(const Rectangle& bounds);

    /// @brief Set the control bounds with individual components.
    /// @param x X position relative to parent.
    /// @param y Y position relative to parent.
    /// @param width Width in pixels.
    /// @param height Height in pixels.
    void SetBounds(Int32 x, Int32 y, Int32 width, Int32 height);

    /// @brief Add a child control.
    /// @param child Pointer to the child control to add.
    /// @note This control takes ownership of the child and will delete it.
    /// @note Child must be heap-allocated with 'new'.
    virtual void AddChild(Control* child);

    /// @brief Remove a child control.
    /// @param child Pointer to the child control to remove.
    /// @note Releases ownership - caller becomes responsible for deletion.
    virtual void RemoveChild(Control* child);

    /// @brief Handle paint event for this control's non-client area.
    /// @param e Paint event arguments with Graphics context.
    virtual void OnPaint(PaintEventArgs& e);

    /// @brief Handle paint event for this control's client area.
    /// @param e Paint event arguments with Graphics context.
    virtual void OnPaintClient(PaintEventArgs& e);

    /// @brief Check if control is visible.
    /// @return True if visible.
    Boolean IsVisible() const
    {
        return Boolean(_visible);
    }

    /// @brief Set control visibility.
    /// @param visible New visibility state.
    void SetVisible(Boolean visible)
    {
        _visible = static_cast<bool>(visible);
        Invalidate();
    }

    /// @brief Mark this control as needing repaint.
    void Invalidate();

    /// @brief Force immediate repaint of this control and children.
    void Update();

    /// @brief Handle mouse input event.
    /// @param e Mouse event arguments with position and button states.
    virtual void OnMouse(MouseEventArgs& e);

    /// @brief Handle keyboard input event.
    /// @param e Keyboard event arguments with key and modifiers.
    virtual void OnKeyboard(KeyboardEventArgs& e);

    /// @brief Propagate mouse event to appropriate child control.
    /// @param e Mouse event arguments.
    void NotifyMouse(MouseEventArgs& e);

    /// @brief Propagate keyboard event to appropriate child control.
    /// @param e Keyboard event arguments.
    void NotifyKeyboard(KeyboardEventArgs& e);

    /// @brief Test if a screen point is within this control's bounds.
    /// @param x Screen X coordinate.
    /// @param y Screen Y coordinate.
    /// @return True if point is within bounds.
    Boolean HitTest(Int32 x, Int32 y) const;

    /// @brief Get the runtime type of this control.
    /// @return ControlType enumeration value.
    /// @note Similar to .NET GetType() for type identification.
    virtual ControlType GetControlType() const
    {
        return ControlType::Control;
    }

    /// @brief Safe downcast to Window.
    /// @return Pointer to this as Window, or nullptr if not a Window.
    /// @note Similar to .NET 'as' operator.
    virtual Window* AsWindow()
    {
        return nullptr;
    }

    /// @brief Safe downcast to Window (const version).
    /// @return Const pointer to this as Window, or nullptr if not a Window.
    virtual const Window* AsWindow() const
    {
        return nullptr;
    }

    /// @brief Safe downcast to Button.
    /// @return Pointer to this as Button, or nullptr if not a Button.
    virtual Button* AsButton()
    {
        return nullptr;
    }

    /// @brief Safe downcast to Button (const version).
    /// @return Const pointer to this as Button, or nullptr if not a Button.
    virtual const Button* AsButton() const
    {
        return nullptr;
    }

    /// @brief Safe downcast to TaskBar.
    /// @return Pointer to this as TaskBar, or nullptr if not a TaskBar.
    virtual TaskBar* AsTaskBar()
    {
        return nullptr;
    }

    /// @brief Safe downcast to TaskBar (const version).
    /// @return Const pointer to this as TaskBar, or nullptr if not a TaskBar.
    virtual const TaskBar* AsTaskBar() const
    {
        return nullptr;
    }

    /// @brief Safe downcast to Desktop.
    /// @return Pointer to this as Desktop, or nullptr if not a Desktop.
    virtual Desktop* AsDesktop()
    {
        return nullptr;
    }

    /// @brief Safe downcast to Desktop (const version).
    /// @return Const pointer to this as Desktop, or nullptr if not a Desktop.
    virtual const Desktop* AsDesktop() const
    {
        return nullptr;
    }

    /// @brief Safe downcast to Picture.
    /// @return Pointer to this as Picture, or nullptr if not a Picture.
    virtual Picture* AsPicture()
    {
        return nullptr;
    }

    /// @brief Safe downcast to Picture (const version).
    /// @return Const pointer to this as Picture, or nullptr if not a Picture.
    virtual const Picture* AsPicture() const
    {
        return nullptr;
    }

    /// @brief Check if this control is a Window.
    /// @return True if this is a Window or Window subclass.
    Boolean IsWindow() const
    {
        return GetControlType() == ControlType::Window;
    }

    /// @brief Check if this control is a Button.
    /// @return True if this is a Button or Button subclass.
    Boolean IsButton() const
    {
        return GetControlType() == ControlType::Button;
    }

    /// @brief Check if this control is a TaskBar.
    /// @return True if this is a TaskBar.
    Boolean IsTaskBar() const
    {
        return GetControlType() == ControlType::TaskBar;
    }

    /// @brief Check if this control is a Picture.
    /// @return True if this is a Picture.
    Boolean IsPicture() const
    {
        return GetControlType() == ControlType::Picture;
    }

    /// @brief Check if this control is a Desktop.
    /// @return True if this is a Desktop.
    Boolean IsDesktop() const
    {
        return GetControlType() == ControlType::Desktop;
    }

    /// @brief Get mutable reference to layout properties.
    /// @return Reference to this control's LayoutProperties.
    LayoutProperties& Layout()
    {
        return _layout;
    }

    /// @brief Get const reference to layout properties.
    /// @return Const reference to this control's LayoutProperties.
    const LayoutProperties& Layout() const
    {
        return _layout;
    }

    /// @brief Measure pass of layout algorithm.
    /// @param availableWidth Available width from parent (pixels).
    /// @param availableHeight Available height from parent (pixels).
    /// @return MeasureResult with preferred dimensions.
    /// @note Called bottom-up during layout; children measured first.
    virtual MeasureResult Measure(Int32 availableWidth, Int32 availableHeight);

    /// @brief Arrange pass of layout algorithm.
    /// @param finalBounds Final bounds assigned by parent.
    /// @note Called top-down during layout; parent arranges children.
    virtual void Arrange(const Rectangle& finalBounds);

    /// @brief Trigger complete layout recalculation.
    /// @note Performs both Measure and Arrange passes.
    void PerformLayout();

    /// @brief Mark layout as needing recalculation.
    void InvalidateLayout();

    /// @brief Get preferred size based on content.
    /// @return MeasureResult with preferred dimensions.
    /// @note Override in subclasses for content-based sizing.
    virtual MeasureResult GetPreferredSize() const;
};

/******************************************************************************/
/*    System::Windows::Forms::DesktopIcon                                     */
/******************************************************************************/

/// @brief Represents an icon displayed on the desktop surface.
/// @details Desktop icons are displayed in a grid pattern with optional labels.
/// Icons are typically 32x32 pixels and loaded from icon libraries.
/// @deprecated Use DesktopIconControl for new code; this struct is for compatibility.
struct DesktopIcon
{
    Image image;      ///< Icon image (typically 32x32)
    int x;            ///< X position on desktop (pixels)
    int y;            ///< Y position on desktop (pixels)

    /// @brief Default constructor.
    DesktopIcon() : image(), x(0), y(0)
    {
    }

    /// @brief Constructor with image and position.
    /// @param img The icon image.
    /// @param px X position on desktop.
    /// @param py Y position on desktop.
    DesktopIcon(const Image& img, int px, int py) : image(img), x(px), y(py)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::DesktopIconControl                              */
/******************************************************************************/

/// @brief Control that displays a desktop icon with image and text label.
/// @details Used by Desktop to display icons in a flexbox layout with wrapping.
/// Each icon has a fixed cell size (64x96 pixels):
/// - Top area (64x64): Contains 32x32 icon with 16px padding all around
/// - Bottom area (64x32): Contains centered, wrapped text with ellipsis
///
/// Icons participate in layout by default and report their cell size as
/// preferred size, enabling automatic arrangement via flex layout.
class DesktopIconControl : public Control
{
    static const int ICON_SIZE = 32;       ///< Icon image size (32x32)
    static const int CELL_WIDTH = 64;      ///< Icon cell width
    static const int CELL_HEIGHT = 68;     ///< Icon cell height
    static const int ICON_AREA_HEIGHT = 44;///< Height of icon area
    static const int TEXT_AREA_HEIGHT = 24;///< Height of text area
    static const int ICON_PADDING = 6;     ///< Padding around icon in icon area

    Image _icon;           ///< Icon image to display
    String _text;          ///< Text label below icon
    Font _font;            ///< Font for text (MS Sans Serif)
    bool _isSelected;      ///< True if icon is selected

    /// @brief Truncate text with ellipsis if it exceeds available width.
    /// @param text Text to truncate.
    /// @param maxWidth Maximum width in pixels.
    /// @return Truncated text with "..." if needed.
    String TruncateWithEllipsis(const String& text, Int32 maxWidth) const;

public:
    /// @brief Constructor with parent and icon image.
    /// @param parent Parent control (typically icon container in Desktop).
    /// @param icon Icon image to display.
    DesktopIconControl(Control* parent, const Image& icon);

    /// @brief Constructor with parent, icon image, and text label.
    /// @param parent Parent control (typically icon container in Desktop).
    /// @param icon Icon image to display.
    /// @param text Text label for the icon.
    DesktopIconControl(Control* parent, const Image& icon, const String& text);

    /// @brief Destructor.
    virtual ~DesktopIconControl() = default;

    /// @brief Get the icon image.
    /// @return Const reference to the icon image.
    const Image& GetIcon() const
    {
        return _icon;
    }

    /// @brief Set the icon image.
    /// @param icon New icon image.
    void SetIcon(const Image& icon)
    {
        _icon = icon;
        Invalidate();
    }

    /// @brief Get the text label.
    /// @return Const reference to the text.
    const String& GetText() const
    {
        return _text;
    }

    /// @brief Set the text label.
    /// @param text New text label.
    void SetText(const String& text)
    {
        _text = text;
        Invalidate();
    }

    /// @brief Check if the icon is selected.
    /// @return True if selected.
    Boolean IsSelected() const
    {
        return Boolean(_isSelected);
    }

    /// @brief Set the selection state.
    /// @param selected New selection state.
    void SetSelected(Boolean selected)
    {
        _isSelected = static_cast<bool>(selected);
        Invalidate();
    }

    /// @brief Paint the icon and text in its cell.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Handle mouse events for selection.
    /// @param e Mouse event arguments.
    virtual void OnMouse(MouseEventArgs& e) override;

    /// @brief Get preferred size for layout (cell size).
    /// @return MeasureResult with cell dimensions.
    virtual MeasureResult GetPreferredSize() const override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::DesktopIconControl.
    virtual ControlType GetControlType() const override
    {
        return ControlType::DesktopIconControl;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Desktop                                         */
/******************************************************************************/

/// @brief Root control representing the desktop surface.
/// @details The Desktop is the root of the control hierarchy and manages:
/// - Window focus and z-order
/// - Window dragging with bitmap capture
/// - Mouse cursor rendering
/// - Desktop icons
/// - TaskBar and StartMenu integration
/// - Main event loop (Run/Stop)
///
/// The Desktop uses a spatial grid for efficient hit testing with many
/// windows open. It also handles keyboard shortcuts (Escape to exit).
///
/// @note Desktop does not have a parent and is typically created on the stack.
class Desktop : public Control
{
    static const int CURSOR_SIZE = 32;     ///< Cursor size (32x32 pixels)
    static const int ICON_SIZE = 32;       ///< Desktop icon size
    static const int ICON_CELL_WIDTH = 64; ///< Icon cell width (includes text area)
    static const int ICON_CELL_HEIGHT = 68;///< Icon cell height (44 icon + 24 text)
    static const int ICON_SPACING_X = 75;  ///< Legacy horizontal icon spacing
    static const int ICON_SPACING_Y = 70;  ///< Legacy vertical icon spacing
    static const int ICON_MARGIN_X = 8;    ///< Left margin for icon grid
    static const int ICON_MARGIN_Y = 8;    ///< Top margin for icon grid
    static const int TASKBAR_HEIGHT = 32;  ///< TaskBar height in pixels

    Color _backgroundColor;         ///< Desktop background color
    Image _wallpaper;               ///< Desktop wallpaper image (optional)
    Window* _focusedWindow;         ///< Currently focused window
    Window* _dragWindow;            ///< Window being dragged
    Int32 _dragOffsetX;             ///< Drag offset from window origin X
    Int32 _dragOffsetY;             ///< Drag offset from window origin Y
    Int32 _dragStartX;              ///< Drag start position X
    Int32 _dragStartY;              ///< Drag start position Y
    Image _dragBitmap;              ///< Captured window bitmap during drag
    Image _cursorImage;             ///< Mouse cursor image (24x24)
    Array<DesktopIcon> _icons;      ///< Desktop icons (legacy, for backward compat)
    bool _isDragging;               ///< True if window is being dragged
    bool _running;                  ///< True if event loop is running
    Int32 _cursorX;                 ///< Current cursor X position
    Int32 _cursorY;                 ///< Current cursor Y position
    Int32 _prevCursorX;             ///< Previous cursor X position
    Int32 _prevCursorY;             ///< Previous cursor Y position
    bool _cursorVisible;            ///< True if cursor should be drawn
    bool _wasMouseDown;             ///< Previous mouse button state
    unsigned int _cursorSave[CURSOR_SIZE * CURSOR_SIZE]; ///< Pixels under cursor
    bool _cursorSaved;              ///< True if cursor save is valid
    Int32 _screenWidth;             ///< Cached screen width
    Int32 _screenHeight;            ///< Cached screen height
    Int32 _nextIconX;               ///< Next icon grid X position (legacy)
    Int32 _nextIconY;               ///< Next icon grid Y position (legacy)

    SpatialGrid _spatialGrid;       ///< Spatial index for hit testing

    // References to special controls (not owned - they're in _children)
    TaskBar* _taskBar;              ///< Reference to taskbar
    StartMenu* _startMenu;          ///< Reference to start menu
    Control* _iconContainer;        ///< Container for desktop icons with flex layout
    DesktopIconControl* _selectedIcon; ///< Currently selected desktop icon
    Drawing::IconLibrary* _iconLibrary; ///< System icon library (owned)

    /// @brief Check for controls needing repaint.
    void CheckForUpdates();

    /// @brief Save pixels under cursor position.
    void SaveUnderCursor();

    /// @brief Restore pixels from cursor save buffer.
    void RestoreCursor();

    /// @brief Draw the cursor at current position.
    void DrawCursor();

    /// @brief Draw all desktop icons (legacy method).
    void DrawIcons();

    /// @brief Capture a window's bitmap for dragging.
    /// @param win The window to capture.
    void CaptureWindowBitmap(Window* win);

    /// @brief Draw the dragged window bitmap.
    void DrawDragBitmap();

    /// @brief Rebuild the spatial grid from children.
    void UpdateSpatialGrid();

protected:
    /// @brief Update client bounds to exclude taskbar area.
    virtual void UpdateClientBounds() override;

public:
    /// @brief Constructor with optional background color.
    /// @param backgroundColor Desktop background color (default: Cyan).
    Desktop(const Color& backgroundColor = Color::Cyan);

    /// @brief Destructor that cleans up all children and resources.
    virtual ~Desktop();

    /// @brief Get the currently focused window.
    /// @return Pointer to focused window, or nullptr if none.
    Window* FocusedWindow() const
    {
        return _focusedWindow;
    }

    /// @brief Set the focused window.
    /// @param window Window to focus, or nullptr to clear focus.
    /// @note Updates window visual state and taskbar button states.
    void SetFocusedWindow(Window* window);

    /// @brief Get the screen width.
    /// @return Screen width in pixels.
    Int32 ScreenWidth() const
    {
        return Int32(_screenWidth);
    }

    /// @brief Get the screen height.
    /// @return Screen height in pixels.
    Int32 ScreenHeight() const
    {
        return Int32(_screenHeight);
    }

    /// @brief Set a wallpaper image for the desktop.
    /// @param wallpaper Image to use as wallpaper (scaled to fit screen).
    void SetWallpaper(const Image& wallpaper);

    /// @brief Get the number of desktop icons.
    /// @return Number of icons as Int32.
    Int32 IconCount() const
    {
        return Int32(_icons.Length());
    }

    /// @brief Set the mouse cursor image.
    /// @param cursorImage Image to use as cursor (typically 24x24).
    void SetCursor(const Image& cursorImage);

    /// @brief Load cursor from an icon library by index.
    /// @param path Path to icon library file (.icl, .dll, .exe).
    /// @param iconIndex Zero-based index of icon in library.
    void LoadCursorFromLibrary(const char* path, Int32 iconIndex);

    /// @brief Load cursor from an icon library by name.
    /// @param path Path to icon library file (.icl, .dll, .exe).
    /// @param iconName Name of the icon in the library.
    void LoadCursorFromLibrary(const char* path, const char* iconName);

    /// @brief Add an icon to the desktop.
    /// @param icon Image for the desktop icon (typically 32x32).
    /// @note Icons are automatically positioned in a grid.
    void AddIcon(const Image& icon);

    /// @brief Add an icon to the desktop with a text label.
    /// @param icon Image for the desktop icon (typically 32x32).
    /// @param text Text label to display below the icon.
    /// @note Icons are automatically positioned in a grid.
    void AddIcon(const Image& icon, const String& text);

    /// @brief Add a desktop icon from an icon library by index.
    /// @param path Path to icon library file.
    /// @param iconIndex Zero-based index of icon in library.
    void AddIconFromLibrary(const char* path, Int32 iconIndex);

    /// @brief Add a desktop icon from an icon library by index with text label.
    /// @param path Path to icon library file.
    /// @param iconIndex Zero-based index of icon in library.
    /// @param text Text label to display below the icon.
    void AddIconFromLibrary(const char* path, Int32 iconIndex, const String& text);

    /// @brief Add a desktop icon from an icon library by name.
    /// @param path Path to icon library file.
    /// @param iconName Name of the icon in the library.
    void AddIconFromLibrary(const char* path, const char* iconName);

    /// @brief Add a desktop icon from an icon library by name with text label.
    /// @param path Path to icon library file.
    /// @param iconName Name of the icon in the library.
    /// @param text Text label to display below the icon.
    void AddIconFromLibrary(const char* path, const char* iconName, const String& text);

    /// @brief Set the taskbar reference.
    /// @param taskBar Pointer to the TaskBar control.
    /// @note TaskBar must be a child of this Desktop.
    void SetTaskBar(TaskBar* taskBar)
    {
        _taskBar = taskBar;
    }

    /// @brief Get the taskbar reference.
    /// @return Pointer to TaskBar, or nullptr if none.
    TaskBar* GetTaskBar() const
    {
        return _taskBar;
    }

    /// @brief Set the start menu reference.
    /// @param menu Pointer to the StartMenu control.
    void SetStartMenu(StartMenu* menu)
    {
        _startMenu = menu;
    }

    /// @brief Get the start menu reference.
    /// @return Pointer to StartMenu, or nullptr if none.
    StartMenu* GetStartMenu() const
    {
        return _startMenu;
    }

    /// @brief Set the system icon library.
    /// @param library Pointer to an IconLibrary (ownership transferred to Desktop).
    void SetIconLibrary(Drawing::IconLibrary* library);

    /// @brief Get the system icon library.
    /// @return Pointer to the IconLibrary, or nullptr if none loaded.
    Drawing::IconLibrary* GetIconLibrary() const
    {
        return _iconLibrary;
    }

    /// @brief Select a desktop icon.
    /// @param icon Pointer to icon to select, or nullptr to deselect all.
    /// @note Only one icon can be selected at a time.
    void SelectIcon(DesktopIconControl* icon);

    /// @brief Get the currently selected desktop icon.
    /// @return Pointer to selected icon, or nullptr if none.
    DesktopIconControl* SelectedIcon() const
    {
        return _selectedIcon;
    }

    /// @brief Add a child control to the desktop.
    /// @param child Control to add.
    /// @note Overridden to update spatial grid and taskbar.
    virtual void AddChild(Control* child) override;

    /// @brief Remove a child control from the desktop.
    /// @param child Control to remove.
    /// @note Overridden to update spatial grid and taskbar.
    virtual void RemoveChild(Control* child) override;

    /// @brief Paint the desktop background and icons.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Handle keyboard events.
    /// @param e Keyboard event arguments.
    /// @note Escape key stops the event loop.
    virtual void OnKeyboard(KeyboardEventArgs& e) override;

    /// @brief Handle mouse events with focus management.
    /// @param e Mouse event arguments.
    /// @note Handles window focus changes and drag operations.
    void HandleMouse(MouseEventArgs& e);

    /// @brief Start the main event loop.
    /// @note Blocks until Stop() is called or Escape is pressed.
    void Run();

    /// @brief Stop the main event loop.
    void Stop();

    /// @brief Get the runtime type of this control.
    /// @return ControlType::Desktop.
    virtual ControlType GetControlType() const override
    {
        return ControlType::Desktop;
    }

    /// @brief Safe downcast to Desktop.
    /// @return Pointer to this Desktop.
    virtual Desktop* AsDesktop() override
    {
        return this;
    }

    /// @brief Safe downcast to Desktop (const version).
    /// @return Const pointer to this Desktop.
    virtual const Desktop* AsDesktop() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Window                                          */
/******************************************************************************/

/// @brief Top-level window with title bar, frame, and client area.
/// @details Provides a Windows 95-style window with:
/// - Title bar with caption text
/// - 3D beveled frame
/// - Draggable by title bar
/// - Focus highlighting (blue title bar when focused)
///
/// Windows automatically register with the TaskBar when added to a Desktop.
/// The client area excludes the title bar and frame borders.
class Window : public Control
{
    static const int TITLE_BAR_HEIGHT = 20;  ///< Title bar height in pixels
    static const int FRAME_WIDTH = 3;        ///< Frame border width in pixels
    static const int BUTTON_SIZE = 16;       ///< Title bar button size (square)
    static const int BUTTON_SPACING = 2;     ///< Space between buttons

    bool _isFocused;                         ///< True if window has focus
    String _title;                           ///< Window title text
    Font _font;                              ///< Font for title rendering
    Color _backColor;                        ///< Client area background color
    bool _isMaximized;                       ///< True if window is maximized
    bool _isMinimized;                       ///< True if window is minimized
    Rectangle _restoreBounds;               ///< Bounds to restore to from maximized
    BorderStyle _borderStyle;               ///< Current border style
    Image _closeIcon;                        ///< Close button icon (16x16)
    Image _maximizeIcon;                     ///< Maximize button icon (16x16)
    Image _minimizeIcon;                     ///< Minimize button icon (16x16)
    Image _restoreIcon;                      ///< Restore button icon (16x16)

    /// @brief Get the close button rectangle (in screen coordinates).
    Rectangle GetCloseButtonRect() const;

    /// @brief Get the maximize button rectangle (in screen coordinates).
    Rectangle GetMaximizeButtonRect() const;

    /// @brief Get the minimize button rectangle (in screen coordinates).
    Rectangle GetMinimizeButtonRect() const;

protected:
    /// @brief Update client bounds to exclude title bar and frame.
    virtual void UpdateClientBounds() override;

public:
    /// @brief Constructor with parent and bounds.
    /// @param parent Parent control (typically Desktop).
    /// @param bounds Initial window bounds.
    /// @note By default, Windows don't participate in layout (floating).
    Window(Control* parent, const Rectangle& bounds);

    /// @brief Destructor.
    virtual ~Window();

    /// @brief Get the window title.
    /// @return Const reference to title string.
    const String& Title() const
    {
        return _title;
    }

    /// @brief Set the window title.
    /// @param title New title string.
    void SetTitle(const String& title)
    {
        _title = title;
        Invalidate();
    }

    /// @brief Set the window title from C string.
    /// @param title New title as null-terminated C string.
    void SetTitle(const char* title)
    {
        _title = String(title);
        Invalidate();
    }

    /// @brief Get the title bar font.
    /// @return Const reference to the Font.
    const Font& GetFont() const
    {
        return _font;
    }

    /// @brief Set the title bar font.
    /// @param font Font for title rendering.
    void SetFont(const Font& font)
    {
        _font = font;
        Invalidate();
    }

    /// @brief Get the client area background color.
    /// @return The background color.
    Color BackColor() const
    {
        return _backColor;
    }

    /// @brief Set the client area background color.
    /// @param color New background color.
    void SetBackColor(const Color& color)
    {
        _backColor = color;
        Invalidate();
    }

    /// @brief Check if window has focus.
    /// @return True if this window is focused.
    Boolean IsFocused() const
    {
        return Boolean(_isFocused);
    }

    /// @brief Set window focus state.
    /// @param focused New focus state.
    /// @note Typically managed by Desktop; call Desktop::SetFocusedWindow instead.
    void SetFocused(Boolean focused)
    {
        _isFocused = static_cast<bool>(focused);
    }

    /// @brief Check if window is maximized.
    /// @return True if maximized.
    Boolean IsMaximized() const
    {
        return Boolean(_isMaximized);
    }

    /// @brief Check if window is minimized.
    /// @return True if minimized.
    Boolean IsMinimized() const
    {
        return Boolean(_isMinimized);
    }

    /// @brief Minimize the window (hide it).
    void Minimize();

    /// @brief Maximize the window to fill the screen.
    void Maximize();

    /// @brief Restore the window to its previous size.
    void Restore();

    /// @brief Close the window (remove from parent).
    void Close();

    /// @brief Load button icons from the desktop's icon library.
    /// @note Call after window is added to a Desktop with an IconLibrary.
    void LoadButtonIcons();

    /// @brief Paint the window frame and title bar.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Handle mouse events in client area.
    /// @param e Mouse event arguments.
    virtual void OnMouse(MouseEventArgs& e) override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::Window.
    virtual ControlType GetControlType() const override
    {
        return ControlType::Window;
    }

    /// @brief Safe downcast to Window.
    /// @return Pointer to this Window.
    virtual Window* AsWindow() override
    {
        return this;
    }

    /// @brief Safe downcast to Window (const version).
    /// @return Const pointer to this Window.
    virtual const Window* AsWindow() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::TaskBar                                         */
/******************************************************************************/

/// @brief Windows 95-style taskbar at the bottom of the screen.
/// @details The TaskBar displays:
/// - Start button (opens StartMenu)
/// - Window buttons for each open window
/// - Active window button shows pressed state
///
/// Window buttons are automatically added and removed as windows are
/// created and destroyed. The TaskBar communicates with the Desktop
/// to manage window focus.
class TaskBar : public Control
{
    static const int WINDOW_BUTTON_WIDTH = 120;    ///< Width of window buttons
    static const int WINDOW_BUTTON_HEIGHT = 24;    ///< Height of window buttons
    static const int WINDOW_BUTTON_SPACING = 2;    ///< Space between buttons
    static const int WINDOW_BUTTON_START_X = 75;   ///< X offset after Start button (65 + margin)

    Button* _startButton;                ///< The Start button
    StartMenu* _startMenu;               ///< Reference to start menu (not owned)
    Desktop* _desktop;                   ///< Back-reference to desktop
    Array<TaskBarButton*> _windowButtons; ///< Buttons for open windows
    TaskTray* _taskTray;                 ///< System tray on right side

    /// @brief Start button click handler.
    /// @param sender The button that was clicked.
    /// @param userData User context (TaskBar pointer).
    static void OnStartButtonClick(Button* sender, void* userData);

public:
    /// @brief Constructor with parent and optional start menu.
    /// @param parent Parent control (typically Desktop).
    /// @param startMenu Optional StartMenu to toggle on Start click.
    TaskBar(Control* parent, StartMenu* startMenu = nullptr);

    /// @brief Destructor.
    virtual ~TaskBar();

    /// @brief Paint the taskbar background.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Get the Start button.
    /// @return Pointer to the Start button control.
    Button* StartButton() const
    {
        return _startButton;
    }

    /// @brief Set the desktop back-reference.
    /// @param desktop Pointer to the Desktop.
    /// @note Called by Desktop::SetTaskBar.
    void SetDesktop(Desktop* desktop)
    {
        _desktop = desktop;
    }

    /// @brief Load icons for Start button and system tray.
    /// @note Call this after SetDesktop() and after Desktop has an IconLibrary.
    void LoadIcons();

    /// @brief Get the desktop back-reference.
    /// @return Pointer to the Desktop.
    Desktop* GetDesktop() const
    {
        return _desktop;
    }

    /// @brief Add a window button for an open window.
    /// @param window The window to create a button for.
    /// @note Button text is set to window title.
    void AddWindowButton(Window* window);

    /// @brief Remove a window button.
    /// @param window The window whose button to remove.
    void RemoveWindowButton(Window* window);

    /// @brief Update button pressed states based on window focus.
    /// @note Called when focused window changes.
    void RefreshWindowButtons();

    /// @brief Find the button for a specific window.
    /// @param window The window to find.
    /// @return Pointer to TaskBarButton, or nullptr if not found.
    TaskBarButton* FindButtonForWindow(Window* window) const;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::TaskBar.
    virtual ControlType GetControlType() const override
    {
        return ControlType::TaskBar;
    }

    /// @brief Safe downcast to TaskBar.
    /// @return Pointer to this TaskBar.
    virtual TaskBar* AsTaskBar() override
    {
        return this;
    }

    /// @brief Safe downcast to TaskBar (const version).
    /// @return Const pointer to this TaskBar.
    virtual const TaskBar* AsTaskBar() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::TaskTray                                        */
/******************************************************************************/

/// @brief System tray (notification area) on the right side of the taskbar.
/// @details Contains small icons representing system status and notifications.
/// Drawn with a sunken border and sizes automatically based on icon count.
class TaskTray : public Control
{
    static const int ICON_SIZE = 16;        ///< Icon size (16x16)
    static const int ICON_SPACING = 2;      ///< Space between icons
    static const int PADDING = 4;           ///< Padding inside the tray

    Array<Image> _icons;                    ///< Icons displayed in the tray

public:
    /// @brief Constructor with parent.
    /// @param parent Parent control (typically TaskBar).
    TaskTray(Control* parent);

    /// @brief Destructor.
    virtual ~TaskTray() = default;

    /// @brief Add an icon to the tray.
    /// @param icon The icon image to add (should be 16x16).
    void AddIcon(const Image& icon);

    /// @brief Get the number of icons in the tray.
    /// @return Number of icons.
    Int32 IconCount() const
    {
        return Int32(_icons.Length());
    }

    /// @brief Calculate the preferred width based on icon count.
    /// @return Width in pixels.
    Int32 CalculateWidth() const
    {
        int count = _icons.Length();
        if (count == 0) return Int32(0);
        return Int32(PADDING * 2 + count * ICON_SIZE + (count - 1) * ICON_SPACING);
    }

    /// @brief Paint the tray with sunken border and icons.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Get preferred size for layout.
    /// @return MeasureResult with calculated width and parent height.
    virtual MeasureResult GetPreferredSize() const override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::TaskTray.
    virtual ControlType GetControlType() const override
    {
        return ControlType::TaskTray;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Button                                          */
/******************************************************************************/

/// @brief Clickable button control with 3D beveled appearance.
/// @details Displays text centered on a raised button surface.
/// When pressed (clicked or toggled), shows a sunken appearance.
///
/// Button visual state combines:
/// - Toggle state: Persistent pressed state (e.g., active window)
/// - Mouse state: Temporary state during mouse click
///
/// Use SetOnClick to register a click event handler.
class Button : public Control
{
    bool _isToggled;              ///< Persistent pressed state
    bool _isMouseDown;            ///< Temporary state during click
    bool _wasMouseDown;           ///< Previous mouse state for click detection
    ClickEventHandler _onClick;   ///< Click event handler function
    void* _onClickUserData;       ///< User data passed to click handler
    String _text;                 ///< Button label text
    Font _font;                   ///< Font for text rendering
    Image _icon;                  ///< Optional icon displayed before text

public:
    /// @brief Constructor with parent and bounds.
    /// @param parent Parent control.
    /// @param bounds Button bounds within parent.
    Button(Control* parent, const Rectangle& bounds);

    /// @brief Destructor.
    virtual ~Button();

    /// @brief Get the button text.
    /// @return Const reference to text string.
    const String& Text() const
    {
        return _text;
    }

    /// @brief Set the button text.
    /// @param text New button text.
    void SetText(const String& text)
    {
        _text = text;
        Invalidate();
    }

    /// @brief Set the button text from C string.
    /// @param text New text as null-terminated C string.
    void SetText(const char* text)
    {
        _text = String(text);
        Invalidate();
    }

    /// @brief Get the button font.
    /// @return Const reference to the Font.
    const Font& GetFont() const
    {
        return _font;
    }

    /// @brief Set the button font.
    /// @param font Font for text rendering.
    void SetFont(const Font& font)
    {
        _font = font;
        Invalidate();
    }

    /// @brief Set the button icon (displayed before text).
    /// @param icon Icon image to display.
    void SetIcon(const Image& icon)
    {
        _icon = icon;
        Invalidate();
    }

    /// @brief Get the button icon.
    /// @return Const reference to the icon image.
    const Image& GetIcon() const
    {
        return _icon;
    }

    /// @brief Check if button appears pressed.
    /// @return True if toggled OR mouse is down.
    /// @note Visual state combines toggle and mouse states.
    Boolean IsPressed() const
    {
        return Boolean(_isToggled || _isMouseDown);
    }

    /// @brief Set the persistent pressed/toggled state.
    /// @param pressed New toggle state.
    /// @note Used by TaskBarButton to show active window.
    void SetPressed(Boolean pressed)
    {
        _isToggled = static_cast<bool>(pressed);
        Invalidate();
    }

    /// @brief Register a click event handler.
    /// @param handler Function to call when button is clicked.
    /// @param userData Optional context data passed to handler.
    void SetOnClick(ClickEventHandler handler, void* userData = nullptr);

    /// @brief Paint the button.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Handle mouse events for click detection.
    /// @param e Mouse event arguments.
    virtual void OnMouse(MouseEventArgs& e) override;

    /// @brief Get preferred size for layout.
    /// @return MeasureResult with current bounds as preferred size.
    virtual MeasureResult GetPreferredSize() const override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::Button.
    virtual ControlType GetControlType() const override
    {
        return ControlType::Button;
    }

    /// @brief Safe downcast to Button.
    /// @return Pointer to this Button.
    virtual Button* AsButton() override
    {
        return this;
    }

    /// @brief Safe downcast to Button (const version).
    /// @return Const pointer to this Button.
    virtual const Button* AsButton() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::Picture                                         */
/******************************************************************************/

/// @brief Control that displays an image.
/// @details Renders an Image at the control's position. The image is
/// drawn at its natural size; no scaling is performed.
class Picture : public Control
{
    Image _image;  ///< The image to display

public:
    /// @brief Constructor with parent and bounds.
    /// @param parent Parent control.
    /// @param bounds Picture bounds within parent.
    Picture(Control* parent, const Rectangle& bounds);

    /// @brief Constructor with parent, bounds, and initial image.
    /// @param parent Parent control.
    /// @param bounds Picture bounds within parent.
    /// @param image Initial image to display.
    Picture(Control* parent, const Rectangle& bounds, const Image& image);

    /// @brief Destructor.
    virtual ~Picture();

    /// @brief Set the displayed image.
    /// @param image New image to display.
    void SetImage(const Image& image);

    /// @brief Get the displayed image.
    /// @return Const reference to the image.
    const Image& GetImage() const
    {
        return _image;
    }

    /// @brief Paint the picture.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Get preferred size based on image dimensions.
    /// @return MeasureResult with image width and height.
    virtual MeasureResult GetPreferredSize() const override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::Picture.
    virtual ControlType GetControlType() const override
    {
        return ControlType::Picture;
    }

    /// @brief Safe downcast to Picture.
    /// @return Pointer to this Picture.
    virtual Picture* AsPicture() override
    {
        return this;
    }

    /// @brief Safe downcast to Picture (const version).
    /// @return Const pointer to this Picture.
    virtual const Picture* AsPicture() const override
    {
        return this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::SpectrumControl                                 */
/******************************************************************************/

/// @brief Control displaying a vertical color gradient.
/// @details Renders a smooth gradient transitioning:
/// - Top: 100% white
/// - Middle: Base color
/// - Bottom: 100% black
///
/// This control demonstrates 32-bit color rendering with smooth gradients.
/// The gradient is pre-computed and cached as an Image for efficient drawing.
///
/// Useful for color picker interfaces or visual demonstrations of
/// the 32-bit graphics pipeline.
class SpectrumControl : public Control
{
    Color _baseColor;     ///< The middle color of the gradient
    Image _gradient;      ///< Cached pre-computed gradient image

    /// @brief Regenerate the gradient image.
    /// @note Called when base color or size changes.
    void RegenerateGradient();

public:
    /// @brief Constructor with parent, bounds, and base color.
    /// @param parent Parent control.
    /// @param bounds Control bounds within parent.
    /// @param baseColor The middle color of the gradient (default: Red).
    SpectrumControl(Control* parent, const Rectangle& bounds,
                    const Color& baseColor = Color::Red);

    /// @brief Destructor.
    virtual ~SpectrumControl();

    /// @brief Get the base color.
    /// @return The middle gradient color.
    Color GetBaseColor() const
    {
        return _baseColor;
    }

    /// @brief Set the base color.
    /// @param color New base color for the gradient.
    /// @note Triggers gradient regeneration.
    void SetBaseColor(const Color& color);

    /// @brief Get interpolated color at a Y position.
    /// @param y Y position (0 = top/white, height-1 = bottom/black).
    /// @return Interpolated color at that position.
    Color GetColorAtY(Int32 y) const;

    /// @brief Paint the spectrum gradient.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::Spectrum.
    virtual ControlType GetControlType() const override
    {
        return ControlType::Spectrum;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::TaskBarButton                                   */
/******************************************************************************/

/// @brief Specialized button representing an open window in the taskbar.
/// @details Shows as pressed/sunken when the associated window is focused.
/// When pressed, displays a checkerboard hatch pattern (Windows 95 style).
///
/// Clicking the button focuses the associated window via the Desktop.
class TaskBarButton : public Button
{
    Window* _window;  ///< Reference to associated window (not owned)

    /// @brief Click handler that focuses the associated window.
    /// @param sender The button that was clicked.
    /// @param userData User context (TaskBarButton pointer).
    static void OnTaskBarButtonClick(Button* sender, void* userData);

public:
    /// @brief Constructor with parent, bounds, and associated window.
    /// @param parent Parent control (typically TaskBar).
    /// @param bounds Button bounds within parent.
    /// @param window The window this button represents.
    TaskBarButton(Control* parent, const Rectangle& bounds, Window* window);

    /// @brief Destructor.
    virtual ~TaskBarButton();

    /// @brief Get the associated window.
    /// @return Pointer to the window this button represents.
    Window* GetWindow() const
    {
        return _window;
    }

    /// @brief Paint with hatch pattern when pressed.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::TaskBarButton.
    virtual ControlType GetControlType() const override
    {
        return ControlType::TaskBarButton;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MenuItem                                        */
/******************************************************************************/

/// @brief Individual menu item with icon and text.
/// @details Displays an icon on the left and menu text.
/// Highlights with blue background on mouse hover.
/// Fires click event when clicked.
class MenuItem : public Control
{
    static const int ICON_SIZE = 32;      ///< Icon display size (32x32)
    static const int ITEM_HEIGHT = 40;    ///< Item height in pixels
    static const int ICON_MARGIN = 4;     ///< Margin around icon
    static const int TEXT_MARGIN = 8;     ///< Margin between icon and text
    static const int SEPARATOR_HEIGHT = 8; ///< Height of separator items

    Image _icon;                   ///< Menu item icon
    String _text;                  ///< Menu item text
    bool _isHighlighted;           ///< True if mouse is hovering
    bool _isSeparator;             ///< True if this is a separator line
    bool _wasPressed;              ///< Previous press state for click detection
    ClickEventHandler _onClick;    ///< Click event handler
    void* _onClickUserData;        ///< User data for click handler
    int _itemIndex;                ///< Index in parent menu

public:
    /// @brief Constructor with parent, bounds, and item index.
    /// @param parent Parent control (typically StartMenu).
    /// @param bounds Item bounds within parent.
    /// @param itemIndex Index of this item in the menu.
    MenuItem(Control* parent, const Rectangle& bounds, int itemIndex);

    /// @brief Destructor.
    virtual ~MenuItem();

    /// @brief Set the menu item icon.
    /// @param icon Icon image (typically 16x16).
    void SetIcon(const Image& icon);

    /// @brief Set the menu item text.
    /// @param text Text to display.
    void SetText(const String& text);

    /// @brief Set whether this is a separator.
    /// @param isSeparator True to make this a separator line.
    void SetSeparator(Boolean isSeparator);

    /// @brief Check if this is a separator.
    /// @return True if this is a separator.
    Boolean IsSeparator() const
    {
        return Boolean(_isSeparator);
    }

    /// @brief Register a click event handler.
    /// @param handler Function to call when item is clicked.
    /// @param userData Optional context data passed to handler.
    void SetOnClick(ClickEventHandler handler, void* userData = nullptr);

    /// @brief Paint the menu item.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Update mouse state and return true if highlight changed.
    /// @param e Mouse event arguments.
    /// @return True if highlight state changed (needs repaint).
    /// @details Used by StartMenu to batch repaints for better performance.
    bool HandleMouseUpdate(MouseEventArgs& e);

    /// @brief Handle mouse events for hover and click.
    /// @param e Mouse event arguments.
    virtual void OnMouse(MouseEventArgs& e) override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::MenuItem.
    virtual ControlType GetControlType() const override
    {
        return ControlType::MenuItem;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::StartMenu                                       */
/******************************************************************************/

/// @brief Windows 95-style Start menu popup.
/// @details Displays above the Start button when opened:
/// - Blue sidebar on the left with "Windows 95" text (rotated)
/// - 12 menu items with icons loaded from sysicons.icl
/// - Highlights items on hover
///
/// The menu is shown/hidden by the Start button or by clicking
/// outside the menu area.
class StartMenu : public Control
{
    static const int MENU_WIDTH = 200;       ///< Total menu width
    static const int SIDEBAR_WIDTH = 24;     ///< Blue sidebar width
    static const int ITEM_COUNT = 11;        ///< Number of menu items
    static const int ITEM_HEIGHT = 40;       ///< Height of each item (for 32x32 icons)
    static const int SEPARATOR_HEIGHT = 8;   ///< Height of separator items

    Desktop* _desktop;              ///< Back-reference to desktop (not owned)
    bool _isVisible;                ///< True if menu is currently shown
    Array<MenuItem*> _items;        ///< Menu item controls

    /// @brief Load icons for menu items from icon library.
    void LoadIcons();

public:
    /// @brief Constructor with desktop reference.
    /// @param desktop The Desktop this menu belongs to.
    StartMenu(Desktop* desktop);

    /// @brief Destructor.
    virtual ~StartMenu();

    /// @brief Check if menu is currently visible.
    /// @return True if the menu is shown.
    Boolean IsVisible() const
    {
        return Boolean(_isVisible);
    }

    /// @brief Show the start menu.
    void Show();

    /// @brief Hide the start menu.
    void Hide();

    /// @brief Toggle menu visibility.
    void Toggle();

    /// @brief Paint the menu background and sidebar.
    /// @param e Paint event arguments.
    virtual void OnPaint(PaintEventArgs& e) override;

    /// @brief Handle mouse events.
    /// @param e Mouse event arguments.
    virtual void OnMouse(MouseEventArgs& e) override;

    /// @brief Get the runtime type of this control.
    /// @return ControlType::StartMenu.
    virtual ControlType GetControlType() const override
    {
        return ControlType::StartMenu;
    }
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_HPP
