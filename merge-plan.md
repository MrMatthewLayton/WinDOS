# Merge Plan: Platform/DOS + System/Devices Reorganization

## Executive Summary

Reorganize the confusing multi-layer device code into a clean, coherent structure under `System/IO/Devices`. Use proper C++ encapsulation (private methods, .cpp-only details) rather than fake "Internal" namespaces.

Since this is DOS-only and will remain DOS-only, we don't need platform abstraction layers - just clean, well-encapsulated classes.

---

## Current Structure (Confusing)

```
src/
├── Platform/
│   └── DOS/
│       ├── Graphics.hpp/.cpp    # VGA/VBE mode setting, LFB mapping
│       ├── Video.hpp/.cpp       # Text-mode cursor, scrolling
│       ├── Keyboard.hpp/.cpp    # Raw INT 16h keyboard input
│       ├── Mouse.hpp/.cpp       # Raw INT 33h mouse driver
│       └── System.hpp/.cpp      # DOS system calls (INT 21h)
└── System/
    ├── Devices/
    │   └── Devices.hpp/.cpp     # Display, Mouse, Keyboard wrappers
    └── IO/
        └── IO.hpp/.cpp          # File class
```

**Problems:**
1. `Graphics` vs `Video` vs `Display` - three names for related concepts
2. `Platform/DOS/Keyboard` vs `System/Devices/Keyboard` - same name, different layers
3. `Devices` separate from `IO`, but devices are I/O
4. `IO.hpp` contains only `File` class - should be `File.hpp`
5. Two-layer abstraction adds complexity without benefit (DOS-only project)

---

## Proposed Structure

```
src/
└── System/
    └── IO/
        ├── Devices/
        │   ├── Display.hpp/.cpp     # All display/graphics functionality
        │   ├── Keyboard.hpp/.cpp    # All keyboard functionality
        │   └── Mouse.hpp/.cpp       # All mouse functionality
        ├── File.hpp/.cpp            # File operations (renamed from IO.hpp)
        └── Environment.hpp/.cpp     # System environment (from DOSSystem)
```

**Benefits:**
1. Devices under IO - semantically correct (devices ARE I/O)
2. One file per device - clear ownership
3. Single layer - no confusing Platform/System split
4. Proper C++ encapsulation - low-level code is truly private

---

## Encapsulation Strategy

**Instead of fake "Internal" namespaces, use proper C++ encapsulation:**

### Option A: Private Static Methods (Preferred)

```cpp
// Display.hpp
namespace System::IO::Devices
{
    class Display
    {
    public:
        // Public API - what users see
        static Display GetCurrent();
        static void SetMode(DisplayMode mode);
        static void FadeIn(Int32 milliseconds);
        static Boolean IsVbeAvailable();

    private:
        // Low-level BIOS code - truly hidden
        static bool DetectVBE();
        static bool SetVBEMode(unsigned short mode);
        static void SetVGAMode(unsigned char mode);
        static void WaitForVSync();
        static void SetPaletteColor(int index, int r, int g, int b);

        // Private state
        static Display _current;
        static bool _vbeAvailable;
        static unsigned char _originalPalette[256][3];
    };
}
```

### Option B: Implementation in .cpp Only

For truly internal structs/helpers that don't need to be in the header at all:

```cpp
// Display.cpp
#include "Display.hpp"

namespace System::IO::Devices
{
    // Anonymous namespace - file-local, invisible to other files
    namespace
    {
        // VBE BIOS structures - only needed in implementation
        struct VbeInfoBlock { ... };
        struct VbeModeInfoBlock { ... };

        // Helper functions - only needed in implementation
        bool CallVBEBios(int function, __dpmi_regs* regs) { ... }
    }

    // Public method implementations use private helpers
    bool Display::IsVbeAvailable()
    {
        // Uses file-local helpers
        VbeInfoBlock info;
        return CallVBEBios(0x4F00, ...);
    }
}
```

### Combined Approach

- **Header (.hpp):** Public API + private method declarations (for methods called from multiple public methods)
- **Implementation (.cpp):** Anonymous namespace for structs, constants, one-off helpers

This gives us:
- True encapsulation (not just naming convention)
- Clean public API in headers
- Implementation details invisible to users
- No "Internal" namespace smell

---

## System::Environment Class

