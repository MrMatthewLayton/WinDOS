#include "Display.hpp"
#include "../../Drawing/Drawing.hpp"
#include <dos.h>
#include <go32.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <pc.h>
#include <sys/movedata.h>
#include <cstring>
#include <conio.h>
#include <cstdlib>

namespace System::IO::Devices
{

/******************************************************************************/
/*    Internal VBE Structures and Constants                                  */
/******************************************************************************/

namespace
{
    // VBE structures - internal to this compilation unit
    #pragma pack(push, 1)

    struct VbeInfoBlock
    {
        char signature[4];        // "VESA" or "VBE2"
        unsigned short version;   // VBE version (BCD: 0x0200 = 2.0)
        unsigned int oemStringPtr;
        unsigned int capabilities;
        unsigned int videoModePtr;
        unsigned short totalMemory;  // In 64KB blocks
        unsigned short oemSoftwareRev;
        unsigned int oemVendorNamePtr;
        unsigned int oemProductNamePtr;
        unsigned int oemProductRevPtr;
        unsigned char reserved[222];
        unsigned char oemData[256];
    };

    struct VbeModeInfoBlock
    {
        unsigned short modeAttributes;
        unsigned char winAAttributes;
        unsigned char winBAttributes;
        unsigned short winGranularity;
        unsigned short winSize;
        unsigned short winASegment;
        unsigned short winBSegment;
        unsigned int winFuncPtr;
        unsigned short bytesPerScanLine;
        unsigned short xResolution;
        unsigned short yResolution;
        unsigned char xCharSize;
        unsigned char yCharSize;
        unsigned char numberOfPlanes;
        unsigned char bitsPerPixel;
        unsigned char numberOfBanks;
        unsigned char memoryModel;
        unsigned char bankSize;
        unsigned char numberOfImagePages;
        unsigned char reserved1;
        unsigned char redMaskSize;
        unsigned char redFieldPosition;
        unsigned char greenMaskSize;
        unsigned char greenFieldPosition;
        unsigned char blueMaskSize;
        unsigned char blueFieldPosition;
        unsigned char rsvdMaskSize;
        unsigned char rsvdFieldPosition;
        unsigned char directColorModeInfo;
        unsigned int physBasePtr;      // LFB physical address
        unsigned int reserved2;
        unsigned short reserved3;
        unsigned char reserved4[206];
    };

    struct VbeSurface
    {
        int selector;              // LDT selector for LFB access
        unsigned long linearAddr;  // Linear address of LFB
        unsigned long size;        // Size of framebuffer in bytes
        unsigned int pitch;        // Bytes per scanline
        unsigned short width;
        unsigned short height;
        unsigned char bpp;
        bool valid;
    };

    #pragma pack(pop)

    // VBE constants
    const unsigned short VBE_SUCCESS = 0x004F;
    const unsigned short VBE_MODE_LFB = 0x4000;  // Bit 14: enable LFB
    const unsigned short VBE_ATTR_SUPPORTED = 0x0001;
    const unsigned short VBE_ATTR_LFB_AVAIL = 0x0080;

    // VBE 3.0 gamma function codes
    const unsigned char VBE_GAMMA_SET = 0x01;
    const unsigned char VBE_GAMMA_GET = 0x02;
    const int VBE_GAMMA_TABLE_SIZE = 256 * 3;  // R, G, B channels

