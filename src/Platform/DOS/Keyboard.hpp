#ifndef PLATFORM_DOS_KEYBOARD_HPP
#define PLATFORM_DOS_KEYBOARD_HPP

namespace Platform { namespace DOS {

class Keyboard {
public:
    // Blocking read - wait for keypress and return ASCII code
    static char ReadChar();

    // Check if a key is available in the buffer
    static bool IsKeyAvailable();

    // Read key with scan code (high byte = scan code, low byte = ASCII)
    static unsigned short ReadScanCode();

    // Read key without removing from buffer (peek)
    static unsigned short PeekKey();
};

}} // namespace Platform::DOS

#endif // PLATFORM_DOS_KEYBOARD_HPP
