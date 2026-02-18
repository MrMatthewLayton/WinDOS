#ifndef PLATFORM_DOS_KEYBOARD_HPP
#define PLATFORM_DOS_KEYBOARD_HPP

namespace Platform::DOS
{

/// @brief Low-level keyboard input interface for DOS using BIOS interrupts.
///
/// The Keyboard class provides static methods for reading keyboard input
/// in DOS protected mode. It uses INT 16h BIOS services for keyboard access,
/// supporting both blocking and non-blocking input operations.
///
/// Key codes are returned in two forms:
/// - ASCII characters (8-bit) for printable keys
/// - Scan codes (16-bit) for all keys including function keys and arrows
///
/// For scan codes, the high byte contains the hardware scan code and the
/// low byte contains the ASCII value (or 0 for non-printable keys).
///
/// @note This class is a static facade and cannot be instantiated.
/// @see System::Devices::Keyboard for higher-level keyboard input
class Keyboard
{
public:
    /// @brief Reads a single character from the keyboard, blocking until a key is pressed.
    ///
    /// This method waits for a keypress and returns the ASCII code of the pressed key.
    /// For non-ASCII keys (function keys, arrows, etc.), returns 0 or the extended
    /// key code. Uses INT 16h AH=00h BIOS service.
    ///
    /// @return The ASCII code of the pressed key (0-255).
    static char ReadChar();

    /// @brief Checks if a key is available in the keyboard buffer.
    ///
    /// This method performs a non-blocking check to determine if a keypress
    /// is waiting in the BIOS keyboard buffer. Uses INT 16h AH=01h BIOS service.
    ///
    /// @return true if a key is available to be read, false otherwise.
    static bool IsKeyAvailable();

    /// @brief Reads a key with its scan code, blocking until a key is pressed.
    ///
    /// This method waits for a keypress and returns both the scan code and ASCII
    /// value packed into a 16-bit word. Uses INT 16h AH=00h BIOS service.
    ///
    /// The return value format is:
    /// - High byte (bits 8-15): Hardware scan code
    /// - Low byte (bits 0-7): ASCII character code (0 for non-printable keys)
    ///
    /// Common scan codes:
    /// - 0x48: Up arrow
    /// - 0x50: Down arrow
    /// - 0x4B: Left arrow
    /// - 0x4D: Right arrow
    /// - 0x3B-0x44: F1-F10
    /// - 0x01: Escape
    ///
    /// @return 16-bit value with scan code in high byte and ASCII in low byte.
    static unsigned short ReadScanCode();

    /// @brief Peeks at the next key in the buffer without removing it.
    ///
    /// This method checks the keyboard buffer and returns the scan code and ASCII
    /// value of the next available key without consuming it. If no key is available,
    /// the behavior depends on the BIOS implementation. Uses INT 16h AH=01h BIOS service.
    ///
    /// @return 16-bit value with scan code in high byte and ASCII in low byte,
    ///         or 0 if no key is available (implementation-dependent).
    static unsigned short PeekKey();
};

} // namespace Platform::DOS

#endif // PLATFORM_DOS_KEYBOARD_HPP
