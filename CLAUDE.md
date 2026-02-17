# CLAUDE.md - Project Context for AI Assistants

## Project Overview

**WinDOS** is a .NET-style Base Class Library (BCL) for DOS, implementing a Windows 95-inspired graphical user interface system that runs in DOS protected mode via DJGPP.

## What We're Building

A complete GUI framework for DOS that mimics both:
1. **.NET Framework APIs** - Type wrappers, String, Array<T>, Exception hierarchy, Console
2. **Windows 95 UI** - Desktop, TaskBar, Windows, Buttons, and a WIMP (Windows, Icons, Menus, Pointer) environment

The goal is to create a nostalgic yet functional DOS GUI system using modern C++17, with APIs familiar to .NET developers.

## Build System

```bash
# Compiler: DJGPP cross-compiler for DOS
CXX = ~/djgpp/bin/i586-pc-msdosdjgpp-g++

# Build commands
make              # Build BCL library (libbcl.a)
make test         # Build comprehensive test suite (test.exe)
make tests        # Build all test executables
make demo         # Build graphics demo
make forms_demo   # Build Windows 95-style forms demo
make clean        # Clean build artifacts
```

**Output:**
- Library: `build/lib/libbcl.a`
- Tests: `build/bin/test.exe` (comprehensive), `build/bin/test_*.exe` (individual)
- Demos: `build/bin/gfxdemo.exe`, `build/bin/forms.exe`

## Project Structure

```
windos/
├── src/                        # Source code (organized by namespace)
│   ├── BCL.hpp                 # Master include file
│   ├── Platform/               # Low-level DOS abstractions
│   │   ├── Platform.hpp
│   │   └── DOS/
│   │       ├── System.hpp/.cpp
│   │       ├── Video.hpp/.cpp
│   │       ├── Graphics.hpp/.cpp
│   │       ├── Keyboard.hpp/.cpp
│   │       └── Mouse.hpp/.cpp
│   └── System/                 # System namespace
│       ├── Types.hpp/.cpp      # Boolean, Char, Int*, Float*
│       ├── Exception.hpp/.cpp  # Exception hierarchy
│       ├── String.hpp/.cpp     # Immutable String class
│       ├── Array.hpp           # Generic Array<T> template
│       ├── Console.hpp/.cpp    # Console I/O with colors
│       ├── Math.hpp            # Math utilities
│       ├── Memory.hpp/.cpp     # MemoryPool, StringIntern
│       ├── IO/                 # System.IO namespace
│       │   └── IO.hpp/.cpp     # File class
│       ├── Drawing/            # System.Drawing namespace
│       │   └── Drawing.hpp/.cpp
│       ├── Devices/            # System.Devices namespace
│       │   └── Devices.hpp/.cpp
│       └── Windows/
│           └── Forms/          # System.Windows.Forms namespace
│               └── Forms.hpp/.cpp
├── tests/                      # Tests and demos
│   ├── test_framework.hpp
│   ├── test.cpp                # Comprehensive test suite
│   ├── test_*.cpp              # Individual unit tests
│   ├── gfx_demo.cpp
│   └── forms_demo.cpp
├── assets/                     # Asset files (bitmaps, icons)
├── build/                      # Build output directory
├── Makefile
├── CLAUDE.md                   # Project context
└── WinDOS.md                   # API documentation
```

---

# CODE REVIEW & SCORING

## Overall Scores

| Category | Score | Notes |
|----------|-------|-------|
| **Quality** | 90/100 | Clean structure, consistent APIs, well-documented |
| **Performance** | 88/100 | StringBuilder, MemoryPool, StringIntern, transparency |
| **Security** | 90/100 | Bounds checking, checked arithmetic, proper exceptions |
| **Pitfalls** | 94/100 | All critical and medium issues resolved |

---

## Quality Assessment (85/100)

### Strengths
- Clean namespace organization mirroring .NET
- Consistent coding style throughout
- Good use of RAII for resource management
- Move semantics implemented correctly
- Well-structured exception hierarchy
- Comprehensive test coverage
- **[FIXED] Wrapper types (Int32, Boolean, Char) used consistently in public APIs**
- **[FIXED] Comparison operators support both wrapper and primitive types to avoid ambiguity**