    // Static globals for VBE state
    int g_lfbSelector = 0;
    int g_gammaSupported = -1;  // -1 = not checked, 0 = no, 1 = yes
    VbeSurface g_vbeSurface = {0, 0, 0, 0, 0, 0, 0, false};

} // anonymous namespace

/******************************************************************************/
/*    Static Member Initialization                                           */
/******************************************************************************/

Display Display::_current = Display::TextMode;
Boolean Display::_vbeAvailable = Boolean(false);
Boolean Display::_vbeChecked = Boolean(false);
void* Display::_mappedLfb = nullptr;
unsigned char Display::_originalPalette[PALETTE_SIZE][3] = {{0}};
Boolean Display::_paletteStashed = Boolean(false);

// VBE 3.0 gamma ramp support
unsigned char Display::_originalGamma[GAMMA_TABLE_SIZE] = {0};
Boolean Display::_gammaStashed = Boolean(false);
Boolean Display::_gammaSupported = Boolean(false);
Boolean Display::_gammaChecked = Boolean(false);

/******************************************************************************/
/*    Constructors and Assignment                                            */
/******************************************************************************/

Display::Display(unsigned char mode, unsigned char bpp, unsigned short width, unsigned short height)
    : _mode(mode), _bitsPerPixel(bpp), _width(width), _height(height)
    , _vbeMode(0), _lfbPhysAddr(0), _lfbPitch(0)
{
}

Display::Display(unsigned short vbeMode, unsigned char bpp, unsigned short width, unsigned short height,
                 unsigned int lfbAddr, unsigned int pitch)
    : _mode(0), _bitsPerPixel(bpp), _width(width), _height(height)
    , _vbeMode(vbeMode), _lfbPhysAddr(lfbAddr), _lfbPitch(pitch)
{
}

Display::Display(const Display& other)
    : _mode(other._mode), _bitsPerPixel(other._bitsPerPixel)
    , _width(other._width), _height(other._height)
    , _vbeMode(other._vbeMode), _lfbPhysAddr(other._lfbPhysAddr)
    , _lfbPitch(other._lfbPitch)
{
}

Display& Display::operator=(const Display& other)
{
    _mode = other._mode;
    _bitsPerPixel = other._bitsPerPixel;
    _width = other._width;
    _height = other._height;
    _vbeMode = other._vbeMode;
    _lfbPhysAddr = other._lfbPhysAddr;
    _lfbPitch = other._lfbPitch;
    return *this;
}

/******************************************************************************/
/*    Private BIOS Methods - Graphics Operations                             */
/******************************************************************************/

void Display::BiosSetVideoMode(unsigned char mode)
{
    __dpmi_regs regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    __dpmi_int(0x10, &regs);
}

void Display::BiosWaitForVSync()
{
    // Wait until not in vertical retrace
    while ((inportb(0x3DA) & 0x08) != 0);
    // Wait until in vertical retrace
    while ((inportb(0x3DA) & 0x08) == 0);
}

void Display::BiosSelectPlane(int plane)
{
    outportb(0x3C4, 0x02);  // Map Mask Register
    outportb(0x3C5, 1 << plane);
}

void Display::BiosCopyToVGA(const void* data, unsigned int offset, unsigned int length)
{
    // VGA memory starts at 0xA0000
    dosmemput(data, length, 0xA0000 + offset);
}

void Display::BiosOutPort(unsigned short port, unsigned char value)
{
    outportb(port, value);
}

unsigned char Display::BiosInPort(unsigned short port)
{
    return inportb(port);
}

bool Display::BiosDetectVBE(void* info)
{
    if (!info)
    {
        return false;
    }

    VbeInfoBlock* vbeInfo = static_cast<VbeInfoBlock*>(info);

    // Use DJGPP's transfer buffer (__tb is guaranteed to be in conventional memory)
    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    // Write "VBE2" signature to request VBE 2.0+ info
    dosmemput("VBE2", 4, tbAddr);

    // Call VBE function 0x4F00 - Get Controller Info
    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F00;
    regs.x.es = tbSeg;
    regs.x.di = tbOff;
    __dpmi_int(0x10, &regs);

    if (regs.x.ax != VBE_SUCCESS)
    {
        return false;
    }

    // Copy VBE info from transfer buffer
    dosmemget(tbAddr, sizeof(VbeInfoBlock), vbeInfo);

    // Verify VESA signature
    if (vbeInfo->signature[0] != 'V' || vbeInfo->signature[1] != 'E' ||
        vbeInfo->signature[2] != 'S' || vbeInfo->signature[3] != 'A')
    {
        return false;
    }

    return true;
}

bool Display::BiosGetVBEModeInfo(unsigned short mode, void* info)
{
    if (!info)
    {
        return false;
    }

    VbeModeInfoBlock* modeInfo = static_cast<VbeModeInfoBlock*>(info);

    // Use transfer buffer
    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    // Call VBE function 0x4F01 - Get Mode Info
    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F01;
    regs.x.cx = mode;
    regs.x.es = tbSeg;
    regs.x.di = tbOff;
    __dpmi_int(0x10, &regs);

    if (regs.x.ax != VBE_SUCCESS)
    {
        return false;
    }

    // Copy mode info from transfer buffer
    dosmemget(tbAddr, sizeof(VbeModeInfoBlock), modeInfo);
    return true;
}

bool Display::BiosSetVBEMode(unsigned short mode, void* surface)
{
    if (!surface)
    {
        return false;
    }

    VbeSurface* vbeSurface = static_cast<VbeSurface*>(surface);

    // Initialize surface
    vbeSurface->valid = false;
    vbeSurface->selector = 0;
    vbeSurface->linearAddr = 0;

    // Get mode info first
    VbeModeInfoBlock modeInfo;
    std::memset(&modeInfo, 0, sizeof(modeInfo));
    if (!BiosGetVBEModeInfo(mode, &modeInfo))
    {
        return false;
    }

    // Check that LFB is available
    if (!(modeInfo.modeAttributes & VBE_ATTR_LFB_AVAIL))
    {
        return false;
    }

    // Set the video mode with LFB enabled
    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F02;
    regs.x.bx = mode | VBE_MODE_LFB;
    __dpmi_int(0x10, &regs);

    if (regs.x.ax != VBE_SUCCESS)
    {
        return false;
    }

    // Map the physical framebuffer to linear address space
    unsigned long lfbSize = static_cast<unsigned long>(modeInfo.yResolution) *
                            static_cast<unsigned long>(modeInfo.bytesPerScanLine);

    __dpmi_meminfo memInfo;
    memInfo.address = modeInfo.physBasePtr;
    memInfo.size = lfbSize;

    if (__dpmi_physical_address_mapping(&memInfo) != 0)
    {
        // Failed to map physical memory
        BiosSetVideoMode(0x03);  // Revert to text mode
        return false;
    }

    // Lock the linear region
    __dpmi_lock_linear_region(&memInfo);

    // Allocate an LDT descriptor for accessing the LFB
    int selector = __dpmi_allocate_ldt_descriptors(1);
    if (selector < 0)
    {
        __dpmi_free_physical_address_mapping(&memInfo);
        BiosSetVideoMode(0x03);
        return false;
    }

    // Set the descriptor base address and limit
    __dpmi_set_segment_base_address(selector, memInfo.address);
    __dpmi_set_segment_limit(selector, lfbSize - 1);

    // Fill in the surface info
    vbeSurface->selector = selector;
    vbeSurface->linearAddr = memInfo.address;
    vbeSurface->size = lfbSize;
    vbeSurface->pitch = modeInfo.bytesPerScanLine;
    vbeSurface->width = modeInfo.xResolution;
    vbeSurface->height = modeInfo.yResolution;
    vbeSurface->bpp = modeInfo.bitsPerPixel;
    vbeSurface->valid = true;

    // Store global selector for GetLfbSelector()
    g_lfbSelector = selector;

    return true;
}

void Display::BiosCleanupVBE(void* surface)
{
    if (!surface)
    {
        return;
    }

    VbeSurface* vbeSurface = static_cast<VbeSurface*>(surface);

    if (!vbeSurface->valid)
    {
        return;
    }

    // Free the LDT descriptor
    if (vbeSurface->selector > 0)
    {
        __dpmi_free_ldt_descriptor(vbeSurface->selector);
    }

    // Unmap the physical memory
    if (vbeSurface->linearAddr != 0)
    {
        __dpmi_meminfo memInfo;
        memInfo.address = vbeSurface->linearAddr;
        memInfo.size = vbeSurface->size;
        __dpmi_free_physical_address_mapping(&memInfo);
    }

    vbeSurface->valid = false;
    vbeSurface->selector = 0;
    vbeSurface->linearAddr = 0;
    g_lfbSelector = 0;
}

/******************************************************************************/
/*    Private BIOS Methods - VBE 3.0 Gamma Ramp                              */
/******************************************************************************/

bool Display::BiosIsGammaSupported()
{
    if (g_gammaSupported >= 0)
    {
        return g_gammaSupported == 1;
    }

    // First check VBE version (need 3.0+)
    VbeInfoBlock vbeInfo;
    if (!BiosDetectVBE(&vbeInfo))
    {
        g_gammaSupported = 0;
        return false;
    }

    // VBE version is BCD: 0x0300 = 3.0
    if (vbeInfo.version < 0x0300)
    {
        g_gammaSupported = 0;
        return false;
    }

    // Try to get the current gamma table as a probe
    // VBE 3.0 function 4F15h, BL=02h (Get Gamma Table)
    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F15;
    regs.h.bl = VBE_GAMMA_GET;
    regs.x.cx = 256;  // Number of entries per channel
    regs.x.es = tbSeg;
    regs.x.di = tbOff;
    __dpmi_int(0x10, &regs);

    // Check if function is supported
    g_gammaSupported = (regs.x.ax == VBE_SUCCESS) ? 1 : 0;
    return g_gammaSupported == 1;
}

bool Display::BiosSetGammaTable(const unsigned char* gammaTable)
{
    if (!gammaTable)
    {
        return false;
    }

    // Use transfer buffer to pass gamma table to BIOS
    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    // Copy gamma table to transfer buffer
    // Format: 256 R values, 256 G values, 256 B values (768 bytes total)
    dosmemput(gammaTable, VBE_GAMMA_TABLE_SIZE, tbAddr);

    // VBE 3.0 function 4F15h, BL=01h (Set Gamma Table)
    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F15;
    regs.h.bl = VBE_GAMMA_SET;
    regs.x.cx = 256;  // Number of entries per channel
    regs.x.es = tbSeg;
    regs.x.di = tbOff;
    __dpmi_int(0x10, &regs);

    return regs.x.ax == VBE_SUCCESS;
}

bool Display::BiosGetGammaTable(unsigned char* gammaTable)
{
    if (!gammaTable)
    {
        return false;
    }

    // Use transfer buffer
    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    // VBE 3.0 function 4F15h, BL=02h (Get Gamma Table)
    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F15;
    regs.h.bl = VBE_GAMMA_GET;
    regs.x.cx = 256;  // Number of entries per channel
    regs.x.es = tbSeg;
    regs.x.di = tbOff;
    __dpmi_int(0x10, &regs);

    if (regs.x.ax != VBE_SUCCESS)
    {
        return false;
    }

    // Copy gamma table from transfer buffer
    dosmemget(tbAddr, VBE_GAMMA_TABLE_SIZE, gammaTable);
    return true;
}

/******************************************************************************/
/*    Private BIOS Methods - Text Mode Operations                            */
/******************************************************************************/

void Display::BiosSetCursorPosition(int row, int col)
{
    __dpmi_regs regs;
    regs.h.ah = 0x02;       // Set cursor position
    regs.h.bh = 0;          // Page number
    regs.h.dh = row;        // Row
    regs.h.dl = col;        // Column
    __dpmi_int(0x10, &regs);
}

void Display::BiosGetCursorPosition(int& row, int& col)
{
    __dpmi_regs regs;
    regs.h.ah = 0x03;       // Get cursor position
    regs.h.bh = 0;          // Page number
    __dpmi_int(0x10, &regs);
    row = regs.h.dh;
    col = regs.h.dl;
}

void Display::BiosWriteChar(char c, unsigned char attr)
{
    __dpmi_regs regs;
    regs.h.ah = 0x09;       // Write character and attribute
    regs.h.al = c;          // Character
    regs.h.bh = 0;          // Page number
    regs.h.bl = attr;       // Attribute
    regs.x.cx = 1;          // Count
    __dpmi_int(0x10, &regs);
}

void Display::BiosScrollUp(int lines, unsigned char attr, int top, int left, int bottom, int right)
{
    __dpmi_regs regs;
    regs.h.ah = 0x06;       // Scroll up
    regs.h.al = lines;      // Lines to scroll (0 = clear)
    regs.h.bh = attr;       // Attribute for blank lines
    regs.h.ch = top;        // Top row
    regs.h.cl = left;       // Left column
    regs.h.dh = bottom;     // Bottom row
    regs.h.dl = right;      // Right column
    __dpmi_int(0x10, &regs);
}

void Display::BiosGetScreenSize(int& rows, int& cols)
{
    // Read from BIOS data area
    // 0x40:0x4A = number of columns
    // 0x40:0x84 = number of rows - 1
    cols = _farpeekb(_dos_ds, 0x44A);
    rows = _farpeekb(_dos_ds, 0x484) + 1;

    // Default to 80x25 if values seem wrong
    if (cols == 0)
    {
        cols = 80;
    }
    if (rows == 0)
    {
        rows = 25;
    }
}

/******************************************************************************/
/*    Public Methods - Display Mode Management                               */
/******************************************************************************/

Display Display::GetCurrent()
{
    return _current;
}

void Display::SetMode(const Display& display)
{
    // Cleanup previous VBE surface if any
    if (g_vbeSurface.valid)
    {
        BiosCleanupVBE(&g_vbeSurface);
        _mappedLfb = nullptr;
    }

    if (display._vbeMode != 0)
    {
        // VBE mode - try to set mode with LFB
        if (BiosSetVBEMode(display._vbeMode, &g_vbeSurface))
        {
            // Success - the surface now contains LFB info
            // Store selector as "mapped LFB" indicator (cast to void* for API compatibility)
            _mappedLfb = reinterpret_cast<void*>(static_cast<unsigned long>(g_vbeSurface.selector));
            _current = display;
        }
        else
        {
            // VBE mode setting failed - revert to text mode
            BiosSetVideoMode(0x03);
            _current = TextMode;
        }
    }
    else
    {
        // Standard VGA mode
        BiosSetVideoMode(display._mode);
        _current = display;
    }
}

void Display::SetDefaultMode()
{
    // Cleanup VBE surface if active
    if (g_vbeSurface.valid)
    {
        BiosCleanupVBE(&g_vbeSurface);
        _mappedLfb = nullptr;
    }
    SetMode(TextMode);
}

void Display::WaitForVSync()
{
    BiosWaitForVSync();
}

/******************************************************************************/
/*    Public Methods - Palette and Gamma                                     */
/******************************************************************************/

void Display::StashPalette()
{
    if (static_cast<bool>(_paletteStashed)) return;

    // Read current palette from VGA DAC
    // Port 0x3C7: Set read index
    // Port 0x3C9: Read RGB values (auto-increments)
    outportb(0x3C7, 0);
    for (Int32 i = Int32(0); static_cast<int>(i) < PALETTE_SIZE; i += 1)
    {
        _originalPalette[static_cast<int>(i)][0] = inportb(0x3C9);  // R
        _originalPalette[static_cast<int>(i)][1] = inportb(0x3C9);  // G
        _originalPalette[static_cast<int>(i)][2] = inportb(0x3C9);  // B
    }
    _paletteStashed = Boolean(true);
}

void Display::SetPaletteScale(Float32 scale)
{
    // Port 0x3C8: Set write index
    // Port 0x3C9: Write RGB values (auto-increments)
    outportb(0x3C8, 0);
    for (Int32 i = Int32(0); static_cast<int>(i) < PALETTE_SIZE; i += 1)
    {
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][0] * static_cast<float>(scale)));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][1] * static_cast<float>(scale)));
        outportb(0x3C9, static_cast<unsigned char>(_originalPalette[static_cast<int>(i)][2] * static_cast<float>(scale)));
    }
}

