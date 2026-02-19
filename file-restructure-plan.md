# File Restructure Plan: One Class Per File

Generated: 2026-02-19

## Overview

The codebase currently has large monolithic files that need to be split into a "one class per file" structure. The directory structure represents the namespace.

---

## Current State

| Directory | Files | Lines | Classes/Types |
|-----------|-------|-------|---------------|
| `System/Drawing/` | 2 | 5,821 | 17 |
| `System/Windows/Forms/` | 2 | 5,056 | 24 |
| **Total** | 4 | 10,877 | 41 |

## Good Examples (Already Follow Pattern)

- `System/IO/Devices/` - 3 classes, 6 files + namespace header
- `System/` - `String`, `Exception`, `Types` each have own files

---

## System/Drawing Restructure

### Current Structure
```
src/System/Drawing/
├── Drawing.hpp    (1,885 lines, 17 types)
└── Drawing.cpp    (3,936 lines)
```

### Target Structure
```
src/System/Drawing/
├── Drawing.hpp           # Master include (includes all below)
├── Enums.hpp             # BufferMode, BorderStyle, FontStyle, StringAlignment
├── Color.hpp
├── Color.cpp
├── Point.hpp
├── Point.cpp
├── Size.hpp
├── Size.cpp
├── Rectangle.hpp
├── Rectangle.cpp
├── Image.hpp
├── Image.cpp
├── SystemIcons.hpp
├── SystemIcons.cpp
├── HatchStyle.hpp
├── HatchStyle.cpp
├── Font.hpp
├── Font.cpp
├── GraphicsBuffer.hpp
├── GraphicsBuffer.cpp
├── Graphics.hpp
├── Graphics.cpp
└── BinaryFormats.hpp     # BitmapFileHeader, BitmapInfoHeader, PE/ICO structs (internal)
```

### Class Breakdown

| Class | Header Lines | Impl Lines | Dependencies |
|-------|--------------|------------|--------------|
| Color | 170 | 100 | Types |
| Point | 60 | 20 | Types |
| Size | 60 | 20 | Types |
| Rectangle | 145 | 50 | Point, Size, Types |
| Image | 270 | 800 | Color, Rectangle, Size |
| SystemIcons | 150 | 50 | Image, Size |
| HatchStyle | 80 | 50 | - |
| Font | 135 | 600 | String, Image |
| GraphicsBuffer | 120 | 400 | Image, Rectangle |
| Graphics | 165 | 500 | Image, Font, Color, Rectangle, GraphicsBuffer |

---

## System/Windows/Forms Restructure

### Current Structure
```
src/System/Windows/Forms/
├── Forms.hpp     (1,979 lines, 24 types)
└── Forms.cpp     (3,077 lines)
```

### Target Structure
```
src/System/Windows/Forms/
├── Forms.hpp             # Master include (includes all below)
├── ControlTypes.hpp      # ControlType enum
├── LayoutEnums.hpp       # FlexDirection, JustifyContent, AlignItems, FlexWrap, SizeMode
├── LayoutProperties.hpp  # LayoutProperties struct, MeasureResult struct
├── EventArgs.hpp         # PaintEventArgs, MouseEventArgs, KeyboardEventArgs
├── SpatialGrid.hpp
├── SpatialGrid.cpp
├── Control.hpp
├── Control.cpp
├── DesktopIcon.hpp       # Simple data struct, no cpp needed
├── DesktopIconControl.hpp
├── DesktopIconControl.cpp
├── Desktop.hpp
├── Desktop.cpp
├── Window.hpp
├── Window.cpp
├── TaskBar.hpp
├── TaskBar.cpp
├── Button.hpp
├── Button.cpp
├── Picture.hpp
├── Picture.cpp
├── SpectrumControl.hpp
├── SpectrumControl.cpp
├── TaskBarButton.hpp
├── TaskBarButton.cpp
├── MenuItem.hpp
├── MenuItem.cpp
├── StartMenu.hpp
└── StartMenu.cpp
```

### Class Breakdown