### Issues

#### Resolved: Type Consistency
~~The codebase inconsistently uses primitives instead of wrapper types in public APIs.~~

**Status:** FIXED - All public APIs now use wrapper types. Added comparison operators for primitive types to resolve operator ambiguity (e.g., `Int32 == 0` now works without explicit casts).

---

## Performance Assessment (88/100)

### Strengths
- Lookup table for chunky-to-planar conversion (Drawing.cpp:109-126)
- Dirty rectangle optimization for VGA updates
- Fast memset-based rectangle filling
- Efficient Bresenham line algorithm
- Move semantics prevent unnecessary copies
- **[NEW] Image::CopyFrom with transparency support for sprite rendering**
- **[NEW] StringBuilder class for efficient string building**
- **[NEW] MemoryPool class for fast fixed-size allocations**
- **[NEW] StringIntern class for string deduplication and O(1) comparison**

### Issues

#### P1/P2: String Operations - MITIGATED
~~String operations create excessive allocations; += is O(n) per append.~~

**Status:** MITIGATED - Added `StringBuilder` class with:
- Pre-allocated buffer with capacity-based growth
- `Append()` methods with O(1) amortized complexity
- Method chaining support: `sb.Append("Hello").Append(' ').Append("World")`
- `AppendLine()` for line-based building
- `Insert()`, `Remove()`, `Clear()` for buffer manipulation

String class remains immutable by design (like .NET), use StringBuilder for loops.

#### P3: Control Hierarchy Traversal - FIXED
~~Linear search through children for hit testing.~~

**Status:** FIXED - Added `SpatialGrid` class with:
- 64x64 pixel cells for O(1) average hit testing
- `Insert()`, `Remove()`, `HitTest()` methods
- Desktop now uses spatial grid for fast child lookups
- Falls back to linear search for edge cases

#### P4: Image::CopyFrom Without Transparency - FIXED
~~Currently copies all pixels. Needs transparent pixel skipping for sprites.~~

**Status:** FIXED - Added `CopyFrom(src, x, y, transparentColor)` overload that skips pixels matching the transparent color.

---

## Security Assessment (87/100)

### Strengths
- Array bounds checking with exceptions
- String index validation
- Null pointer checks in most places
- Division by zero protection in wrapper types
- **[NEW] Image::FromBitmap throws descriptive exceptions (FileNotFoundException, InvalidDataException)**
- **[NEW] Added IOException, FileNotFoundException, InvalidDataException to exception hierarchy**

### Issues

#### S1: Integer Overflow Protection - FIXED
~~No overflow protection in String operations.~~

**Status:** FIXED - Added `System::Math` class with:
- `CheckedAdd()`, `CheckedSubtract()`, `CheckedMultiply()` - throw `OverflowException`
- `TryAdd()`, `TrySubtract()`, `TryMultiply()` - return success/failure
- `CheckedCast()` for safe type conversions
- String operations (Replace, operator+) now use checked arithmetic

#### S2: File Path Injection (Low Risk in DOS)
```cpp
// Drawing.cpp:311
FILE* file = std::fopen(path, "rb");
```
No path validation. Low risk in DOS environment but should validate.

#### S3: Unchecked malloc Returns
```cpp
// Drawing.cpp:137
_data = static_cast<unsigned char*>(std::malloc(size));
if (_data) { ... }  // Good - checked
```
Most malloc calls are checked, but verify all are.

---

## Pitfalls Assessment (94/100)

### Resolved Issues

#### PIT-1: Hardcoded Screen Dimensions - FIXED
~~Screen size was hardcoded throughout (640x480).~~

**Status:** FIXED - Desktop now queries `Display::GetCurrent()` for screen dimensions and caches them. All Forms components (Control, Desktop, TaskBar) use dynamic screen dimensions. Added `Desktop::ScreenWidth()` and `Desktop::ScreenHeight()` accessors.