Boolean Display::CheckGammaSupport()
{
    if (static_cast<bool>(_gammaChecked))
    {
        return _gammaSupported;
    }
    _gammaChecked = Boolean(true);
    _gammaSupported = Boolean(BiosIsGammaSupported());
    return _gammaSupported;
}

Boolean Display::IsGammaSupported()
{
    return CheckGammaSupport();
}

void Display::StashGamma()
{
    if (static_cast<bool>(_gammaStashed)) return;

    // Try to get current gamma table
    if (BiosGetGammaTable(_originalGamma))
    {
        _gammaStashed = Boolean(true);
    }
    else
    {
        // Initialize with identity gamma (no correction)
        for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
        {
            _originalGamma[static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));         // R
            _originalGamma[256 + static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));   // G
            _originalGamma[512 + static_cast<int>(i)] = static_cast<unsigned char>(static_cast<int>(i));   // B
        }
        _gammaStashed = Boolean(true);
    }
}

void Display::SetGammaScale(Float32 scale)
{
    unsigned char scaledGamma[GAMMA_TABLE_SIZE];

    // Scale each channel's gamma entries
    for (Int32 i = Int32(0); static_cast<int>(i) < 256; i += 1)
    {
        scaledGamma[static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[static_cast<int>(i)] * static_cast<float>(scale));         // R
        scaledGamma[256 + static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[256 + static_cast<int>(i)] * static_cast<float>(scale));   // G
        scaledGamma[512 + static_cast<int>(i)] = static_cast<unsigned char>(_originalGamma[512 + static_cast<int>(i)] * static_cast<float>(scale));   // B
    }

    BiosSetGammaTable(scaledGamma);
}

