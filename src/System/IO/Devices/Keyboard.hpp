#ifndef SYSTEM_IO_DEVICES_KEYBOARD_HPP
#define SYSTEM_IO_DEVICES_KEYBOARD_HPP

#include "../../Types.hpp"

namespace System::IO::Devices
{

/******************************************************************************/
/*    KeyboardStatus                                                          */
/******************************************************************************/

/// @brief Represents the current state of keyboard modifier keys.
///
/// Contains the states of shift, control, alt, and lock keys.
/// Obtained via Keyboard::GetStatus().
class KeyboardStatus
{
public:
    /// @brief True if either Shift key is pressed.
    Boolean shiftPressed;

    /// @brief True if either Ctrl key is pressed.
    Boolean ctrlPressed;

    /// @brief True if either Alt key is pressed.
    Boolean altPressed;

    /// @brief True if Caps Lock is active.
    Boolean capsLock;

    /// @brief True if Num Lock is active.
    Boolean numLock;

    /// @brief True if Scroll Lock is active.
    Boolean scrollLock;

    /// @brief Constructs a default KeyboardStatus with no modifiers active.
    KeyboardStatus()
        : shiftPressed(false), ctrlPressed(false), altPressed(false)
        , capsLock(false), numLock(false), scrollLock(false)
    {
    }
};

/******************************************************************************/
/*    Keyboard                                                                */
/******************************************************************************/

/// @brief Provides keyboard input functionality using BIOS interrupts (INT 16h).
///
/// The Keyboard class is a static facade for DOS keyboard operations.
/// It provides polling-based keyboard input via IsKeyPressed(), ReadKey(),
/// and PeekKey() methods, as well as modifier key status via GetStatus().
///
/// Key codes are returned in two forms:
/// - ASCII characters (Char) for printable keys
/// - Scan codes (UInt16) for all keys including function keys and arrows
///
/// For scan codes, the high byte contains the hardware scan code and the
/// low byte contains the ASCII value (or 0 for non-printable keys).
///
/// @note This class is a static facade and cannot be instantiated.
class Keyboard
{
    Keyboard() = delete;  // Static class

    // Low-level BIOS calls - kept for PeekKey() use
    static unsigned short BiosPeekKey();

public:
    /// @brief Checks if a key is available in the keyboard buffer.
    /// @return True if a keypress is waiting to be read
    ///
    /// Performs a non-blocking check to determine if a keypress
    /// is waiting in the BIOS keyboard buffer.
    static Boolean IsKeyPressed();

    /// @brief Reads and removes a key from the keyboard buffer.
    /// @return The ASCII character of the pressed key
    ///
    /// Blocks until a key is pressed if the buffer is empty.
    /// Use IsKeyPressed() to check for available input first.
    static Char ReadKey();

    /// @brief Reads a key with its scan code, blocking until a key is pressed.
    /// @param scanCode Output parameter for the scan code (high byte)
    /// @param character Output parameter for the ASCII character (low byte)
    ///
    /// This method waits for a keypress and returns both the scan code and ASCII
    /// value. The scan code identifies the physical key pressed, while the ASCII
    /// value provides the character (0 for non-printable keys).
    ///
    /// Common scan codes:
    /// - 0x48: Up arrow
    /// - 0x50: Down arrow
    /// - 0x4B: Left arrow
    /// - 0x4D: Right arrow
    /// - 0x3B-0x44: F1-F10
    /// - 0x01: Escape
    static void ReadKey(UInt8& scanCode, Char& character);

    /// @brief Peeks at the next key without removing it from the buffer.
    /// @return The ASCII character of the next key, or '\0' if none available
    ///
    /// Does not block; returns immediately. The key remains in the buffer
    /// for subsequent ReadKey() or PeekKey() calls.
    static Char PeekKey();

    /// @brief Peeks at the next key with scan code without removing it.
    /// @param scanCode Output parameter for the scan code (high byte)
    /// @param character Output parameter for the ASCII character (low byte)
    /// @return True if a key was available, false otherwise
    ///
    /// Checks the keyboard buffer and returns the scan code and ASCII
    /// value of the next available key without consuming it. Returns false
    /// if no key is available.
    static Boolean PeekKey(UInt8& scanCode, Char& character);

    /// @brief Gets the current keyboard modifier status.
    /// @return KeyboardStatus containing modifier and lock key states
    ///
    /// Queries the BIOS for the current state of Shift, Ctrl, Alt, and
    /// lock keys (Caps Lock, Num Lock, Scroll Lock).
    static KeyboardStatus GetStatus();

    /// @brief Reads a character from keyboard input (alias for ReadKey).
    /// @return The ASCII character of the pressed key
    ///
    /// Blocks until a key is pressed if the buffer is empty.
    /// This is an alias for ReadKey() for compatibility with Console API.
    static char ReadChar()
    {
        return static_cast<char>(ReadKey());
    }

    /// @brief Checks if a key is waiting in the buffer (alias for IsKeyPressed).
    /// @return True if a keypress is waiting to be read
    ///
    /// This is an alias for IsKeyPressed() for compatibility with Console API.
    static Boolean IsKeyAvailable()
    {
        return IsKeyPressed();
    }
};

} // namespace System::IO::Devices

#endif // SYSTEM_IO_DEVICES_KEYBOARD_HPP
