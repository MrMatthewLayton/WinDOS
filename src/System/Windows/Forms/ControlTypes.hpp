#ifndef SYSTEM_WINDOWS_FORMS_CONTROLTYPES_HPP
#define SYSTEM_WINDOWS_FORMS_CONTROLTYPES_HPP

/******************************************************************************/
/*    System::Windows::Forms::ControlType                                     */
/*                                                                            */
/*    Type identification enumeration for safe casting of controls.           */
/******************************************************************************/

namespace System::Windows::Forms
{

/// @brief Type identification enumeration for safe casting of controls.
/// @details Similar to .NET's GetType() mechanism. Enables runtime type
/// identification without RTTI, which may not be available in all DOS environments.
/// Use GetControlType() to query and AsXxx() methods for safe downcasting.
enum class ControlType
{
    Control,           ///< Base control type
    Desktop,           ///< Desktop surface (root of control hierarchy)
    Window,            ///< Top-level window with title bar and frame
    TaskBar,           ///< Windows 95-style taskbar
    TaskBarButton,     ///< Button in taskbar representing an open window
    Button,            ///< Clickable button control
    Picture,           ///< Image display control
    Spectrum,          ///< Vertical color gradient control (32-bit)
    StartMenu,         ///< Windows 95-style Start menu popup
    MenuItem,          ///< Individual item within a menu
    DesktopIconControl,///< Desktop icon with image
    TaskTray           ///< System tray area with notification icons
};

} // namespace System::Windows::Forms

#endif // SYSTEM_WINDOWS_FORMS_CONTROLTYPES_HPP
