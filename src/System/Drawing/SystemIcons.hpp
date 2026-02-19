/// @file SystemIcons.hpp
/// @brief System.Drawing.SystemIcons - Named constants for system icon library.

#ifndef SYSTEM_DRAWING_SYSTEMICONS_HPP
#define SYSTEM_DRAWING_SYSTEMICONS_HPP

#include "Image.hpp"
#include "Size.hpp"

namespace System::Drawing
{

/******************************************************************************/
/*    System::Drawing::SystemIcons                                            */
/*                                                                            */
/*    Named constants for icons in sysicons.icl. Use these string names with  */
/*    Image::FromIconLibrary(path, name, size) or SystemIcons::Load(name).    */
/******************************************************************************/

/// @brief Provides named constants for icons in the system icon library.
///
/// This class contains 98 named constants corresponding to icons in sysicons.icl.
/// Use these constants with Image::FromIconLibrary() or the convenience method
/// SystemIcons::Load().
///
/// @par Example
/// @code
/// // Load using SystemIcons helper
/// Image computer = SystemIcons::Load(SystemIcons::Computer, Size::IconLarge);
///
/// // Or load directly from any icon library
/// Image folder = Image::FromIconLibrary(
///     SystemIcons::LibraryPath,
///     SystemIcons::FolderOpen,
///     Size(32, 32)
/// );
/// @endcode
///
/// @par Icon Categories
/// - Application icons: AppLogo, AppMsdos, AppWindos, etc.
/// - Recycle bin: BinEmpty, BinFull
/// - Computer/network: Computer, ComputerNet, ComputerSync
/// - Cursors: CursorHand, CursorLoading, CursorPointer
/// - Devices: DeviceKeyboard, DeviceMouse
/// - Dialogs: DialogError, DialogInfo, DialogQuestion, DialogSuccess, DialogWarning
/// - Drives: DriveCdrom, DriveFloppy, DriveHdd, DriveUsb
/// - Files: File, FileAudioMidi, FileImage, FileTxt, etc.
/// - Folders: FolderApps, FolderClosed, FolderDocs, FolderOpen, etc.
/// - UI elements: UiArrowDown, UiCheck, UiRadio, UiScroll, etc.
class SystemIcons
{
public:
    /// @brief Path to the system icon library file.
    static constexpr const char* LibraryPath = "sysicons.icl";

    // Application icons
    static constexpr const char* AppLogo        = "app-logo";       ///< Application logo
    static constexpr const char* AppMixer       = "app-mixer";      ///< Volume mixer app
    static constexpr const char* AppMsdos       = "app-msdos";      ///< MS-DOS prompt
    static constexpr const char* AppWindos      = "app-windos";     ///< WinDOS logo
    static constexpr const char* AppWinfx1      = "app-winfx-1";    ///< Windows effects 1
    static constexpr const char* AppWinfx2      = "app-winfx-2";    ///< Windows effects 2

    // Recycle bin
    static constexpr const char* BinEmpty       = "bin-empty";      ///< Empty recycle bin
    static constexpr const char* BinFull        = "bin-full";       ///< Full recycle bin

    // Computer and network
    static constexpr const char* Computer       = "computer";       ///< Desktop computer
    static constexpr const char* ComputerNet    = "computer-net";   ///< Networked computer
    static constexpr const char* ComputerSync   = "computer-sync";  ///< Syncing computer

    // Cursors
    static constexpr const char* CursorHand     = "cursor-hand";    ///< Hand/link cursor
    static constexpr const char* CursorLoading  = "cursor-loading"; ///< Loading/wait cursor
    static constexpr const char* CursorPointer  = "cursor-pointer"; ///< Standard arrow cursor

    // Devices
    static constexpr const char* DeviceKeyboard = "device-keyboard"; ///< Keyboard device
    static constexpr const char* DeviceMouse    = "device-mouse";    ///< Mouse device