#### PIT-3: Window Type Check by Position - FIXED
~~Dangerous static_cast based on Y position to determine if a control was a Window.~~

**Status:** FIXED - Added comprehensive type identification system:
- `ControlType` enum for runtime type identification
- `GetControlType()` virtual method (similar to .NET `GetType()`)
- `AsWindow()`, `AsButton()`, etc. safe cast methods (similar to .NET `as` operator)
- `IsWindow()`, `IsButton()`, etc. type check methods (similar to .NET `is` operator)
- HandleMouse now uses `child->AsWindow()` instead of position-based casting

#### PIT-2: Control Destructor Deletes Children - DOCUMENTED
~~Undocumented ownership behavior could lead to crashes.~~

**Status:** DOCUMENTED - Added comprehensive ownership documentation:
- Block comment in Forms.hpp Control class explaining ownership rules
- Comments on AddChild/RemoveChild methods
- Full "Ownership Semantics" section in WinDOS.md with correct/incorrect examples
- Expanded notes in CLAUDE.md Important Notes section

This is intentional behavior (composite pattern), now properly documented.

### Remaining Issues

None critical. Minor items remain in MEDIUM/LOW priority.

#### PIT-4: Static Frame Buffer
```cpp
// Drawing.cpp:564
GraphicsBuffer* GraphicsBuffer::_frameBuffer = nullptr;
```
Global state makes testing difficult and prevents multiple displays.

#### PIT-5: Missing Virtual Destructor Warning
All control classes have virtual destructors (good), but ensure all base classes do.

---

## Outstanding TODO Items

### HIGH PRIORITY - Must Fix Before Production

1. **[x] Type Consistency** - Replace primitive types with wrapper types in all public APIs ✓ DONE
2. **[x] Screen Size Constants** - Remove hardcoded 640x480; use Display properties ✓ DONE
3. **[x] Safe Window Casting** - Add type identification to Control hierarchy ✓ DONE
4. **[x] Document Ownership** - Clear documentation on child control ownership ✓ DONE

### MEDIUM PRIORITY - Should Fix

5. **[x] String Performance** - StringBuilder class implemented ✓ DONE
6. **[x] Transparency Support** - Image::CopyFrom with transparent color ✓ DONE
7. **[x] Integer Overflow** - Math class with checked arithmetic ✓ DONE
8. **[x] Error Handling** - Image::FromBitmap throws exceptions ✓ DONE

### LOW PRIORITY - Nice to Have

9. **[x] Spatial Partitioning** - SpatialGrid class with 64px cells for O(1) hit testing ✓ DONE
10. **[x] String Interning** - StringIntern class with hash table pool ✓ DONE
11. **[x] Memory Pool** - MemoryPool class for fixed-size allocations ✓ DONE

---

## Features Ported from Legacy Code

All HIGH and MEDIUM priority features have been ported from legacy code.

### COMPLETED
| Feature | Location | Description |
|---------|----------|-------------|
| ~~File I/O System~~ | System/IO/IO.hpp | `File::ReadAllBytes()`, `File::Exists()`, `File::GetSize()` ✓ DONE |
| ~~Icon Loading~~ | System/Drawing/Drawing.hpp | `Image::FromIcon()`, `Image::FromIconLibrary()` with scaling and named icon support ✓ DONE |
| ~~Desktop Icons~~ | System/Windows/Forms/Forms.hpp | `Desktop::AddIcon()`, `DesktopIcon` struct, grid layout ✓ DONE |
| ~~Palette Fade~~ | System/Devices/Devices.hpp | `Display::FadeIn(ms)`, `FadeOut(ms)` - VGA and VBE modes ✓ DONE |
| ~~Custom Cursors~~ | System/Windows/Forms/Forms.hpp | `Desktop::SetCursor()`, `LoadCursorFromLibrary()` ✓ DONE |
| ~~Hatch Patterns~~ | System/Drawing/Drawing.hpp | 38 fill patterns in `HatchStyle` class ✓ DONE |
| ~~Pattern Fills~~ | System/Drawing/Drawing.hpp | `Graphics::FillRectangle(rect, HatchStyle, fg, bg)` ✓ DONE |
| ~~Math Utilities~~ | System/Math.hpp | `Abs()`, `Min()`, `Max()`, `Clamp()`, `Swap()` ✓ DONE |

