#ifndef PLATFORM_DOS_MOUSE_HPP
#define PLATFORM_DOS_MOUSE_HPP

namespace Platform::DOS
{

/// @brief Represents the current state of the mouse device.
///
/// This structure captures the mouse cursor position and the pressed state
/// of all three mouse buttons at a single point in time.
struct MouseState
{
    /// @brief The x-coordinate of the mouse cursor in screen pixels.
    int x;

    /// @brief The y-coordinate of the mouse cursor in screen pixels.
    int y;

    /// @brief True if the left mouse button is currently pressed.
    bool leftButton;

    /// @brief True if the right mouse button is currently pressed.
    bool rightButton;

    /// @brief True if the middle mouse button is currently pressed.
    bool middleButton;
};

/// @brief Provides static methods for interacting with the DOS mouse driver.
///
/// The Mouse class is a facade over the INT 33h DOS mouse driver services.
/// All methods are static since there is only one mouse in the system.
/// The mouse must be initialized before use by calling Initialize().
///
/// @note This class uses the DOS mouse driver (typically loaded via CTMOUSE
/// or similar). The driver must be loaded before Initialize() is called.
class Mouse
{
public:
    /// @brief Initializes the mouse driver and resets the mouse state.
    ///
    /// This function must be called before using any other Mouse methods.
    /// It resets the mouse driver to its default state, including position,
    /// bounds, and sensitivity settings.
    ///
    /// @return True if a mouse driver is present and initialized successfully,
    ///         false if no mouse driver is available.
    static bool Initialize();

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
    /// @return A MouseState structure containing the current x/y position
    ///         and button states.
    static MouseState GetState();

    /// @brief Sets the mouse cursor position.
    ///
    /// Moves the mouse cursor to the specified screen coordinates. The
    /// position is clamped to the current horizontal and vertical bounds.
    ///
    /// @param x The new x-coordinate in screen pixels.
    /// @param y The new y-coordinate in screen pixels.
    static void SetPosition(int x, int y);

    /// @brief Sets the horizontal movement bounds for the mouse cursor.
    ///
    /// Restricts the mouse cursor's horizontal movement to the specified
    /// range. The cursor cannot be moved outside these bounds by either
    /// user input or SetPosition().
    ///
    /// @param min The minimum x-coordinate (left edge).
    /// @param max The maximum x-coordinate (right edge).
    static void SetHorizontalBounds(int min, int max);

    /// @brief Sets the vertical movement bounds for the mouse cursor.
    ///
    /// Restricts the mouse cursor's vertical movement to the specified
    /// range. The cursor cannot be moved outside these bounds by either
    /// user input or SetPosition().
    ///
    /// @param min The minimum y-coordinate (top edge).
    /// @param max The maximum y-coordinate (bottom edge).
    static void SetVerticalBounds(int min, int max);

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
    static void SetSensitivity(int horizontalMickeys, int verticalMickeys);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_MOUSE_HPP