    // Dialog icons (message boxes)
    static constexpr const char* DialogError1   = "dialog-error-1";    ///< Error icon variant 1
    static constexpr const char* DialogError2   = "dialog-error-2";    ///< Error icon variant 2
    static constexpr const char* DialogInfo1    = "dialog-info-1";     ///< Info icon variant 1
    static constexpr const char* DialogInfo2    = "dialog-info-2";     ///< Info icon variant 2
    static constexpr const char* DialogQuestion1= "dialog-question-1"; ///< Question icon variant 1
    static constexpr const char* DialogQuestion2= "dialog-question-2"; ///< Question icon variant 2
    static constexpr const char* DialogSuccess1 = "dialog-success-1";  ///< Success icon variant 1
    static constexpr const char* DialogSuccess2 = "dialog-success-2";  ///< Success icon variant 2
    static constexpr const char* DialogWarning1 = "dialog-warning-1";  ///< Warning icon variant 1
    static constexpr const char* DialogWarning2 = "dialog-warning-2";  ///< Warning icon variant 2

    // Display
    static constexpr const char* Display        = "display";           ///< Monitor/display
    static constexpr const char* DisplaySettings1 = "display-settings-1"; ///< Display settings 1
    static constexpr const char* DisplaySettings2 = "display-settings-2"; ///< Display settings 2

    // Drives
    static constexpr const char* DriveCdrom     = "drive-cdrom";   ///< CD-ROM drive
    static constexpr const char* DriveFloppy    = "drive-floppy";  ///< Floppy disk drive
    static constexpr const char* DriveHdd       = "drive-hdd";     ///< Hard disk drive
    static constexpr const char* DriveUsb       = "drive-usb";     ///< USB drive

    // Files
    static constexpr const char* File           = "file";              ///< Generic file
    static constexpr const char* FileAudioMidi  = "file-audio-midi";   ///< MIDI audio file
    static constexpr const char* FileAudioPcm   = "file-audio-pcm";    ///< PCM/WAV audio file
    static constexpr const char* FileBinary     = "file-binary";       ///< Binary file
    static constexpr const char* FileFont       = "file-font";         ///< Font file
    static constexpr const char* FileImage      = "file-image";        ///< Image file
    static constexpr const char* FileIso        = "file-iso";          ///< ISO disc image
    static constexpr const char* FileMedia      = "file-media";        ///< Media file
    static constexpr const char* FileRtf        = "file-rtf";          ///< Rich text file
    static constexpr const char* FileSrcAssembly= "file-src-assembly"; ///< Assembly source
    static constexpr const char* FileSrcBasic   = "file-src-basic";    ///< BASIC source
    static constexpr const char* FileSrcC       = "file-src-c";        ///< C source file
    static constexpr const char* FileSrcCpp     = "file-src-cpp";      ///< C++ source file
    static constexpr const char* FileSrcH       = "file-src-h";        ///< C header file
    static constexpr const char* FileSrcHpp     = "file-src-hpp";      ///< C++ header file
    static constexpr const char* FileSystem     = "file-system";       ///< System file
    static constexpr const char* FileTxt        = "file-txt";          ///< Text file
    static constexpr const char* FileXlChart    = "file-xl-chart";     ///< Excel chart
    static constexpr const char* FileXlSheet    = "file-xl-sheet";     ///< Excel spreadsheet

    // Folders
    static constexpr const char* FolderApps     = "folder-apps";       ///< Applications folder
    static constexpr const char* FolderClosed   = "folder-closed";     ///< Closed folder
    static constexpr const char* FolderDocs     = "folder-docs";       ///< Documents folder
    static constexpr const char* FolderLibrary  = "folder-library";    ///< Library folder
    static constexpr const char* FolderOpen     = "folder-open";       ///< Open folder
    static constexpr const char* FolderOpenFiles= "folder-open-files"; ///< Folder with files

    // Mixer/Sound
    static constexpr const char* Mixer          = "mixer";             ///< Audio mixer
    static constexpr const char* Sound          = "sound";             ///< Sound/speaker

