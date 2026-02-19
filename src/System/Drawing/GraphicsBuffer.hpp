/// @file GraphicsBuffer.hpp
/// @brief System.Drawing.GraphicsBuffer - Framebuffer management for VGA and VBE modes.

#ifndef SYSTEM_DRAWING_GRAPHICSBUFFER_HPP
#define SYSTEM_DRAWING_GRAPHICSBUFFER_HPP

#include "../Types.hpp"
#include "Enums.hpp"
#include "Rectangle.hpp"
#include "Image.hpp"

namespace System::Drawing
{

// Forward declaration
class GraphicsBuffer;

/******************************************************************************/
/*    System::Drawing::BufferWriter (function pointer type)                   */
/******************************************************************************/

/// @brief Function pointer type for flushing graphics buffer to display.
/// @param buffer The buffer to flush to the display.
typedef void (*BufferWriter)(const GraphicsBuffer& buffer);

/******************************************************************************/
/*    System::Drawing::GraphicsBuffer                                         */
/*                                                                            */
/*    Unified graphics buffer. All rendering is done to a 32-bit Image.       */
/*    For low-color display modes, content is dithered when flushed.          */
/******************************************************************************/

/// @brief Graphics buffer for rendering and display.
///
/// Manages a 32-bit ARGB image buffer and handles flushing to the display
/// hardware. For VGA modes (4bpp/8bpp), content is dithered using Bayer
/// dithering when flushed. For VBE modes (24bpp/32bpp), content is copied
/// directly to the linear framebuffer.
///
/// @par Static Framebuffer
/// The static framebuffer is shared by all graphics operations. Create it
/// once at startup using CreateFrameBuffer() or CreateFrameBuffer32(), and
/// flush periodically using FlushFrameBuffer().
///
/// @par Example
/// @code
/// // Initialize VGA mode 12h (640x480 16-color)
/// Display::SetMode(VideoMode::VGA640x480);
/// GraphicsBuffer::CreateFrameBuffer(640, 480, 0x12);
///
/// // Draw to framebuffer
/// GraphicsBuffer* fb = GraphicsBuffer::GetFrameBuffer();
/// fb->GetImage().Clear(Color::Blue);
///
/// // Flush to screen
/// GraphicsBuffer::FlushFrameBuffer();
/// @endcode
class GraphicsBuffer
{
    BufferWriter _writer;       ///< Function to flush buffer to display
    Rectangle _bounds;          ///< Buffer dimensions
    Image _image;               ///< 32-bit ARGB image buffer
    unsigned int _lfbPitch;     ///< Bytes per scanline for LFB
    unsigned char _bpp;         ///< Display bits per pixel (4, 8, 24, or 32)
    unsigned char _videoMode;   ///< VGA mode (0x12, 0x13) or 0 for VBE

    static GraphicsBuffer* _frameBuffer;  ///< Static framebuffer instance
    static void* _lfbAddress;             ///< Mapped linear framebuffer address
    static unsigned int _lfbSize;         ///< Size of mapped LFB

    /// @brief Private constructor (use factory methods).
    GraphicsBuffer(BufferWriter writer, const Rectangle& bounds, unsigned char bpp, unsigned char videoMode);

public:
    /// @brief Destructor.
    ~GraphicsBuffer();

    /// @brief Gets the buffer dimensions.
    /// @return Rectangle describing buffer position and size.
    const Rectangle& Bounds() const
    {
        return _bounds;
    }

    /// @brief Gets the internal image buffer for direct access.
    /// @return Reference to the 32-bit ARGB image.
    Image& GetImage()
    {
        return _image;
    }

    /// @brief Gets the internal image buffer (const version).
    /// @return Const reference to the 32-bit ARGB image.
    const Image& GetImage() const
    {
        return _image;
    }

    /// @brief Gets the linear framebuffer pitch (VBE modes only).
    /// @return Bytes per scanline.
    UInt32 LfbPitch() const
    {
        return UInt32(_lfbPitch);
    }

    /// @brief Gets the display bits per pixel.
    /// @return Bits per pixel (4, 8, 24, or 32).
    UInt8 Bpp() const
    {
        return UInt8(_bpp);
    }

    /// @brief Gets the VGA video mode.
    /// @return VGA mode (0x12, 0x13) or 0 for VBE mode.
    UInt8 VideoMode() const
    {
        return UInt8(_videoMode);
    }

    /// @brief Checks if this buffer is using VBE mode.
    /// @return True for VBE modes, false for VGA modes.
    Boolean IsVbeMode() const
    {
        return Boolean(_videoMode == 0);
    }

    /// @brief Marks the buffer as needing flush.
    void Invalidate();

    /// @brief Creates the static framebuffer for VGA modes.
    /// @param width Buffer width in pixels.
    /// @param height Buffer height in pixels.
    /// @param videoMode VGA mode number (0x12 for 16-color, 0x13 for 256-color).
    ///
    /// Call this after setting the video mode with Display::SetMode().
    static void CreateFrameBuffer(Int32 width, Int32 height, UInt8 videoMode);

    /// @brief Creates the static framebuffer for VBE modes.
    /// @param width Buffer width in pixels.
    /// @param height Buffer height in pixels.
    /// @param vbeMode VBE mode number.
    /// @param lfbAddr Physical address of the linear framebuffer.
    /// @param pitch Bytes per scanline in the LFB.
    /// @param bpp Bits per pixel (24 or 32).
    ///
    /// Call this after setting the video mode with Display::SetMode() for VBE modes.
    static void CreateFrameBuffer32(Int32 width, Int32 height, UInt16 vbeMode,
                                    void* lfbAddr, UInt32 pitch, UInt8 bpp);

    /// @brief Destroys the static framebuffer and frees resources.
    static void DestroyFrameBuffer();

    /// @brief Flushes the static framebuffer to the display.
    ///
    /// For VGA modes, applies Bayer dithering and writes to video memory.
    /// For VBE modes, copies directly to the linear framebuffer.
    static void FlushFrameBuffer();

    /// @brief Gets the static framebuffer instance.
    /// @return Pointer to framebuffer, or nullptr if not created.
    static GraphicsBuffer* GetFrameBuffer();

    /// @brief Creates a graphics buffer for a specific region.
    /// @param mode Buffering mode (Single or Double).
    /// @param bounds Buffer dimensions.
    /// @return Pointer to new buffer (caller owns).
    static GraphicsBuffer* Create(BufferMode mode, const Rectangle& bounds);

    /// @brief Gets the mapped linear framebuffer address.
    /// @return LFB address for VBE modes, nullptr otherwise.
    static void* GetLFBAddress()
    {
        return _lfbAddress;
    }
};

} // namespace System::Drawing

#endif // SYSTEM_DRAWING_GRAPHICSBUFFER_HPP