### LOW PRIORITY - Remaining
| Feature | Source | Description |
|---------|--------|-------------|
| String Formatting | text.h | `StringFormatter::Format()` with varargs |
| MutableArray | collect.h | `InsertAt()`, `RemoveAt()` |
| ASCII Constants | text.h | `ASCII_ESCAPE`, `ASCII_TAB`, etc. |

---

## Code Conventions

- C++17 features (if constexpr, structured bindings, etc.)
- RAII for resource management
- Copy/move semantics throughout
- Bounds checking with exceptions
- Static methods for hardware access facades
- Virtual methods for control customization
- **USE WRAPPER TYPES** (`Int32`, `Boolean`, `String`) not primitives in public APIs

## Testing

Run comprehensive tests:
```bash
make test
# Copy test.exe and CWSDPMI.EXE to DOS environment
# Run: test.exe
```

Tests use a simple framework in `test_framework.hpp`:
```cpp
ASSERT(condition, "message");
ASSERT_EQ(expected, actual, "message");
ASSERT_THROWS(expression, ExceptionType, "message");
```

## Important Notes for Development

1. **VGA Planar Mode**: Mode 0x12 uses 4 bit planes. Each pixel requires writing to multiple planes via port I/O.

2. **Coordinate Systems**:
   - Control bounds are relative to parent
   - Use `ScreenBounds()` for absolute coordinates
   - Client area excludes borders/title bars

3. **Memory**: DOS protected mode via CWSDPMI.EXE provides 32-bit flat memory model.

4. **File Paths**: Use backslashes in DOS paths (`assets\\test.bmp`).

5. **No Exceptions in Destructors**: Destructors don't throw; cleanup is silent.

6. **Child Control Ownership**: Parent controls OWN and DELETE their children in destructor.
   - Always use `new` to allocate children: `new Button(parent, bounds)`
   - Never allocate children on the stack - causes double-free crash
   - Never manually delete children that have a parent
   - `RemoveChild()` releases ownership - caller must then delete or re-parent
   - See `WinDOS.md` Control class documentation for full details

---

## VBE 2.0+ High-Resolution Support (Complete)

### Overview

VESA BIOS Extensions (VBE) 2.0+ support for high-resolution modes (800x600+) with linear framebuffer access. Fully integrated with the unified 32bpp graphics pipeline.

### What Was Implemented

#### Platform Layer (`src/Platform/DOS/Graphics.hpp/.cpp`)
- `VbeInfoBlock` and `VbeModeInfoBlock` structs for VBE BIOS data
- `VbeSurface` struct to track LFB selector, address, pitch, dimensions
- `DetectVBE()` - INT 10h AX=4F00h to detect VBE presence
- `GetVBEModeInfo()` - INT 10h AX=4F01h to query mode capabilities
- `SetVBEMode()` - INT 10h AX=4F02h with LFB mapping via DPMI
- `CleanupVBE()` - Proper resource cleanup (LDT descriptor, physical mapping)
- `GetLfbSelector()` - Returns LDT selector for LFB access

#### Drawing Layer (`src/System/Drawing/Drawing.hpp/.cpp`)
- Unified `Color` class (32-bit ARGB) with `Lerp()` for gradients
- Unified `Image` class (32-bit ARGB pixels)
- `GraphicsBuffer::CreateFrameBuffer32()` with bpp parameter
- `Linear32BufferWriter()` - Copies framebuffer to LFB via `movedata()`
- Supports both 24bpp and 32bpp VBE modes
- Bayer dithering for 4bpp/8bpp display modes

#### Device Layer (`src/System/Devices/Devices.hpp/.cpp`)
- `Display::IsVbeAvailable()` - Checks for VBE 2.0+
- `Display::DetectVbeMode()` - Finds suitable VBE mode
- `Display::SetMode()` updated to handle VBE modes with LFB setup

