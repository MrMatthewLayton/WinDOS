#ifndef SYSTEM_WINDOWS_FORMS_LAYOUTENUMS_HPP
#define SYSTEM_WINDOWS_FORMS_LAYOUTENUMS_HPP

/******************************************************************************/
/*    System::Windows::Forms Layout Enumerations                              */
/*                                                                            */
/*    Enumerations for the flexbox-like layout system.                        */
/******************************************************************************/

namespace System::Windows::Forms
{

/// @brief Direction for laying out child controls in a flex container.
/// @details Similar to CSS flex-direction property. Controls how children
/// are arranged within their parent container.
enum class FlexDirection : unsigned char
{
    Row,            ///< Horizontal layout (left to right)
    Column          ///< Vertical layout (top to bottom)
};

/// @brief Alignment of children along the main axis.
/// @details Similar to CSS justify-content property. Controls how extra
/// space is distributed among children along the primary layout axis.
enum class JustifyContent : unsigned char
{
    Start,          ///< Pack items at start of main axis
    Center,         ///< Center items along main axis
    End,            ///< Pack items at end of main axis
    SpaceBetween,   ///< Evenly distribute; first at start, last at end
    SpaceAround     ///< Evenly distribute with equal space around each
};

/// @brief Alignment of children along the cross axis.
/// @details Similar to CSS align-items property. Controls how children
/// are positioned perpendicular to the main layout axis.
enum class AlignItems : unsigned char
{
    Start,          ///< Align to start of cross axis
    Center,         ///< Center along cross axis
    End,            ///< Align to end of cross axis
    Stretch         ///< Stretch to fill cross axis
};

/// @brief Whether children wrap to multiple lines when container is full.
/// @details Similar to CSS flex-wrap property. Controls whether children
/// that exceed the container's main axis size wrap to additional lines.
enum class FlexWrap : unsigned char
{
    NoWrap,         ///< All items on single line (default)
    Wrap            ///< Items wrap to next line/column when container full
};

/// @brief Determines how a control's size is calculated.
/// @details Controls whether sizing is automatic based on content,
/// explicitly fixed, or fills available space in the parent.
enum class SizeMode : unsigned char
{
    Auto,           ///< Size based on content and children
    Fixed,          ///< Size is explicitly set (uses Bounds)
    Fill            ///< Fill available space in parent container
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_LAYOUTENUMS_HPP