/******************************************************************************/
/*    Public Methods - Fade Effects                                          */
/******************************************************************************/

void Display::FadeIn(Int32 milliseconds)
{
    Int32 ms = milliseconds;
    if (static_cast<int>(ms) < FRAME_MS) ms = Int32(FRAME_MS);

    Int32 steps = Int32(static_cast<int>(ms) / FRAME_MS);
    if (static_cast<int>(steps) < 1) steps = Int32(1);

    // For VBE modes, try hardware gamma ramp first (VBE 3.0)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24)
    {
        // Try VBE 3.0 gamma ramp (hardware-accelerated, like VGA palette)
        if (static_cast<bool>(CheckGammaSupport()))
        {
            StashGamma();

            // Fade from black to full brightness using gamma ramp
            for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(steps); step += 1)
            {
                Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
                SetGammaScale(scale);
                WaitForVSync();
            }
            return;
        }

        // Fallback: software pixel-based fade (slower)
        // Use fewer steps since each step is expensive
        const Int32 VBE_FADE_STEPS = Int32(8);
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        Int32 width = img.Width();
        Int32 height = img.Height();

        // Store original pixel data
        UInt32* original = static_cast<UInt32*>(std::malloc(static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));

        // Fade from black to original
        for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(VBE_FADE_STEPS); step += 1)
        {
            Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(VBE_FADE_STEPS));

            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(width) * static_cast<int>(height); i += 1)
            {
                UInt32 p = original[static_cast<int>(i)];
                UInt8 a = UInt8((static_cast<unsigned int>(p) >> 24) & 0xFF);
                UInt8 r = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 16) & 0xFF) * static_cast<float>(scale)));
                UInt8 g = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 8) & 0xFF) * static_cast<float>(scale)));
                UInt8 b = UInt8(static_cast<unsigned char>((static_cast<unsigned int>(p) & 0xFF) * static_cast<float>(scale)));
                pixels[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) | (static_cast<unsigned int>(r) << 16) | (static_cast<unsigned int>(g) << 8) | static_cast<unsigned int>(b);
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        // Restore original
        std::memcpy(pixels, original, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));
        Drawing::GraphicsBuffer::FlushFrameBuffer();

        std::free(original);
        return;
    }

    // VGA palette-based fade (hardware-accelerated)
    StashPalette();

    // Fade from black to full palette
    for (Int32 step = Int32(0); static_cast<int>(step) <= static_cast<int>(steps); step += 1)
    {
        Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

void Display::FadeOut(Int32 milliseconds)
{
    Int32 ms = milliseconds;
    if (static_cast<int>(ms) < FRAME_MS) ms = Int32(FRAME_MS);

    Int32 steps = Int32(static_cast<int>(ms) / FRAME_MS);
    if (static_cast<int>(steps) < 1) steps = Int32(1);

    // For VBE modes, try hardware gamma ramp first (VBE 3.0)
    if (_current._vbeMode != 0 && _current._bitsPerPixel >= 24)
    {
        // Try VBE 3.0 gamma ramp (hardware-accelerated, like VGA palette)
        if (static_cast<bool>(CheckGammaSupport()))
        {
            StashGamma();

            // Fade from full brightness to black using gamma ramp
            for (Int32 step = steps; static_cast<int>(step) >= 0; step -= 1)
            {
                Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
                SetGammaScale(scale);
                WaitForVSync();
            }
            return;
        }

        // Fallback: software pixel-based fade (slower)
        // Use fewer steps since each step is expensive
        const Int32 VBE_FADE_STEPS = Int32(8);
        Drawing::GraphicsBuffer* fb = Drawing::GraphicsBuffer::GetFrameBuffer();
        if (!fb) return;

        Drawing::Image& img = fb->GetImage();
        Int32 width = img.Width();
        Int32 height = img.Height();

        // Store original pixel data
        UInt32* original = static_cast<UInt32*>(std::malloc(static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32)));
        if (!original) return;

        unsigned int* pixels = img.Data();
        std::memcpy(original, pixels, static_cast<int>(width) * static_cast<int>(height) * sizeof(UInt32));

        // Fade from original to black
        for (Int32 step = VBE_FADE_STEPS; static_cast<int>(step) >= 0; step -= 1)
        {
            Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(VBE_FADE_STEPS));

            for (Int32 i = Int32(0); static_cast<int>(i) < static_cast<int>(width) * static_cast<int>(height); i += 1)
            {
                UInt32 p = original[static_cast<int>(i)];
                UInt8 a = UInt8((static_cast<unsigned int>(p) >> 24) & 0xFF);
                UInt8 r = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 16) & 0xFF) * static_cast<float>(scale)));
                UInt8 g = UInt8(static_cast<unsigned char>(((static_cast<unsigned int>(p) >> 8) & 0xFF) * static_cast<float>(scale)));
                UInt8 b = UInt8(static_cast<unsigned char>((static_cast<unsigned int>(p) & 0xFF) * static_cast<float>(scale)));
                pixels[static_cast<int>(i)] = (static_cast<unsigned int>(a) << 24) | (static_cast<unsigned int>(r) << 16) | (static_cast<unsigned int>(g) << 8) | static_cast<unsigned int>(b);
            }

            Drawing::GraphicsBuffer::FlushFrameBuffer();
            WaitForVSync();
        }

        std::free(original);
        return;
    }

    // VGA palette-based fade (hardware-accelerated)
    StashPalette();

    // Fade from full palette to black
    for (Int32 step = steps; static_cast<int>(step) >= 0; step -= 1)
    {
        Float32 scale = Float32(static_cast<float>(static_cast<int>(step)) / static_cast<int>(steps));
        SetPaletteScale(scale);
        WaitForVSync();
    }
}

