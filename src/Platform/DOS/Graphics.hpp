#ifndef PLATFORM_DOS_GRAPHICS_HPP
#define PLATFORM_DOS_GRAPHICS_HPP

namespace Platform { namespace DOS {

/******************************************************************************/
/*    VBE 2.0+ Structures (packed for BIOS compatibility)                     */
/******************************************************************************/

#pragma pack(push, 1)

// VBE Controller Info Block (returned by INT 10h AX=4F00h)
struct VbeInfoBlock {
    char     signature[4];      // "VESA" or "VBE2"
    unsigned short version;     // VBE version (e.g., 0x0200 = 2.0)
    unsigned int   oemString;   // Far pointer to OEM string
    unsigned int   capabilities;// Capability flags
    unsigned int   modeList;    // Far pointer to mode list
    unsigned short totalMemory; // Memory in 64KB blocks
    // VBE 2.0+ fields
    unsigned short oemVersion;
    unsigned int   oemVendor;
    unsigned int   oemProduct;
    unsigned int   oemRevision;
    unsigned char  reserved[222];
    unsigned char  oemData[256];
};

// VBE Mode Info Block (returned by INT 10h AX=4F01h)
struct VbeModeInfoBlock {
    // Mandatory for all VBE versions
    unsigned short modeAttributes;
    unsigned char  winAAttributes;
    unsigned char  winBAttributes;
    unsigned short winGranularity;
    unsigned short winSize;
    unsigned short winASegment;
    unsigned short winBSegment;
    unsigned int   winFuncPtr;
    unsigned short bytesPerScanLine;
    // VBE 1.2+ fields
    unsigned short xResolution;
    unsigned short yResolution;
    unsigned char  xCharSize;
    unsigned char  yCharSize;
    unsigned char  numberOfPlanes;
    unsigned char  bitsPerPixel;
    unsigned char  numberOfBanks;
    unsigned char  memoryModel;
    unsigned char  bankSize;
    unsigned char  numberOfImagePages;
    unsigned char  reserved1;
    // Direct color fields
    unsigned char  redMaskSize;
    unsigned char  redFieldPosition;
    unsigned char  greenMaskSize;
    unsigned char  greenFieldPosition;
    unsigned char  blueMaskSize;
    unsigned char  blueFieldPosition;
    unsigned char  rsvdMaskSize;
    unsigned char  rsvdFieldPosition;
    unsigned char  directColorModeInfo;
    // VBE 2.0+ fields
    unsigned int   physBasePtr;     // Physical address of LFB
    unsigned int   reserved2;
    unsigned short reserved3;
    // VBE 3.0+ fields
    unsigned short linBytesPerScanLine;
    unsigned char  bnkNumberOfImagePages;
    unsigned char  linNumberOfImagePages;
    unsigned char  linRedMaskSize;
    unsigned char  linRedFieldPosition;
    unsigned char  linGreenMaskSize;
    unsigned char  linGreenFieldPosition;
    unsigned char  linBlueMaskSize;
    unsigned char  linBlueFieldPosition;
    unsigned char  linRsvdMaskSize;
    unsigned char  linRsvdFieldPosition;
    unsigned int   maxPixelClock;
    unsigned char  reserved4[189];
};

#pragma pack(pop)

/******************************************************************************/
/*    VBE Constants                                                           */
/******************************************************************************/

// VBE function return values
const unsigned short VBE_SUCCESS = 0x004F;

// VBE mode flags
const unsigned short VBE_MODE_LFB = 0x4000;  // Enable linear framebuffer
const unsigned short VBE_MODE_CLEAR = 0x8000; // Don't clear display

// Common VBE mode numbers
const unsigned short VBE_MODE_640x480x8    = 0x101;
const unsigned short VBE_MODE_800x600x8    = 0x103;
const unsigned short VBE_MODE_1024x768x8   = 0x105;
const unsigned short VBE_MODE_640x480x16   = 0x111;
const unsigned short VBE_MODE_800x600x16   = 0x114;
const unsigned short VBE_MODE_640x480x24   = 0x112;
const unsigned short VBE_MODE_800x600x24   = 0x115;
const unsigned short VBE_MODE_640x480x32   = 0x112;  // Often same as 24bpp
const unsigned short VBE_MODE_800x600x32   = 0x115;  // May need probing

// Mode attribute flags
const unsigned short VBE_ATTR_SUPPORTED    = 0x0001;
const unsigned short VBE_ATTR_LFB_AVAIL    = 0x0080;

// VBE 3.0 Gamma/DAC functions (INT 10h AX=4F15h)
const unsigned char VBE_GAMMA_SET          = 0x01;  // BL=01h: Set gamma table
const unsigned char VBE_GAMMA_GET          = 0x02;  // BL=02h: Get gamma table

// Gamma table size (256 entries per channel, 3 channels)
const int VBE_GAMMA_TABLE_SIZE = 256 * 3;

/******************************************************************************/
/*    Graphics class                                                          */
/******************************************************************************/

// VBE framebuffer surface info
struct VbeSurface {
    int selector;              // LDT selector for LFB access
    unsigned long linearAddr;  // Linear address of mapped LFB
    unsigned long size;        // Size of mapped region
    unsigned int pitch;        // Bytes per scanline
    unsigned short width;
    unsigned short height;
    unsigned char bpp;
    bool valid;
};

class Graphics {
public:
    // Set VGA video mode
    static void SetVideoMode(unsigned char mode);

    // Get current video mode
    static unsigned char GetVideoMode();

    // Wait for vertical retrace (vsync)
    static void WaitForVSync();

    // Write byte to VGA plane (for mode 0x12)
    static void SelectPlane(int plane);

    // Copy data to VGA memory
    static void CopyToVGA(const void* data, unsigned int offset, unsigned int length);

    // Read/write VGA ports
    static void OutPort(unsigned short port, unsigned char value);
    static unsigned char InPort(unsigned short port);

    // VBE 2.0+ functions
    static bool DetectVBE(VbeInfoBlock* info);
    static bool GetVBEModeInfo(unsigned short mode, VbeModeInfoBlock* info);
    static bool SetVBEMode(unsigned short mode, VbeSurface* surface);
    static void CleanupVBE(VbeSurface* surface);

    // LFB access using selector (use _farsetsel/_farnspokeb etc.)
    static int GetLfbSelector();

    // VBE 3.0 Gamma ramp functions
    // Returns true if VBE 3.0 gamma control is supported
    static bool IsGammaSupported();

    // Set gamma table (256 entries per R/G/B channel, 768 bytes total)
    // Each entry maps input intensity to output intensity (0-255)
    // Identity: gamma[i] = i for all i
    // Fade to black: gamma[i] = i * scale
    static bool SetGammaTable(const unsigned char* gammaTable);

    // Get current gamma table
    static bool GetGammaTable(unsigned char* gammaTable);
};

}} // namespace Platform::DOS

#endif // PLATFORM_DOS_GRAPHICS_HPP
