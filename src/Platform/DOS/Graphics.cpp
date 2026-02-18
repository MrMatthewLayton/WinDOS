#include "Graphics.hpp"
#include <dos.h>
#include <go32.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <pc.h>
#include <sys/movedata.h>
#include <cstring>
#include <sys/nearptr.h>

namespace Platform::DOS
{

void Graphics::SetVideoMode(unsigned char mode)
{
    __dpmi_regs regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    __dpmi_int(0x10, &regs);
}

unsigned char Graphics::GetVideoMode()
{
    __dpmi_regs regs;
    regs.h.ah = 0x0F;
    __dpmi_int(0x10, &regs);
    return regs.h.al;
}

void Graphics::WaitForVSync()
{
    // Wait until not in vertical retrace
    while ((inportb(0x3DA) & 0x08) != 0);
    // Wait until in vertical retrace
    while ((inportb(0x3DA) & 0x08) == 0);
}

void Graphics::SelectPlane(int plane)
{
    outportb(0x3C4, 0x02);  // Map Mask Register
    outportb(0x3C5, 1 << plane);
}

void Graphics::CopyToVGA(const void* data, unsigned int offset, unsigned int length)
{
    // VGA memory starts at 0xA0000
    dosmemput(data, length, 0xA0000 + offset);
}

void Graphics::OutPort(unsigned short port, unsigned char value)
{
    outportb(port, value);
}

unsigned char Graphics::InPort(unsigned short port)
{
    return inportb(port);
}

/******************************************************************************/
/*    VBE 2.0+ Implementation                                                 */
/*    Based on: https://github.com/tobijk/vesa-dos-djgpp                      */
/*    Uses LDT descriptor approach for LFB access (not near pointers)         */
/******************************************************************************/

// Global LFB selector for use with _farsetsel/_farnspokeb etc.
static int g_lfbSelector = 0;

bool Graphics::DetectVBE(VbeInfoBlock* info)
{
    if (!info)
    {
        return false;
    }

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
    dosmemget(tbAddr, sizeof(VbeInfoBlock), info);

    // Verify VESA signature
    if (info->signature[0] != 'V' || info->signature[1] != 'E' ||
        info->signature[2] != 'S' || info->signature[3] != 'A')
    {
        return false;
    }

    return true;
}

bool Graphics::GetVBEModeInfo(unsigned short mode, VbeModeInfoBlock* info)
{
    if (!info)
    {
        return false;
    }

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
    dosmemget(tbAddr, sizeof(VbeModeInfoBlock), info);
    return true;
}

bool Graphics::SetVBEMode(unsigned short mode, VbeSurface* surface)
{
    if (!surface)
    {
        return false;
    }

    // Initialize surface
    surface->valid = false;
    surface->selector = 0;
    surface->linearAddr = 0;

    // Get mode info first
    VbeModeInfoBlock modeInfo;
    std::memset(&modeInfo, 0, sizeof(modeInfo));
    if (!GetVBEModeInfo(mode, &modeInfo))
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
        SetVideoMode(0x03);  // Revert to text mode
        return false;
    }

    // Lock the linear region
    __dpmi_lock_linear_region(&memInfo);

    // Allocate an LDT descriptor for accessing the LFB
    int selector = __dpmi_allocate_ldt_descriptors(1);
    if (selector < 0)
    {
        __dpmi_free_physical_address_mapping(&memInfo);
        SetVideoMode(0x03);
        return false;
    }

    // Set the descriptor base address and limit
    __dpmi_set_segment_base_address(selector, memInfo.address);
    __dpmi_set_segment_limit(selector, lfbSize - 1);

    // Fill in the surface info
    surface->selector = selector;
    surface->linearAddr = memInfo.address;
    surface->size = lfbSize;
    surface->pitch = modeInfo.bytesPerScanLine;
    surface->width = modeInfo.xResolution;
    surface->height = modeInfo.yResolution;
    surface->bpp = modeInfo.bitsPerPixel;
    surface->valid = true;

    // Store global selector for GetLfbSelector()
    g_lfbSelector = selector;

    return true;
}

void Graphics::CleanupVBE(VbeSurface* surface)
{
    if (!surface || !surface->valid)
    {
        return;
    }

    // Free the LDT descriptor
    if (surface->selector > 0)
    {
        __dpmi_free_ldt_descriptor(surface->selector);
    }

    // Unmap the physical memory
    if (surface->linearAddr != 0)
    {
        __dpmi_meminfo memInfo;
        memInfo.address = surface->linearAddr;
        memInfo.size = surface->size;
        __dpmi_free_physical_address_mapping(&memInfo);
    }

    surface->valid = false;
    surface->selector = 0;
    surface->linearAddr = 0;
    g_lfbSelector = 0;
}

int Graphics::GetLfbSelector()
{
    return g_lfbSelector;
}

/******************************************************************************/
/*    VBE 3.0 Gamma Ramp Implementation                                        */
/*    Function 4F15h - Display Power Management & Gamma Control                */
/******************************************************************************/

// Cache gamma support detection result
static int g_gammaSupported = -1;  // -1 = not checked, 0 = no, 1 = yes

bool Graphics::IsGammaSupported()
{
    if (g_gammaSupported >= 0)
    {
        return g_gammaSupported == 1;
    }

    // First check VBE version (need 3.0+)
    VbeInfoBlock vbeInfo;
    if (!DetectVBE(&vbeInfo))
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

bool Graphics::SetGammaTable(const unsigned char* gammaTable)
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

bool Graphics::GetGammaTable(unsigned char* gammaTable)
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

} // namespace Platform::DOS
