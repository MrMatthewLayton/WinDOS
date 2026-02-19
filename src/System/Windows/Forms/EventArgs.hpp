#ifndef SYSTEM_WINDOWS_FORMS_EVENTARGS_HPP
#define SYSTEM_WINDOWS_FORMS_EVENTARGS_HPP

#include "System/Types.hpp"
#include "../../Drawing/Drawing.hpp"

/******************************************************************************/
/*    System::Windows::Forms Event Arguments                                  */
/*                                                                            */
/*    Event argument classes for paint, mouse, and keyboard events.           */
/******************************************************************************/

namespace System::Windows::Forms
{

// Import types from Drawing namespace
using System::Drawing::Graphics;
using System::Drawing::Rectangle;

/******************************************************************************/
/*    System::Windows::Forms::PaintEventArgs                                  */
/******************************************************************************/

/// @brief Event arguments passed to paint event handlers.
/// @details Contains the Graphics context for drawing and the bounds
/// that need to be repainted. Also includes clip bounds for clipping
/// child controls to parent boundaries. Similar to .NET PaintEventArgs.
class PaintEventArgs
{
public:
    Graphics* graphics;     ///< Graphics context for drawing operations
    Rectangle bounds;       ///< Bounds of the area to be painted
    Rectangle clipBounds;   ///< Clip region in screen coordinates (drawing clipped to this area)

    /// @brief Default constructor.
    PaintEventArgs() : graphics(nullptr), bounds(), clipBounds()
    {
    }

    /// @brief Constructor with graphics context and bounds.
    /// @param g Pointer to the Graphics context.
    /// @param b Rectangle defining the paint bounds.
    /// @note Clip bounds defaults to the same as bounds.
    PaintEventArgs(Graphics* g, const Rectangle& b)
        : graphics(g), bounds(b), clipBounds(b)
    {
    }

    /// @brief Constructor with graphics context, bounds, and clip region.
    /// @param g Pointer to the Graphics context.
    /// @param b Rectangle defining the paint bounds.
    /// @param clip Rectangle defining the clip region in screen coordinates.
    PaintEventArgs(Graphics* g, const Rectangle& b, const Rectangle& clip)
        : graphics(g), bounds(b), clipBounds(clip)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MouseEventArgs                                  */
/******************************************************************************/

/// @brief Event arguments for mouse events.
/// @details Contains mouse position and button states at the time of
/// the event. Coordinates are in screen space.
class MouseEventArgs
{
public:
    Int32 x;                ///< X coordinate in screen space
    Int32 y;                ///< Y coordinate in screen space
    Boolean leftButton;     ///< True if left button is pressed
    Boolean rightButton;    ///< True if right button is pressed

    /// @brief Default constructor.
    MouseEventArgs() : x(0), y(0), leftButton(false), rightButton(false)
    {
    }

    /// @brief Constructor with position and button states.
    /// @param x X coordinate in screen space.
    /// @param y Y coordinate in screen space.
    /// @param left State of left mouse button.
    /// @param right State of right mouse button.
    MouseEventArgs(Int32 x, Int32 y, Boolean left, Boolean right)
        : x(x), y(y), leftButton(left), rightButton(right)
    {
    }
};

/******************************************************************************/
/*    System::Windows::Forms::KeyboardEventArgs                               */
/******************************************************************************/

/// @brief Event arguments for keyboard events.
/// @details Contains the key character and modifier key states.
class KeyboardEventArgs
{
public:
    Char key;               ///< The key character pressed
    Boolean alt;            ///< True if Alt key is held
    Boolean ctrl;           ///< True if Ctrl key is held
    Boolean shift;          ///< True if Shift key is held

    /// @brief Default constructor.
    KeyboardEventArgs() : key('\0'), alt(false), ctrl(false), shift(false)
    {
    }

    /// @brief Constructor with key and modifier states.
    /// @param k The key character.
    /// @param a Alt key state.
    /// @param c Ctrl key state.
    /// @param s Shift key state.
    KeyboardEventArgs(Char k, Boolean a, Boolean c, Boolean s)
        : key(k), alt(a), ctrl(c), shift(s)
    {
    }
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_EVENTARGS_HPP