    // Network signal strength
    static constexpr const char* NetworkSignal0 = "network-signal-0";  ///< No signal
    static constexpr const char* NetworkSignal1 = "network-signal-1";  ///< Weak signal
    static constexpr const char* NetworkSignal2 = "network-signal-2";  ///< Medium signal
    static constexpr const char* NetworkSignal3 = "network-signal-3";  ///< Strong signal

    // Overlays (for composite icons)
    static constexpr const char* OverlayError   = "overlay-error";     ///< Error overlay (X)
    static constexpr const char* OverlayShortcut= "overlay-shortcut";  ///< Shortcut arrow
    static constexpr const char* OverlaySuccess = "overlay-success";   ///< Success checkmark
    static constexpr const char* OverlayWarning = "overlay-warning";   ///< Warning overlay

    // Shields (security)
    static constexpr const char* Shield         = "shield";            ///< Shield icon
    static constexpr const char* ShieldDanger   = "shield-danger";     ///< Danger shield
    static constexpr const char* ShieldFull     = "shield-full";       ///< Full protection
    static constexpr const char* ShieldInfo     = "shield-info";       ///< Info shield
    static constexpr const char* ShieldSuccess  = "shield-success";    ///< Success shield
    static constexpr const char* ShieldWarning  = "shield-warning";    ///< Warning shield

    // Storage
    static constexpr const char* StoreCdrom     = "store-cdrom";       ///< CD-ROM media
    static constexpr const char* StoreFloppy    = "store-floppy";      ///< Floppy disk media
    static constexpr const char* StoreUsb       = "store-usb";         ///< USB media

    // Text
    static constexpr const char* TextPwrd       = "text-pwrd";         ///< WordPad/word processor
    static constexpr const char* TextSelect     = "text-select";       ///< Text selection

    // Transfer indicators
    static constexpr const char* TxAsync        = "tx-async";          ///< Async transfer
    static constexpr const char* TxIdle         = "tx-idle";           ///< Idle transfer
    static constexpr const char* TxReceive      = "tx-receive";        ///< Receiving data
    static constexpr const char* TxSend         = "tx-send";           ///< Sending data

    // UI elements
    static constexpr const char* UiArrowDown    = "ui-arrow-down";     ///< Down arrow
    static constexpr const char* UiArrowLeft    = "ui-arrow-left";     ///< Left arrow
    static constexpr const char* UiArrowRight   = "ui-arrow-right";    ///< Right arrow
    static constexpr const char* UiArrowUp      = "ui-arrow-up";       ///< Up arrow
    static constexpr const char* UiCheck0       = "ui-check-0";        ///< Unchecked checkbox
    static constexpr const char* UiCheck1       = "ui-check-1";        ///< Checked checkbox
    static constexpr const char* UiCheck2       = "ui-check-2";        ///< Indeterminate checkbox
    static constexpr const char* UiRadio0       = "ui-radio-0";        ///< Unselected radio
    static constexpr const char* UiRadio1       = "ui-radio-1";        ///< Selected radio
    static constexpr const char* UiScroll       = "ui-scroll";         ///< Scrollbar grip
    static constexpr const char* UiScrollDown   = "ui-scroll-down";    ///< Scroll down
    static constexpr const char* UiScrollLeft   = "ui-scroll-left";    ///< Scroll left
    static constexpr const char* UiScrollRight  = "ui-scroll-right";   ///< Scroll right
    static constexpr const char* UiScrollUp     = "ui-scroll-up";      ///< Scroll up

    /// @brief Loads an icon by name from the system icon library.
    /// @param iconName One of the SystemIcons constants (e.g., SystemIcons::Computer).
    /// @param size Desired icon size.
    /// @return Loaded icon image.
    /// @throws FileNotFoundException if sysicons.icl is not found.
    /// @throws InvalidDataException if the icon name is not found.
    static Image Load(const char* iconName, const Size& size)
    {
        return Image::FromIconLibrary(LibraryPath, iconName, size);
    }
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_SYSTEMICONS_HPP
