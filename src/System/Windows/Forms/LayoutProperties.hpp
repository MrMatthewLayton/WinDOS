#ifndef SYSTEM_WINDOWS_FORMS_LAYOUTPROPERTIES_HPP
#define SYSTEM_WINDOWS_FORMS_LAYOUTPROPERTIES_HPP

#include "System/Types.hpp"
#include "System/Windows/Forms/LayoutEnums.hpp"

/******************************************************************************/
/*    System::Windows::Forms::LayoutProperties                                */
/*    System::Windows::Forms::MeasureResult                                   */
/*                                                                            */
/*    Structures for the flexbox-like layout system.                          */
/******************************************************************************/

namespace System::Windows::Forms
{

/******************************************************************************/
/*    System::Windows::Forms::LayoutProperties                                */
/******************************************************************************/

/// @brief Configuration structure for flexbox-like layout behavior.
/// @details Embedded in each Control to configure how it participates in
/// automatic layout. Supports a two-pass layout algorithm (Measure + Arrange)
/// similar to WPF's layout system combined with CSS Flexbox concepts.
///
/// The layout system performs:
/// 1. Measure pass (bottom-up): Each control reports preferred size
/// 2. Arrange pass (top-down): Parent assigns final bounds to children
///
/// @note By default, Windows have participatesInLayout=false (floating).
/// @note Use fluent setters for convenient configuration chaining.
///
/// Example usage:
/// @code
/// control.Layout().SetDirection(FlexDirection::Row)
///                 .SetJustifyContent(JustifyContent::SpaceAround)
///                 .SetFlexGrow(1)
///                 .SetMargin(4);
/// @endcode
struct LayoutProperties
{
    // Container properties (when this control has children)
    FlexDirection direction;        ///< Layout direction for children
    JustifyContent justifyContent;  ///< Main axis alignment
    AlignItems alignItems;          ///< Cross axis alignment
    FlexWrap wrap;                  ///< Whether children wrap to multiple lines
    Int32 gap;                      ///< Space between children (pixels)

    // Self properties (when this control is inside a flex container)
    Int32 flexGrow;                 ///< Growth factor (0 = don't grow, 1+ = proportional)
    Int32 flexShrink;               ///< Shrink factor (0 = don't shrink, 1+ = proportional)

    // Sizing constraints
    SizeMode widthMode;             ///< How width is determined
    SizeMode heightMode;            ///< How height is determined
    Int32 minWidth;                 ///< Minimum width constraint (pixels)
    Int32 minHeight;                ///< Minimum height constraint (pixels)
    Int32 maxWidth;                 ///< Maximum width constraint (pixels)
    Int32 maxHeight;                ///< Maximum height constraint (pixels)

    // Spacing (top, right, bottom, left order)
    Int32 marginTop;                ///< Top margin (pixels)
    Int32 marginRight;              ///< Right margin (pixels)
    Int32 marginBottom;             ///< Bottom margin (pixels)
    Int32 marginLeft;               ///< Left margin (pixels)
    Int32 paddingTop;               ///< Top padding (pixels)
    Int32 paddingRight;             ///< Right padding (pixels)
    Int32 paddingBottom;            ///< Bottom padding (pixels)
    Int32 paddingLeft;              ///< Left padding (pixels)

    // Behavior flags
    bool participatesInLayout;      ///< If false, control is floating (e.g., Windows)
    bool needsLayout;               ///< Dirty flag for layout optimization
    bool alwaysOnTop;               ///< If true, control is painted above other controls
    Int32 zIndex;                   ///< Z-index for paint order (higher = on top, TaskBar uses 1000)

    /// @brief Default constructor with sensible initial values.
    /// @details Initializes with Column direction, Start justification,
    /// Stretch alignment, no gaps/margins/padding, and participates in layout.
    LayoutProperties()
        : direction(FlexDirection::Column)
        , justifyContent(JustifyContent::Start)
        , alignItems(AlignItems::Stretch)
        , wrap(FlexWrap::NoWrap)
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
        , alwaysOnTop(false)
        , zIndex(0)
    {
    }

    /// @brief Set the layout direction for children.
    /// @param dir The FlexDirection (Row or Column).
    /// @return Reference to this for method chaining.
    LayoutProperties& SetDirection(FlexDirection dir)
    {
        direction = dir;
        return *this;
    }

    /// @brief Set main axis alignment for children.
    /// @param jc The JustifyContent alignment mode.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetJustifyContent(JustifyContent jc)
    {
        justifyContent = jc;
        return *this;
    }

    /// @brief Set cross axis alignment for children.
    /// @param ai The AlignItems alignment mode.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetAlignItems(AlignItems ai)
    {
        alignItems = ai;
        return *this;
    }

    /// @brief Set whether children wrap to multiple lines.
    /// @param w The FlexWrap mode.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetWrap(FlexWrap w)
    {
        wrap = w;
        return *this;
    }

    /// @brief Set the gap between children.
    /// @param g Gap size in pixels.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetGap(Int32 g)
    {
        gap = g;
        return *this;
    }

    /// @brief Set the flex grow factor.
    /// @param fg Growth factor (0 = don't grow, positive = proportional growth).
    /// @return Reference to this for method chaining.
    LayoutProperties& SetFlexGrow(Int32 fg)
    {
        flexGrow = fg;
        return *this;
    }