/******************************************************************************/
/*    Public Methods - VBE Detection                                         */
/******************************************************************************/

Boolean Display::IsVbeAvailable()
{
    if (static_cast<bool>(_vbeChecked))
    {
        return _vbeAvailable;
    }

    _vbeChecked = Boolean(true);

    // Check if VBE 2.0+ is available
    VbeInfoBlock vbeInfo;
    _vbeAvailable = Boolean(BiosDetectVBE(&vbeInfo));

    // Also verify it's VBE 2.0 or higher (we need LFB support)
    if (static_cast<bool>(_vbeAvailable) && vbeInfo.version < 0x0200)
    {
        _vbeAvailable = Boolean(false);
    }

    return _vbeAvailable;
}

Display Display::DetectVbeMode(UInt16 width, UInt16 height, UInt8 bpp)
{
    unsigned short targetWidth = static_cast<unsigned short>(width);
    unsigned short targetHeight = static_cast<unsigned short>(height);
    UInt8 targetBpp = bpp;

    // Use static buffers to avoid large stack allocations
    static VbeInfoBlock vbeInfo;
    static VbeModeInfoBlock modeInfo;

    // Check if VBE is available
    if (!BiosDetectVBE(&vbeInfo))
    {
        return TextMode;  // VBE not available
    }

    // Common mode numbers to try for 800x600x32
    // These may vary by graphics card, so we try several
    unsigned short modesToTry[] = {
        0x115,  // Standard VESA 800x600x24/32
        0x143,  // Some cards use this
        0x190,  // Alternative
        0x192,  // Alternative
        0x118,  // 1024x768x24 (fallback)
        0x112,  // 640x480x24 (fallback)
        0xFFFF  // End marker
    };

    for (Int32 i = Int32(0); modesToTry[static_cast<int>(i)] != 0xFFFF; i += 1)
    {
        unsigned short mode = modesToTry[static_cast<int>(i)];

        if (!BiosGetVBEModeInfo(mode, &modeInfo))
        {
            continue;
        }

        // Check if mode is supported and has LFB
        if (!(modeInfo.modeAttributes & VBE_ATTR_SUPPORTED))
        {
            continue;
        }
        if (!(modeInfo.modeAttributes & VBE_ATTR_LFB_AVAIL))
        {
            continue;
        }

        // Check dimensions and color depth
        Boolean matchesWidth = Boolean(modeInfo.xResolution == targetWidth);
        Boolean matchesHeight = Boolean(modeInfo.yResolution == targetHeight);
        Boolean matchesBpp = Boolean(modeInfo.bitsPerPixel == static_cast<unsigned char>(targetBpp) ||
                          (static_cast<unsigned char>(targetBpp) == 32 && modeInfo.bitsPerPixel == 24));  // 24/32 often interchangeable

        if (static_cast<bool>(matchesWidth) && static_cast<bool>(matchesHeight) && static_cast<bool>(matchesBpp))
        {
            return Display(mode, modeInfo.bitsPerPixel,
                          modeInfo.xResolution, modeInfo.yResolution,
                          modeInfo.physBasePtr, modeInfo.bytesPerScanLine);
        }
    }

    // Mode not found
    return TextMode;
}