#### Forms Layer (`src/System/Windows/Forms/Forms.hpp/.cpp`)
- `SpectrumControl` - Vertical gradient widget (white→color→black)
- Uses unified `Color` and `Image` classes

### Key Technical Findings

#### 1. Transfer Buffer Corruption
**Problem:** DJGPP's `__tb` transfer buffer is shared with DOS I/O (printf, etc.).
**Solution:** Copy data from transfer buffer IMMEDIATELY after BIOS calls, before any printf.
```cpp
__dpmi_int(0x10, &regs);
dosmemget(tbAddr, sizeof(VbeInfoBlock), &vbeInfo);  // Copy FIRST
printf("Result: ...");  // printf can now safely use __tb
```

#### 2. LDT Selector Approach (Not Near Pointers)
**Problem:** `__djgpp_nearptr_enable()` causes page faults with some DPMI hosts.
**Solution:** Use LDT descriptor with `movedata()`:
```cpp
int selector = __dpmi_allocate_ldt_descriptors(1);
__dpmi_set_segment_base_address(selector, memInfo.address);
__dpmi_set_segment_limit(selector, lfbSize - 1);
movedata(_my_ds(), srcOffset, selector, dstOffset, rowBytes);
```

#### 3. 24bpp vs 32bpp Mode Detection
**Problem:** VBE mode 0x115 is often 24bpp (3 bytes/pixel), not 32bpp.
**Solution:** Track actual bpp and convert pixel data accordingly:
- 32bpp: Direct copy (4 bytes/pixel)
- 24bpp: Convert ARGB to BGR (3 bytes/pixel)

#### 4. Unified 32bpp Pipeline (Implemented)
**Problem:** Dual 8-bit/32-bit buffer approach caused compositing complexity and artifacts.
**Solution:** Unified 32bpp pipeline - all drawing uses single 32-bit `Image`. Bayer dithering converts to VGA palette indices only at display time.

### Current Status: COMPLETE

**What Works:**
- VBE mode detection and initialization (800x600 and higher)
- Unified 32bpp graphics pipeline throughout
- Single `Color` class (32-bit ARGB) - matches .NET `System.Drawing.Color`
- Single `Image` class (32-bit pixels)
- Bayer dithering for 4bpp/8bpp VGA modes
- Direct output to 24bpp/32bpp VBE linear framebuffer
- SpectrumControl with smooth gradients
- All Forms controls work correctly in all display modes

### Files Modified for VBE Support

| File | Changes |
|------|---------|
| `src/Platform/DOS/Graphics.hpp` | VBE structs, VbeSurface, function declarations |
| `src/Platform/DOS/Graphics.cpp` | VBE detection, mode setting, LFB mapping |
| `src/System/Drawing/Drawing.hpp` | Unified Color/Image (32-bit), CreateFrameBuffer32 |
| `src/System/Drawing/Drawing.cpp` | Bayer dithering, Linear32BufferWriter, 24/32bpp conversion |
| `src/System/Devices/Devices.hpp` | Display VBE members and methods |
| `src/System/Devices/Devices.cpp` | VBE mode detection and SetMode |
| `src/System/Windows/Forms/Forms.hpp` | SpectrumControl class |
| `src/System/Windows/Forms/Forms.cpp` | SpectrumControl implementation |
| `tests/forms_demo.cpp` | VBE mode usage |
| `tests/vbetest.cpp` | Standalone VBE diagnostic tool |
| `tests/vbebcl.cpp` | VBE test using BCL library |
| `tests/vbeform.cpp` | Minimal VBE forms test |

### Test Programs

```bash
make vbe_debug   # Standalone VBE step-by-step test (vbetest.exe)
make vbe_bcl     # VBE test with BCL graphics (vbebcl.exe)
make vbe_form    # Minimal VBE forms test (vbeform.exe)
make forms_demo  # Full forms demo with VBE (forms.exe)
```

### QEMU Configuration for VBE

Use `-vga std` for proper VBE 2.0+ emulation:
```bash
qemu-system-i386 -vga std -m 64 ...
```

