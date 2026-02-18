#ifndef SYSTEM_IO_DEVICES_MOUSE_HPP
#define SYSTEM_IO_DEVICES_MOUSE_HPP

#include "../../Types.hpp"

namespace System::IO::Devices
{

/******************************************************************************/
/*    MouseStatus                                                             */
/******************************************************************************/

/// @brief Represents the current state of the mouse device.
///
/// This class captures the mouse cursor position and the pressed state
/// of all three mouse buttons at a single point in time.
class MouseStatus
{
public:
    /// @brief The x-coordinate of the mouse cursor in screen pixels.
    Int32 x;

    /// @brief The y-coordinate of the mouse cursor in screen pixels.
    Int32 y;

    /// @brief True if the left mouse button is currently pressed.
    Boolean leftButton;

    /// @brief True if the right mouse button is currently pressed.
    Boolean rightButton;

    /// @brief True if the middle mouse button is currently pressed.
    Boolean middleButton;

    /// @brief Constructs a default MouseStatus with position (0,0) and no buttons pressed.
    MouseStatus()
        : x(0), y(0), leftButton(false), rightButton(false), middleButton(false)
    {
    }

    /// @brief Constructs a MouseStatus with the specified values.
    /// @param x X coordinate of the cursor
    /// @param y Y coordinate of the cursor
    /// @param left True if left button is pressed
    /// @param right True if right button is pressed
    /// @param middle True if middle button is pressed
    MouseStatus(Int32 x, Int32 y, Boolean left, Boolean right, Boolean middle)
        : x(x), y(y), leftButton(left), rightButton(right), middleButton(middle)
    {
    }
};

/******************************************************************************/
/*    Mouse                                                                   */
/******************************************************************************/

/// @brief Provides mouse input functionality using the DOS mouse driver (INT 33h).
///
/// The Mouse class is a static facade for DOS mouse driver operations.
/// All methods are static since there is only one mouse in the system.
/// The mouse must be initialized before use by calling Initialize().
///
/// @note This class uses the DOS mouse driver (typically loaded via CTMOUSE
/// or similar). The driver must be loaded before Initialize() is called.
class Mouse
{
    Mouse() = delete;  // Static class

    static Boolean _initialized;
    static Boolean _available;

    // Low-level BIOS calls - kept for SetBounds() use
    static void BiosSetHorizontalBounds(int min, int max);
    static void BiosSetVerticalBounds(int min, int max);

public:
    /// @brief Initializes the mouse driver and resets the mouse state.
    ///
    /// This function must be called before using any other Mouse methods.
    /// It resets the mouse driver to its default state, including position,
    /// bounds, and sensitivity settings.
    ///
    /// @return True if a mouse driver is present and initialized successfully,
    ///         false if no mouse driver is available.
    static Boolean Initialize();

    /// @brief Checks if a mouse is available and initialized.
    /// @return True if mouse is ready for use
    static Boolean IsAvailable();

    /// @brief Shows the hardware mouse cursor.
    ///
    /// Makes the hardware mouse cursor visible on screen. The cursor
    /// maintains an internal show/hide counter, so each call to ShowCursor()
    /// should be balanced with a call to HideCursor().
    ///
    /// @note When using a custom software cursor (as in the Forms system),
    ///       the hardware cursor should typically remain hidden.
    static void ShowCursor();

    /// @brief Hides the hardware mouse cursor.
    ///
    /// Makes the hardware mouse cursor invisible. The cursor maintains an
    /// internal show/hide counter, so multiple calls to HideCursor() require
    /// the same number of ShowCursor() calls to make it visible again.
    static void HideCursor();

    /// @brief Gets the current mouse position and button state.
    ///
    /// Queries the mouse driver for the current cursor position and the
    /// pressed state of all three mouse buttons.
    ///
    /// @return A MouseStatus structure containing the current x/y position
    ///         and button states.
    static MouseStatus GetStatus();

    /// @brief Gets the mouse X coordinate.
    /// @return Current X position of the cursor
    static Int32 GetX();

    /// @brief Gets the mouse Y coordinate.
    /// @return Current Y position of the cursor
    static Int32 GetY();

    /// @brief Sets the mouse cursor position.
    ///
    /// Moves the mouse cursor to the specified screen coordinates. The
    /// position is clamped to the current horizontal and vertical bounds.
    ///
    /// @param x The new x-coordinate in screen pixels.
    /// @param y The new y-coordinate in screen pixels.
    static void SetPosition(Int32 x, Int32 y);

    /// @brief Sets the mouse movement bounds.
    /// @param minX Minimum X coordinate (left edge)
    /// @param minY Minimum Y coordinate (top edge)
    /// @param maxX Maximum X coordinate (right edge)
    /// @param maxY Maximum Y coordinate (bottom edge)
    ///
    /// Constrains the cursor to the specified rectangular area.
    /// The cursor cannot be moved outside these bounds by either
    /// user input or SetPosition().
    static void SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY);

    /// @brief Sets the mouse sensitivity (movement speed).
    ///
    /// Configures how many mickeys (mouse movement units) are required to
    /// move the cursor 8 pixels. Higher values result in slower cursor
    /// movement, requiring more physical mouse movement for the same
    /// on-screen distance.
    ///
    /// @param horizontalMickeys Mickeys per 8 horizontal pixels. Default is 8.
    /// @param verticalMickeys Mickeys per 8 vertical pixels. Default is 16.
    ///
    /// @note The default vertical value is higher than horizontal to account
    ///       for typical screen aspect ratios, making diagonal movement feel
    ///       more natural.
    static void SetSensitivity(Int32 horizontalMickeys, Int32 verticalMickeys);

    /// @brief Checks if the left mouse button is pressed.
    /// @return True if left button is currently down
    static Boolean IsLeftButtonPressed();

    /// @brief Checks if the right mouse button is pressed.
    /// @return True if right button is currently down
    static Boolean IsRightButtonPressed();
};

} // namespace System::IO::Devices

#endif // SYSTEM_IO_DEVICES_MOUSE_HPP