/******************************************************************************/
/*    Public Methods - Text Mode Operations                                  */
/******************************************************************************/

void Display::SetCursorPosition(Int32 row, Int32 col)
{
    BiosSetCursorPosition(static_cast<int>(row), static_cast<int>(col));
}

void Display::GetCursorPosition(Int32& row, Int32& col)
{
    int r, c;
    BiosGetCursorPosition(r, c);
    row = Int32(r);
    col = Int32(c);
}

void Display::GetScreenSize(Int32& rows, Int32& cols)
{
    int r, c;
    BiosGetScreenSize(r, c);
    rows = Int32(r);
    cols = Int32(c);
}

void Display::ClearScreen(UInt8 attr)
{
    int rows, cols;
    BiosGetScreenSize(rows, cols);
    BiosScrollUp(0, static_cast<unsigned char>(attr), 0, 0, rows - 1, cols - 1);
    BiosSetCursorPosition(0, 0);
}

void Display::WriteChar(char c, UInt8 attr)
{
    BiosWriteChar(c, static_cast<unsigned char>(attr));
}

void Display::ScrollUp(Int32 lines, UInt8 attr, Int32 left, Int32 top, Int32 right, Int32 bottom)
{
    BiosScrollUp(static_cast<int>(lines), static_cast<unsigned char>(attr),
                 static_cast<int>(top), static_cast<int>(left),
                 static_cast<int>(bottom), static_cast<int>(right));
}

/******************************************************************************/
/*    Low-Level VGA Operations (public wrappers for GraphicsBuffer use)      */
/******************************************************************************/

void Display::SelectPlane(Int32 plane)
{
    BiosSelectPlane(static_cast<int>(plane));
}

void Display::CopyToVGA(const void* data, UInt32 offset, UInt32 length)
{
    BiosCopyToVGA(data, static_cast<unsigned int>(offset), static_cast<unsigned int>(length));
}

void Display::OutPort(UInt16 port, UInt8 value)
{
    BiosOutPort(static_cast<unsigned short>(port), static_cast<unsigned char>(value));
}

UInt8 Display::InPort(UInt16 port)
{
    return UInt8(BiosInPort(static_cast<unsigned short>(port)));
}

Int32 Display::GetLfbSelector()
{
    // Return the global LFB selector allocated by BiosSetVBEMode
    return Int32(g_lfbSelector);
}

/******************************************************************************/
/*    Static Const Member Initialization                                     */
/******************************************************************************/

const Display Display::TextMode(0x03, 0, 80, 25);
const Display Display::VGA_320x200x8(0x13, 8, 320, 200);
const Display Display::VGA_640x480x4(0x12, 4, 640, 480);

} // namespace System::IO::Devices
