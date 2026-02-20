#ifndef SYSTEM_DRAWING_ICONLIBRARY_HPP
#define SYSTEM_DRAWING_ICONLIBRARY_HPP

#include "../Types.hpp"
#include "../Array.hpp"
#include "../String.hpp"
#include "Size.hpp"
#include "Image.hpp"

namespace System::Drawing
{

/******************************************************************************/
/*    System::Drawing::IconSize                                               */
/*                                                                            */
/*    Standard icon sizes for loading from icon libraries.                    */
/******************************************************************************/

/// @brief Standard icon sizes for loading from icon libraries.
enum class IconSize
{
    Small  = 16,   ///< 16x16 pixels (toolbar, menu items)
    Cursor = 24,   ///< 24x24 pixels (mouse cursors)
    Medium = 32,   ///< 32x32 pixels (desktop icons, lists)
    Large  = 48    ///< 48x48 pixels (large icons view)
};

/******************************************************************************/
/*    System::Drawing::IconLibrary                                            */
/*                                                                            */
/*    Represents a PE-format icon library (.icl, .dll, .exe) for loading      */
/*    icons by index or name.                                                 */
/******************************************************************************/

/// @brief Represents a PE-format icon library for loading icons.
/// @details IconLibrary provides access to icons stored in PE-format files
/// such as .icl, .dll, or .exe files. Icons can be loaded by numeric index
/// or by name (if the library contains named resources).
///
/// @par Example
/// @code
/// // Load an icon library
/// IconLibrary* icons = IconLibrary::FromFile("sysicons.icl");
///
/// // Load icons by index or name
/// Image icon1 = icons->FromIndex(0, IconSize::Medium);
/// Image icon2 = icons->FromName("folder-open", IconSize::Small);
///
/// // Get all icons
/// Array<Image> allIcons = icons->GetAll(IconSize::Medium);
///
/// // Clean up
/// delete icons;
/// @endcode
class IconLibrary
{
    Array<UInt8> _fileData;      ///< Cached file data
    String _path;                ///< Path to the icon library file
    Int32 _iconCount;            ///< Number of icons in library
    Array<String> _iconNames;    ///< Cached icon names (if available)

    /// @brief Private constructor - use FromFile() factory method.
    IconLibrary();

    /// @brief Parse the PE file to count icons and cache names.
    void ParseLibrary();

    /// @brief Load an icon from the cached file data.
    /// @param iconIndex Zero-based index of the icon.
    /// @param size Target size in pixels.
    /// @return The loaded and scaled Image.
    Image LoadIcon(Int32 iconIndex, Int32 size) const;

public:
    /// @brief Destructor.
    ~IconLibrary() = default;

    // Non-copyable
    IconLibrary(const IconLibrary&) = delete;
    IconLibrary& operator=(const IconLibrary&) = delete;

    /// @brief Load an icon library from a file.
    /// @param path Path to the icon library file (.icl, .dll, .exe).
    /// @return Pointer to a new IconLibrary instance. Caller owns the memory.
    /// @throws FileNotFoundException If the file does not exist.
    /// @throws InvalidDataException If the file is not a valid PE icon library.
    static IconLibrary* FromFile(const char* path);

    /// @brief Get an icon by index.
    /// @param index Zero-based index of the icon.
    /// @param size Desired icon size.
    /// @return The loaded Image scaled to the requested size.
    /// @throws ArgumentOutOfRangeException If index is out of range.
    Image FromIndex(Int32 index, IconSize size) const;

    /// @brief Get an icon by name.
    /// @param name Name of the icon in the library.
    /// @param size Desired icon size.
    /// @return The loaded Image scaled to the requested size.
    /// @throws ArgumentException If the named icon is not found.
    Image FromName(const char* name, IconSize size) const;

    /// @brief Get all icons from the library.
    /// @param size Desired icon size for all icons.
    /// @return Array of all icons scaled to the requested size.
    Array<Image> GetAll(IconSize size) const;

    /// @brief Get the number of icons in the library.
    /// @return The icon count.
    Int32 GetCount() const
    {
        return _iconCount;
    }

    /// @brief Get the path to the library file.
    /// @return The file path.
    const String& GetPath() const
    {
        return _path;
    }

    /// @brief Get all icon names in the library.
    /// @return Array of icon names (may be empty if library has no named icons).
    const Array<String>& GetNames() const
    {
        return _iconNames;
    }

    /// @brief Check if the library contains a named icon.
    /// @param name Name to check.
    /// @return True if the name exists in the library.
    Boolean HasName(const char* name) const;

    /// @brief Get the index of a named icon.
    /// @param name Name of the icon.
    /// @return Zero-based index, or -1 if not found.
    Int32 GetIndex(const char* name) const;
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_ICONLIBRARY_HPP