| Class | Header Lines | Impl Lines | Dependencies |
|-------|--------------|------------|--------------|
| SpatialGrid | 75 | 178 | Rectangle, Array |
| Control | 276 | 950 | LayoutProperties, EventArgs, Rectangle, Array |
| DesktopIconControl | 95 | 147 | Control, Image, String |
| Desktop | 228 | 813 | Control, SpatialGrid, TaskBar, StartMenu, Window |
| Window | 104 | 106 | Control, Font, String |
| TaskBar | 91 | 183 | Control, Button, StartMenu |
| Button | 112 | 103 | Control, Font |
| Picture | 59 | 93 | Control, Image |
| SpectrumControl | 48 | 132 | Control, Color |
| TaskBarButton | 37 | 99 | Button, Window |
| MenuItem | 46 | 86 | Control, Image |
| StartMenu | 53 | 180 | Control, MenuItem |

---

## Master Include Pattern

Each namespace directory should have a master include file that imports everything:

```cpp
// Drawing.hpp
#ifndef SYSTEM_DRAWING_HPP
#define SYSTEM_DRAWING_HPP

// Include all Drawing classes
#include "System/Drawing/Enums.hpp"
#include "System/Drawing/Color.hpp"
#include "System/Drawing/Point.hpp"
#include "System/Drawing/Size.hpp"
#include "System/Drawing/Rectangle.hpp"
#include "System/Drawing/Image.hpp"
#include "System/Drawing/SystemIcons.hpp"
#include "System/Drawing/HatchStyle.hpp"
#include "System/Drawing/Font.hpp"
#include "System/Drawing/GraphicsBuffer.hpp"
#include "System/Drawing/Graphics.hpp"

#endif // SYSTEM_DRAWING_HPP
```

---

## Implementation Order

### Phase 1: Create Enum/Struct Files (No Implementation)
1. `Drawing/Enums.hpp` - extract 4 enums
2. `Forms/ControlTypes.hpp` - extract ControlType enum
3. `Forms/LayoutEnums.hpp` - extract 5 layout enums
4. `Forms/LayoutProperties.hpp` - extract LayoutProperties struct
5. `Forms/EventArgs.hpp` - extract 3 event args classes

### Phase 2: Split Drawing Classes
1. `Color.hpp/.cpp` - standalone, few dependencies
2. `Point.hpp/.cpp` - standalone
3. `Size.hpp/.cpp` - standalone
4. `Rectangle.hpp/.cpp` - depends on Point, Size
5. `Image.hpp/.cpp` - core class, many dependencies
6. `HatchStyle.hpp/.cpp` - standalone patterns
7. `Font.hpp/.cpp` - complex, needs Image
8. `GraphicsBuffer.hpp/.cpp` - needs Image
9. `Graphics.hpp/.cpp` - needs everything
10. `SystemIcons.hpp/.cpp` - needs Image

### Phase 3: Split Forms Classes
1. `SpatialGrid.hpp/.cpp` - utility, few dependencies
2. `Control.hpp/.cpp` - base class
3. `DesktopIcon.hpp` - simple struct
4. `DesktopIconControl.hpp/.cpp` - extends Control
5. `Button.hpp/.cpp` - extends Control
6. `Picture.hpp/.cpp` - extends Control
7. `SpectrumControl.hpp/.cpp` - extends Control
8. `TaskBarButton.hpp/.cpp` - extends Button
9. `MenuItem.hpp/.cpp` - extends Control
10. `Window.hpp/.cpp` - extends Control
11. `TaskBar.hpp/.cpp` - extends Control
12. `StartMenu.hpp/.cpp` - extends Control
13. `Desktop.hpp/.cpp` - extends Control (most complex)

### Phase 4: Update Build System
- Update Makefile with new source files
- Update `rtcorlib.hpp` master include

### Phase 5: Verify
- Build with `make clean && make`
- Run `make test`
- Run `make forms_demo`

---

## Estimated Effort

| Phase | Files | Estimated Time |
|-------|-------|----------------|
| Phase 1: Enums/Structs | 5 | 1-2 hours |
| Phase 2: Drawing | 20 | 4-6 hours |
| Phase 3: Forms | 26 | 4-6 hours |
| Phase 4: Build System | 2 | 30 minutes |
| Phase 5: Verification | - | 1 hour |
| **Total** | **53** | **~12 hours** |

---

## Notes

- Circular dependency: Desktop ↔ Window ↔ TaskBar
  - Use forward declarations in headers
  - Include full headers in .cpp files

- Internal/Binary format structs can stay in consolidated files
  - `BinaryFormats.hpp` for PE/ICO/BMP parsing structs
  - These are implementation details, not public API

- Consider namespace-level forward declaration files:
  - `DrawingForward.hpp` with forward declarations of all Drawing classes
  - `FormsForward.hpp` with forward declarations of all Forms classes
