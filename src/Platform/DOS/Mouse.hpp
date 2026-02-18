#ifndef PLATFORM_DOS_MOUSE_HPP
#define PLATFORM_DOS_MOUSE_HPP

namespace Platform::DOS
{

struct MouseState
{
    int x;
    int y;
    bool leftButton;
    bool rightButton;
    bool middleButton;
};

class Mouse
{
public:
    // Initialize mouse driver, returns true if mouse is available
    static bool Initialize();

    // Show/hide hardware mouse cursor
    static void ShowCursor();
    static void HideCursor();

    // Get mouse position and button state
    static MouseState GetState();

    // Set mouse cursor position
    static void SetPosition(int x, int y);

    // Set mouse movement bounds
    static void SetHorizontalBounds(int min, int max);
    static void SetVerticalBounds(int min, int max);

    // Set mouse sensitivity (mickeys per 8 pixels)
    // Higher values = slower mouse. Default is horizontal=8, vertical=16
    static void SetSensitivity(int horizontalMickeys, int verticalMickeys);
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_MOUSE_HPP