Note: `-device VGA` doesn't properly emulate VBE info block responses.

---

## Flexbox-like Layout System (Complete)

### Overview

A CSS Flexbox-inspired layout system that automatically calculates and assigns control positions using a two-pass algorithm (Measure + Arrange). This solves the problem of hardcoded screen coordinates and enables responsive UI design.

### What Was Implemented

#### Layout Enums (`src/System/Windows/Forms/Forms.hpp`)
- `FlexDirection` - Row or Column layout direction
- `JustifyContent` - Main axis alignment (Start, Center, End, SpaceBetween, SpaceAround)
- `AlignItems` - Cross axis alignment (Start, Center, End, Stretch)
- `SizeMode` - How size is determined (Auto, Fixed, Fill)

#### LayoutProperties Struct
Embedded in each Control to configure layout behavior:
```cpp
struct LayoutProperties {
    FlexDirection direction;      // Row or Column
    JustifyContent justifyContent;// Main axis positioning
    AlignItems alignItems;        // Cross axis alignment
    Int32 gap;                    // Space between children
    Int32 flexGrow;               // Proportional growth factor
    Int32 flexShrink;             // Proportional shrink factor
    SizeMode widthMode, heightMode;
    Int32 minWidth, minHeight, maxWidth, maxHeight;
    Int32 marginTop, marginRight, marginBottom, marginLeft;
    Int32 paddingTop, paddingRight, paddingBottom, paddingLeft;
    bool participatesInLayout;    // false = floating (Windows)
    bool needsLayout;             // Dirty flag
};
```

#### Control Layout Methods
- `Layout()` - Access layout properties
- `Measure(availableWidth, availableHeight)` - Bottom-up size calculation
- `Arrange(finalBounds)` - Top-down positioning
- `PerformLayout()` - Trigger full layout pass
- `InvalidateLayout()` - Mark as needing layout
- `GetPreferredSize()` - Override for content-based sizing

### Layout Algorithm

**Phase 1: Measure (bottom-up)**
- Each control reports preferred size based on content/children
- Children measured first, then parent accumulates
- Respects min/max constraints

**Phase 2: Arrange (top-down)**
- Parent assigns final bounds to children based on:
  - Available space
  - FlexGrow distribution
  - JustifyContent positioning
  - AlignItems alignment

### Key Design Decisions

1. **No RTTI**: Uses existing ControlType enum for type identification
2. **No STL**: Uses existing Array<T> template
3. **Dirty flag optimization**: Only recalculate when `needsLayout = true`
4. **Backward compatible**: Default settings preserve existing behavior
5. **Fluent setters**: `control.Layout().SetFlexGrow(1).SetMargin(4)`
6. **Windows are floating**: `participatesInLayout = false` by default

### Files Modified

| File | Changes |
|------|---------|
| `src/System/Windows/Forms/Forms.hpp` | Layout enums, LayoutProperties, MeasureResult, Control layout methods |
| `src/System/Windows/Forms/Forms.cpp` | Measure(), Arrange(), ArrangeFlexChildren(), PerformLayout(), control updates |
| `tests/test_layout.cpp` | Comprehensive test suite (59 tests) |
| `tests/forms_demo.cpp` | Updated to demonstrate layout system |

### Test Results

All 59 layout tests pass:
- Layout properties defaults and fluent API
- Measure pass (single control, fixed size, min/max, row/column)
- Arrange pass (positioning, FlexGrow, JustifyContent, AlignItems)
- Floating controls, nested layout, padding, margins

### Usage Example

```cpp
// Window with row layout for spectrum controls
window->Layout().SetDirection(FlexDirection::Row)
                .SetJustifyContent(JustifyContent::SpaceAround)
                .SetAlignItems(AlignItems::Stretch)
                .SetPadding(10);

// Child controls with flex grow
SpectrumControl* red = new SpectrumControl(window, bounds, Color::Red);
red->Layout().SetFlexGrow(1).SetMargin(5);

SpectrumControl* green = new SpectrumControl(window, bounds, Color::Green);
green->Layout().SetFlexGrow(1).SetMargin(5);

// Trigger layout calculation
window->PerformLayout();
```