The `Platform/DOS/System.hpp` contains `DOSSystem` with these functions:
- `Exit(int code)` - terminate program via INT 21h
- `WriteString(const char*)` - write to stdout via INT 21h
- `ReadLine(char*, int)` - read from stdin
- `GetVersion()` - get DOS version

**Proposed `System::IO::Environment` class (mirrors .NET's System.Environment):**

```cpp
// Environment.hpp
namespace System::IO
{
    /// @brief Provides information about and access to the current environment.
    class Environment
    {
    public:
        /// @brief Terminates the process with the specified exit code.
        /// @param exitCode The exit code to return to the operating system.
        static void Exit(Int32 exitCode);

        /// @brief Gets the command line for the process.
        /// @return The command line string.
        static String GetCommandLine();

        /// @brief Gets the value of an environment variable.
        /// @param name The name of the environment variable.
        /// @return The value, or empty string if not found.
        static String GetEnvironmentVariable(const String& name);

        /// @brief Gets the current working directory.
        /// @return The full path of the current directory.
        static String GetCurrentDirectory();

        /// @brief Sets the current working directory.
        /// @param path The new current directory path.
        static void SetCurrentDirectory(const String& path);

        /// @brief Gets the DOS version.
        /// @return The major and minor version as a formatted string (e.g., "7.10").
        static String GetOSVersion();

        /// @brief Gets the newline string for this environment.
        /// @return "\r\n" for DOS.
        static const char* NewLine();

    private:
        Environment() = delete;  // Static class
    };
}
```

**Benefits over inlining into Console:**
- Reusable - any code can call `Environment::Exit()`
- Matches .NET API - familiar to .NET developers
- Single responsibility - Console handles I/O formatting, Environment handles system interaction
- `GetCurrentDirectory()`, `GetEnvironmentVariable()` are useful beyond Console

**What moves where:**
| DOSSystem Function | New Location |
|-------------------|--------------|
| `Exit()` | `Environment::Exit()` |
| `WriteString()` | Inline in `Console.cpp` (internal detail) |
| `ReadLine()` | Inline in `Console.cpp` (internal detail) |
| `GetVersion()` | `Environment::GetOSVersion()` |

---

## Detailed Class Designs

### Display Class

**Merges:** `Platform/DOS/Graphics` + `Platform/DOS/Video` + `System/Devices/Display`

```cpp
// Display.hpp
namespace System::IO::Devices
{
    enum class DisplayMode : UInt16
    {
        Text80x25 = 0x03,
        VGA640x480x4 = 0x12,
        SVGA800x600x8 = 0x103,
        // ... etc
    };

    class Display
    {
    public:
        // Current display info
        static Display GetCurrent();
        UInt16 Width() const;
        UInt16 Height() const;
        UInt8 BitsPerPixel() const;
        Boolean IsVbeMode() const;

        // Mode setting
        static Boolean IsVbeAvailable();
        static Display SetMode(DisplayMode mode);
        static Display SetVbeMode(UInt16 width, UInt16 height, UInt8 bpp);

        // Visual effects
        static void FadeIn(Int32 milliseconds);
        static void FadeOut(Int32 milliseconds);

        // VSync
        static void WaitForVSync();

        // Text mode cursor (from Video)
        static void SetCursorPosition(Int32 row, Int32 col);
        static void GetCursorPosition(Int32& row, Int32& col);
        static void SetCursorVisible(Boolean visible);

        // Palette (VGA modes)
        static void SetPaletteColor(UInt8 index, UInt8 r, UInt8 g, UInt8 b);
        static void GetPaletteColor(UInt8 index, UInt8& r, UInt8& g, UInt8& b);

        // Gamma (VBE 3.0)
        static Boolean IsGammaSupported();
        static void SetGammaScale(Float32 scale);

        // LFB access (for Drawing layer)
        static int GetLfbSelector();
        static UInt32 GetLfbPitch();

    private:
        // Low-level BIOS operations (truly private)
        static bool DetectVBE();
        static bool GetVBEModeInfo(unsigned short mode, void* info);
        static bool SetVBEModeInternal(unsigned short mode);
        static void SetVGAModeInternal(unsigned char mode);
        static bool MapLFB(unsigned int physAddr, unsigned int size);
        static void UnmapLFB();

        // State
        static Display _current;
        static bool _vbeDetected;
        static bool _vbeAvailable;
        static int _lfbSelector;
        static unsigned char _savedPalette[256][3];
        static unsigned char _savedGamma[768];

        // Instance data
        UInt16 _width;
        UInt16 _height;
        UInt8 _bpp;
        UInt16 _mode;
        bool _isVbe;
    };
}
```

### Keyboard Class

**Merges:** `Platform/DOS/Keyboard` + `System/Devices/Keyboard`

```cpp
// Keyboard.hpp
namespace System::IO::Devices
{
    class KeyboardStatus
    {
    public:
        Boolean ShiftPressed() const;
        Boolean CtrlPressed() const;
        Boolean AltPressed() const;
        Boolean CapsLockActive() const;
        Boolean NumLockActive() const;
        Boolean ScrollLockActive() const;

    private:
        UInt8 _flags;
        friend class Keyboard;
    };

    class Keyboard
    {
    public:
        /// @brief Checks if a key is available in the buffer.
        static Boolean IsKeyAvailable();

        /// @brief Reads a character, blocking until available.
        static Char ReadKey();

        /// @brief Reads raw scan code and character.
        static void ReadKey(UInt8& scanCode, Char& character);

        /// @brief Peeks at the next key without removing it.
        static Boolean PeekKey(UInt8& scanCode, Char& character);

        /// @brief Gets current modifier key status.
        static KeyboardStatus GetStatus();

    private:
        // Low-level BIOS calls (truly private)
        static unsigned short BiosReadKey();
        static bool BiosIsKeyAvailable();
        static unsigned short BiosPeekKey();
        static unsigned char BiosGetShiftFlags();
    };
}
```

### Mouse Class

**Merges:** `Platform/DOS/Mouse` + `System/Devices/Mouse`

```cpp
// Mouse.hpp
namespace System::IO::Devices
{
    class MouseStatus
    {
    public:
        Int32 X() const;
        Int32 Y() const;
        Boolean LeftButton() const;
        Boolean RightButton() const;
        Boolean MiddleButton() const;

    private:
        Int32 _x, _y;
        Boolean _left, _right, _middle;
        friend class Mouse;
    };

    class Mouse
    {
    public:
        /// @brief Initializes the mouse driver.
        /// @return True if mouse is available.
        static Boolean Initialize();

        /// @brief Checks if mouse driver is available.
        static Boolean IsAvailable();

        /// @brief Gets current mouse position and button state.
        static MouseStatus GetStatus();

        /// @brief Sets the mouse cursor position.
        static void SetPosition(Int32 x, Int32 y);

        /// @brief Sets mouse movement bounds.
        static void SetBounds(Int32 minX, Int32 minY, Int32 maxX, Int32 maxY);

        /// @brief Shows the hardware mouse cursor.
        static void ShowCursor();

        /// @brief Hides the hardware mouse cursor.
        static void HideCursor();

        /// @brief Sets mouse sensitivity.
        static void SetSensitivity(Int32 horizontal, Int32 vertical, Int32 threshold);

    private:
        // Low-level INT 33h calls (truly private)
        static bool BiosInitialize();
        static void BiosGetState(int& x, int& y, int& buttons);
        static void BiosSetPosition(int x, int y);
        static void BiosSetBounds(int minX, int minY, int maxX, int maxY);
        static void BiosShowCursor();
        static void BiosHideCursor();

        // State
        static bool _initialized;
        static bool _available;
    };
}
```

---

## Migration Steps

### Phase 1: Create Structure
```bash
mkdir -p src/System/IO/Devices
```

### Phase 2: Implement Mouse.hpp/.cpp
- Merge `Platform/DOS/Mouse` + `System/Devices::Mouse`
- Private BIOS methods, public wrapper API
- Test with forms_demo

### Phase 3: Implement Keyboard.hpp/.cpp
- Merge `Platform/DOS/Keyboard` + `System/Devices::Keyboard`
- Private BIOS methods, public wrapper API
- Test with Console and forms

### Phase 4: Implement Display.hpp/.cpp
- Merge `Platform/DOS/Graphics` + `Platform/DOS/Video` + `System/Devices::Display`
- Most complex - VGA, VBE, text cursor, palette, gamma, LFB
- Private BIOS methods, public wrapper API
- Test with all demos

### Phase 5: Implement Environment.hpp/.cpp
- Move `DOSSystem::Exit()` → `Environment::Exit()`
- Move `DOSSystem::GetVersion()` → `Environment::GetOSVersion()`
- Add `GetCurrentDirectory()`, `GetEnvironmentVariable()`
- Inline `WriteString`/`ReadLine` into Console.cpp

### Phase 6: Rename IO.hpp → File.hpp
- Simple rename
- Update includes

### Phase 7: Create Umbrella Header
```cpp
// src/System/IO/IO.hpp
#ifndef SYSTEM_IO_HPP
#define SYSTEM_IO_HPP

#include "File.hpp"
#include "Environment.hpp"
#include "Devices/Display.hpp"
#include "Devices/Keyboard.hpp"
#include "Devices/Mouse.hpp"

#endif
```

### Phase 8: Update All Includes
- `System/Drawing` → uses `System/IO/Devices/Display`
- `System/Windows/Forms` → uses all devices
- `System/Console` → uses `Keyboard`, `Environment`
- `rtcorlib.hpp` → updated master include

### Phase 9: Update Makefile
- New source file paths
- Remove old Platform/DOS files

### Phase 10: Delete Old Files
- Remove `src/Platform/` directory
- Remove `src/System/Devices/` directory

### Phase 11: Test
- Build all targets
- Run test suite
- Run forms_demo in QEMU

---

## Files Summary

### Create
| File | Content |
|------|---------|
| `src/System/IO/Devices/Display.hpp` | Display class declaration |
| `src/System/IO/Devices/Display.cpp` | Display implementation |
| `src/System/IO/Devices/Keyboard.hpp` | Keyboard class declaration |
| `src/System/IO/Devices/Keyboard.cpp` | Keyboard implementation |
| `src/System/IO/Devices/Mouse.hpp` | Mouse class declaration |
| `src/System/IO/Devices/Mouse.cpp` | Mouse implementation |
| `src/System/IO/Environment.hpp` | Environment class declaration |
| `src/System/IO/Environment.cpp` | Environment implementation |
| `src/System/IO/File.hpp` | Renamed from IO.hpp |
| `src/System/IO/File.cpp` | Renamed from IO.cpp |

### Delete
| File | Reason |
|------|--------|
| `src/Platform/DOS/Graphics.hpp/.cpp` | Merged into Display |
| `src/Platform/DOS/Video.hpp/.cpp` | Merged into Display |
| `src/Platform/DOS/Keyboard.hpp/.cpp` | Merged into Keyboard |
| `src/Platform/DOS/Mouse.hpp/.cpp` | Merged into Mouse |
| `src/Platform/DOS/System.hpp/.cpp` | Merged into Environment + Console |
| `src/Platform/DOS/Platform.hpp` | No longer needed |
| `src/System/Devices/Devices.hpp/.cpp` | Merged into individual device files |
| `src/System/IO/IO.hpp/.cpp` | Renamed to File.hpp/.cpp |

### Update
| File | Changes |
|------|---------|
| `src/System/Drawing/Drawing.hpp` | New Display include |
| `src/System/Drawing/Drawing.cpp` | Use `System::IO::Devices::Display` |
| `src/System/Windows/Forms/Forms.hpp` | New device includes |
| `src/System/Windows/Forms/Forms.cpp` | Use new device namespaces |
| `src/System/Console.hpp` | New includes |
| `src/System/Console.cpp` | Use Keyboard, inline DOS writes |
| `src/rtcorlib.hpp` | Updated includes |
| `Makefile` | New source paths |
| `tests/*.cpp` | Updated includes where needed |

---

## Estimated Effort

| Task | Effort |
|------|--------|
| Mouse.hpp/.cpp | 45 min |
| Keyboard.hpp/.cpp | 45 min |
| Display.hpp/.cpp | 2.5 hours |
| Environment.hpp/.cpp | 30 min |
| File rename | 10 min |
| Update includes | 45 min |
| Update Makefile | 15 min |
| Testing & fixes | 1 hour |
| **Total** | **~6.5 hours** |

---

## Decision Required

Please confirm:
1. **Proceed with this plan?**
2. **Environment class:** Include it as proposed, or inline DOSSystem into Console?
3. **Any other concerns?**