    /// @brief Set the flex shrink factor.
    /// @param fs Shrink factor (0 = don't shrink, positive = proportional shrink).
    /// @return Reference to this for method chaining.
    LayoutProperties& SetFlexShrink(Int32 fs)
    {
        flexShrink = fs;
        return *this;
    }

    /// @brief Set the width sizing mode.
    /// @param mode The SizeMode for width calculation.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetWidthMode(SizeMode mode)
    {
        widthMode = mode;
        return *this;
    }

    /// @brief Set the height sizing mode.
    /// @param mode The SizeMode for height calculation.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetHeightMode(SizeMode mode)
    {
        heightMode = mode;
        return *this;
    }

    /// @brief Set minimum size constraints.
    /// @param w Minimum width in pixels.
    /// @param h Minimum height in pixels.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetMinSize(Int32 w, Int32 h)
    {
        minWidth = w;
        minHeight = h;
        return *this;
    }

    /// @brief Set maximum size constraints.
    /// @param w Maximum width in pixels.
    /// @param h Maximum height in pixels.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetMaxSize(Int32 w, Int32 h)
    {
        maxWidth = w;
        maxHeight = h;
        return *this;
    }

    /// @brief Set uniform margin on all sides.
    /// @param all Margin size in pixels applied to all sides.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetMargin(Int32 all)
    {
        marginTop = marginRight = marginBottom = marginLeft = all;
        return *this;
    }

    /// @brief Set margin with separate vertical and horizontal values.
    /// @param vertical Margin for top and bottom (pixels).
    /// @param horizontal Margin for left and right (pixels).
    /// @return Reference to this for method chaining.
    LayoutProperties& SetMargin(Int32 vertical, Int32 horizontal)
    {
        marginTop = marginBottom = vertical;
        marginRight = marginLeft = horizontal;
        return *this;
    }

    /// @brief Set margin with individual values for each side.
    /// @param top Top margin in pixels.
    /// @param right Right margin in pixels.
    /// @param bottom Bottom margin in pixels.
    /// @param left Left margin in pixels.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetMargin(Int32 top, Int32 right, Int32 bottom, Int32 left)
    {
        marginTop = top;
        marginRight = right;
        marginBottom = bottom;
        marginLeft = left;
        return *this;
    }

    /// @brief Set uniform padding on all sides.
    /// @param all Padding size in pixels applied to all sides.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetPadding(Int32 all)
    {
        paddingTop = paddingRight = paddingBottom = paddingLeft = all;
        return *this;
    }

    /// @brief Set padding with separate vertical and horizontal values.
    /// @param vertical Padding for top and bottom (pixels).
    /// @param horizontal Padding for left and right (pixels).
    /// @return Reference to this for method chaining.
    LayoutProperties& SetPadding(Int32 vertical, Int32 horizontal)
    {
        paddingTop = paddingBottom = vertical;
        paddingRight = paddingLeft = horizontal;
        return *this;
    }

    /// @brief Set padding with individual values for each side.
    /// @param top Top padding in pixels.
    /// @param right Right padding in pixels.
    /// @param bottom Bottom padding in pixels.
    /// @param left Left padding in pixels.
    /// @return Reference to this for method chaining.
    LayoutProperties& SetPadding(Int32 top, Int32 right, Int32 bottom, Int32 left)
    {
        paddingTop = top;
        paddingRight = right;
        paddingBottom = bottom;
        paddingLeft = left;
        return *this;
    }

    /// @brief Set whether this control participates in automatic layout.
    /// @param participates If false, control is floating and positioned manually.
    /// @return Reference to this for method chaining.
    /// @note Windows typically have this set to false.
    LayoutProperties& SetParticipatesInLayout(bool participates)
    {
        participatesInLayout = participates;
        return *this;
    }

    /// @brief Set whether this control should always be painted on top.
    /// @param onTop If true, control is painted above other controls.
    /// @return Reference to this for method chaining.
    /// @note Use for menus, tooltips, or floating toolbars.
    LayoutProperties& SetAlwaysOnTop(bool onTop)
    {
        alwaysOnTop = onTop;
        return *this;
    }

    /// @brief Set the z-index for paint order.
    /// @param z Z-index value (higher = painted on top).
    /// @return Reference to this for method chaining.
    /// @note TaskBar uses zIndex=1000. Normal controls use 0.
    LayoutProperties& SetZIndex(Int32 z)
    {
        zIndex = z;
        return *this;
    }
};

/******************************************************************************/
/*    System::Windows::Forms::MeasureResult                                   */
/******************************************************************************/

/// @brief Result of the Measure pass during layout calculation.
/// @details Contains the preferred size of a control based on its content
/// and children. Used during the two-pass layout algorithm.
struct MeasureResult
{
    Int32 preferredWidth;   ///< Preferred width based on content (pixels)
    Int32 preferredHeight;  ///< Preferred height based on content (pixels)

    /// @brief Default constructor initializing to zero size.
    MeasureResult() : preferredWidth(0), preferredHeight(0)
    {
    }

    /// @brief Constructor with explicit dimensions.
    /// @param w Preferred width in pixels.
    /// @param h Preferred height in pixels.
    MeasureResult(Int32 w, Int32 h) : preferredWidth(w), preferredHeight(h)
    {
    }
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_LAYOUTPROPERTIES_HPP