---

## Outstanding TODO Items (Updated)

### HIGH PRIORITY

12. **[x] Unified 32bpp Graphics Pipeline** - ✓ DONE
    - Unified `Color` class (32-bit ARGB), old 8-bit removed
    - Unified `Image` class (32-bit pixels), old 8-bit removed
    - All Graphics methods use 32-bit Color/Image
    - Bayer dithering for 4bpp/8bpp display modes
    - Single framebuffer, mode-specific flush

13. **[ ] VGA Palette Control** - Allow setting custom palette colors
    - `Display::SetPaletteColor(index, r, g, b)`
    - `Display::SetPalette(colors[256])`
    - Support for 4bpp (16 colors) and 8bpp (256 colors) modes
    - Useful for optimizing dithering with application-specific palettes

14. **[x] Flexbox-like Layout System** - ✓ DONE
    - Two-pass layout algorithm (Measure + Arrange)
    - FlexDirection, JustifyContent, AlignItems, FlexGrow
    - Padding, margins, gaps, min/max constraints
    - Floating controls (Windows don't participate in layout)
    - 59 unit tests passing

### MEDIUM PRIORITY

15. **[x] Fix SpectrumControl in VBE mode** - ✓ DONE (resolved by unified 32bpp pipeline)

16. **[x] Fix StartMenu hover highlighting** - ✓ DONE
    - Menu items now properly highlight blue on hover
    - Items revert to grey when mouse leaves
    - Fixed by propagating mouse events to all menu items

### COMPLETED - TaskBar Window List

17. **[x] TaskBar window buttons** - ✓ DONE
    - Window buttons automatically appear in taskbar when windows are created
    - Clicking a taskbar button focuses the corresponding window
    - Active window's button shows pressed/sunken state
    - Button states properly separated: toggle state (persistent) vs mouse state (temporary)
    - Fixed virtual dispatch issue: Window constructor registers with taskbar after base class construction

18. **[x] Button toggle state** - ✓ DONE
    - Start button stays pressed while start menu is open
    - Window buttons stay pressed while their window is focused
    - Added `_isToggled` and `_isMouseDown` separation in Button class
    - Visual state = toggled OR mouse down

### COMPLETED - Recent Features

24. **[x] Named Icon Support** - ✓ DONE
    - `Image::FromIconLibrary(path, iconName, size)` - Load icons by name from PE resources
    - `Image::GetIconLibraryNames(path)` - Get all icon names in a library
    - `Image::GetIconLibraryIndex(path, iconName)` - Get index for a named icon
    - `SystemIcons` class with 98 named constants (Computer, FolderOpen, etc.)
    - Parses PE resource directory for RT_GROUP_ICON named entries
    - UTF-16LE name strings converted to ASCII

25. **[x] VBE 3.0 Gamma Ramp Support** - ✓ DONE (future compatibility)
    - `Display::IsGammaSupported()` - Check if VBE 3.0 gamma is available
    - `Display::SetGammaScale(scale)` - Set brightness via gamma table
    - Platform layer: `IsGammaSupported()`, `SetGammaTable()`, `GetGammaTable()`
    - Uses INT 10h AX=4F15h (VBE 3.0 gamma/DAC functions)
    - Note: QEMU's vgabios only implements VBE 2.0; gamma functions return unsupported
    - FadeIn/FadeOut attempts gamma ramp first, falls back to pixel processing

### LOW PRIORITY - Layout System Future Work

26. **[ ] Wrap support** - Add `wrap` property for multi-line layouts
    - When children exceed container, wrap to next line
    - Similar to CSS `flex-wrap`

27. **[ ] Layout animations** - Smooth transitions when layout changes
    - Interpolate between old and new positions
    - Useful for window minimize/maximize effects

28. **[ ] Auto-layout on resize** - Trigger layout when control size changes
    - Currently requires manual PerformLayout() call
    - Could integrate with SetBounds() automatically

29. **[ ] Layout for TaskBar buttons** - Use flexbox for auto-arrangement
    - Currently uses manual positioning in AddWindowButton/RemoveWindowButton
    - Could use Row layout with FlexGrow for automatic distribution

30. **[ ] StartMenu/MenuItem relative positioning** - Refactor to use layout system
    - Currently uses some hardcoded coordinates
    - Could benefit from Column layout for menu items

---

## Named Icon Support (Complete)

### Overview

PE-format icon libraries (.icl, .dll, .exe) store icons with both numeric IDs and string names. The named icon support allows loading icons by their human-readable names instead of opaque indices.

### What Was Implemented

#### Image Class Extensions (`src/System/Drawing/Drawing.hpp/.cpp`)
- `FromIconLibrary(path, iconName, size)` - Load icon by name with automatic scaling
- `GetIconLibraryNames(path)` - Returns Array<String> of all named icons
- `GetIconLibraryIndex(path, iconName)` - Get numeric index for a named icon
- PE resource directory parsing for RT_GROUP_ICON entries
- UTF-16LE name string decoding

#### SystemIcons Class (`src/System/Drawing/Drawing.hpp`)
Provides 98 named constants for the standard icon library:
```cpp
class SystemIcons {
public:
    static constexpr const char* LibraryPath = "sysicons.icl";

    // Hardware icons
    static constexpr const char* Computer = "computer";
    static constexpr const char* ComputerNet = "computer-net";
    static constexpr const char* Disk35 = "disk-35";
    static constexpr const char* DiskHard = "disk-hard";
    static constexpr const char* DiskCD = "disk-cd";

    // Folder icons
    static constexpr const char* Folder = "folder";
    static constexpr const char* FolderOpen = "folder-open";
    static constexpr const char* FolderApps = "folder-apps";
    static constexpr const char* FolderDocs = "folder-docs";

    // File icons
    static constexpr const char* FileBlank = "file-blank";
    static constexpr const char* FileTxt = "file-txt";
    static constexpr const char* FileExe = "file-exe";

    // UI icons
    static constexpr const char* CursorPointer = "cursor-pointer";
    static constexpr const char* StartFlag = "start-flag";
    // ... 83 more icons

    static Image Load(const char* iconName, const Size& size);
};
```

#### Desktop Integration (`src/System/Windows/Forms/Forms.hpp/.cpp`)
- `Desktop::LoadCursorFromLibrary(path, iconName)` - Load cursor by name
- `Desktop::AddIconFromLibrary(path, iconName)` - Add desktop icon by name
- `StartMenu::LoadIcons()` - Uses SystemIcons for menu item icons

### PE Resource Directory Structure

Icon libraries use the PE (Portable Executable) format with a resource section:
```
.rsrc section
└── Root Directory
    └── Type Directory (RT_GROUP_ICON = 14)
        ├── Named Entry: bit 31 set in nameOrId → offset to UTF-16LE string
        │   └── Language Directory → Data Entry → GRPICONDIR
        └── ID Entry: bit 31 clear → numeric ID
            └── Language Directory → Data Entry → GRPICONDIR
```

Name strings are length-prefixed UTF-16LE (2-byte little-endian characters).

### Usage Example

```cpp
using namespace System::Drawing;

// Load icon by name with specific size
Image computerIcon = SystemIcons::Load(SystemIcons::Computer, Size::Icon32);

// Or load directly from any icon library
Image customIcon = Image::FromIconLibrary("myicons.icl", "my-icon-name", Size(48, 48));

// Get all available icon names
Array<String> names = Image::GetIconLibraryNames("sysicons.icl");
for (int i = 0; i < names.Length(); i++) {
    Console::WriteLine(names[i]);
}

// Desktop integration
Desktop desktop(Color::Cyan);
desktop.LoadCursorFromLibrary(SystemIcons::LibraryPath, SystemIcons::CursorPointer);
desktop.AddIconFromLibrary(SystemIcons::LibraryPath, SystemIcons::Computer);
```

---

## See Also

- `WinDOS.md` - Full API documentation (MSDN-style)
- `tests/*.cpp` - Usage examples
