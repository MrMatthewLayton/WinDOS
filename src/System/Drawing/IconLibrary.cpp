#include "IconLibrary.hpp"
#include "Image.hpp"
#include "../IO/File.hpp"
#include "../Exception.hpp"

namespace System::Drawing
{

/******************************************************************************/
/*    IconLibrary Implementation                                              */
/******************************************************************************/

IconLibrary::IconLibrary()
    : _fileData()
    , _path()
    , _iconCount(Int32(0))
    , _iconNames()
{
}

IconLibrary* IconLibrary::FromFile(const char* path)
{
    if (!path || path[0] == '\0')
    {
        throw ArgumentNullException("path");
    }

    if (!IO::File::Exists(path))
    {
        throw FileNotFoundException(path);
    }

    IconLibrary* library = new IconLibrary();
    library->_path = String(path);

    // Get icon count using existing Image method
    library->_iconCount = Image::GetIconLibraryCount(path);

    // Get icon names using existing Image method
    library->_iconNames = Image::GetIconLibraryNames(path);

    return library;
}

Image IconLibrary::FromIndex(Int32 index, IconSize size) const
{
    if (static_cast<int>(index) < 0 || index >= _iconCount)
    {
        throw ArgumentOutOfRangeException("index");
    }

    Int32 sizePixels = Int32(static_cast<int>(size));
    Size iconSize(sizePixels, sizePixels);

    return Image::FromIconLibrary(_path.GetRawString(), index, iconSize);
}

Image IconLibrary::FromName(const char* name, IconSize size) const
{
    if (!name || name[0] == '\0')
    {
        throw ArgumentNullException("name");
    }

    Int32 index = GetIndex(name);
    if (static_cast<int>(index) < 0)
    {
        throw ArgumentException("Icon not found in library.");
    }

    return FromIndex(index, size);
}

Array<Image> IconLibrary::GetAll(IconSize size) const
{
    Array<Image> result(static_cast<int>(_iconCount));

    for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(_iconCount); i += 1)
    {
        result[static_cast<int>(i)] = FromIndex(i, size);
    }

    return result;
}

Boolean IconLibrary::HasName(const char* name) const
{
    return Boolean(static_cast<int>(GetIndex(name)) >= 0);
}

Int32 IconLibrary::GetIndex(const char* name) const
{
    if (!name || name[0] == '\0')
    {
        return Int32(-1);
    }

    String searchName(name);
    for (Int32 i = Int32(0); static_cast<int>(i) < _iconNames.Length(); i += 1)
    {
        if (_iconNames[static_cast<int>(i)] == searchName)
        {
            return i;
        }
    }

    return Int32(-1);
}

} // namespace System::Drawing
