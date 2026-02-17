// VBE Debug Test - Isolates each VBE operation to find crash point
// Run this to identify which VBE step causes the page fault

#include <cstdio>
#include <cstring>
#include <conio.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>

// VBE structures (copied from Graphics.hpp to be self-contained)
#pragma pack(push, 1)
struct VbeInfoBlock {
    char     signature[4];
    unsigned short version;
    unsigned int   oemString;
    unsigned int   capabilities;
    unsigned int   modeList;
    unsigned short totalMemory;
    unsigned short oemVersion;
    unsigned int   oemVendor;
    unsigned int   oemProduct;
    unsigned int   oemRevision;
    unsigned char  reserved[222];
    unsigned char  oemData[256];
};

struct VbeModeInfoBlock {
    unsigned short modeAttributes;
    unsigned char  winAAttributes;
    unsigned char  winBAttributes;
    unsigned short winGranularity;
    unsigned short winSize;
    unsigned short winASegment;
    unsigned short winBSegment;
    unsigned int   winFuncPtr;
    unsigned short bytesPerScanLine;
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
    unsigned char  redMaskSize;
    unsigned char  redFieldPosition;
    unsigned char  greenMaskSize;
    unsigned char  greenFieldPosition;
    unsigned char  blueMaskSize;
    unsigned char  blueFieldPosition;
    unsigned char  rsvdMaskSize;
    unsigned char  rsvdFieldPosition;
    unsigned char  directColorModeInfo;
    unsigned int   physBasePtr;
    unsigned int   reserved2;
    unsigned short reserved3;
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

const unsigned short VBE_SUCCESS = 0x004F;
const unsigned short VBE_MODE_LFB = 0x4000;
const unsigned short VBE_ATTR_LFB_AVAIL = 0x0080;

void waitKey() {
    printf("Press any key...\n");
    getch();
}

int main() {
    printf("=== VBE Debug Test ===\n\n");

    // Use static buffers to avoid stack issues
    static VbeInfoBlock vbeInfo;
    static VbeModeInfoBlock modeInfo;

    //-------------------------------------------------------------------------
    // Step 1: Test transfer buffer access
    //-------------------------------------------------------------------------
    printf("Step 1: Testing transfer buffer (__tb)...\n");

    unsigned long tbAddr = __tb;
    unsigned short tbSeg = static_cast<unsigned short>(tbAddr >> 4);
    unsigned short tbOff = static_cast<unsigned short>(tbAddr & 0x0F);

    printf("  __tb = 0x%lX, seg:off = %04X:%04X\n", tbAddr, tbSeg, tbOff);

    // Try writing to transfer buffer
    printf("  Writing 'TEST' to transfer buffer...\n");
    dosmemput("TEST", 4, tbAddr);

    char readBack[5] = {0};
    dosmemget(tbAddr, 4, readBack);
    printf("  Read back: '%s'\n", readBack);

    if (std::strcmp(readBack, "TEST") != 0) {
        printf("  FAILED: Transfer buffer read/write mismatch!\n");
        waitKey();
        return 1;
    }
    printf("  OK: Transfer buffer works.\n\n");

    //-------------------------------------------------------------------------
    // Step 2: VBE Detection (INT 10h AX=4F00h)
    //-------------------------------------------------------------------------
    printf("Step 2: Detecting VBE (INT 10h AX=4F00h)...\n");
    waitKey();

    // Write VBE2 signature
    dosmemput("VBE2", 4, tbAddr);

    __dpmi_regs regs;
    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F00;
    regs.x.es = tbSeg;
    regs.x.di = tbOff;

    printf("  Calling INT 10h...\n");
    __dpmi_int(0x10, &regs);

    // CRITICAL: Copy result IMMEDIATELY before any printf (which uses __tb!)
    unsigned short axResult = regs.x.ax;
    dosmemget(tbAddr, sizeof(VbeInfoBlock), &vbeInfo);

    // Also save raw bytes for debugging
    unsigned char rawBytes[16];
    dosmemget(tbAddr, 16, rawBytes);  // This is fine - vbeInfo already saved

    printf("  Returned: AX=0x%04X\n", axResult);

    if (axResult != VBE_SUCCESS) {
        printf("  FAILED: VBE not available (AX != 0x004F)\n");
        waitKey();
        return 1;
    }

    // Dump raw bytes (from saved copy)
    printf("  Raw bytes (first 16):\n  ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", rawBytes[i]);
    }
    printf("\n  As ASCII: ");
    for (int i = 0; i < 16; i++) {
        unsigned char c = rawBytes[i];
        printf("%c", (c >= 32 && c < 127) ? c : '.');
    }
    printf("\n");

    printf("  Signature: %.4s (hex: %02X %02X %02X %02X)\n",
           vbeInfo.signature,
           (unsigned char)vbeInfo.signature[0],
           (unsigned char)vbeInfo.signature[1],
           (unsigned char)vbeInfo.signature[2],
           (unsigned char)vbeInfo.signature[3]);
    printf("  Version: %d.%d (raw: 0x%04X)\n",
           vbeInfo.version >> 8, vbeInfo.version & 0xFF, vbeInfo.version);
    printf("  Total memory: %d x 64KB = %dKB\n", vbeInfo.totalMemory, vbeInfo.totalMemory * 64);

    if (vbeInfo.signature[0] != 'V' || vbeInfo.signature[1] != 'E' ||
        vbeInfo.signature[2] != 'S' || vbeInfo.signature[3] != 'A') {
        printf("  FAILED: Invalid VESA signature\n");
        printf("\n  Trying alternate: use __tb_size for bounds...\n");
        printf("  __tb_size = %lu\n", (unsigned long)_go32_info_block.size_of_transfer_buffer);
        waitKey();
        return 1;
    }
    printf("  OK: VBE detected.\n\n");

    //-------------------------------------------------------------------------
    // Step 3: Get Mode Info (INT 10h AX=4F01h)
    //-------------------------------------------------------------------------
    unsigned short testMode = 0x115;  // 800x600x24/32
    printf("Step 3: Getting mode info for 0x%03X (INT 10h AX=4F01h)...\n", testMode);
    waitKey();

    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F01;
    regs.x.cx = testMode;
    regs.x.es = tbSeg;
    regs.x.di = tbOff;

    printf("  Calling INT 10h...\n");
    __dpmi_int(0x10, &regs);

    printf("  Returned: AX=0x%04X\n", regs.x.ax);

    if (regs.x.ax != VBE_SUCCESS) {
        printf("  Mode 0x%03X not supported, trying 0x101 (640x480x8)...\n", testMode);
        testMode = 0x101;

        std::memset(&regs, 0, sizeof(regs));
        regs.x.ax = 0x4F01;
        regs.x.cx = testMode;
        regs.x.es = tbSeg;
        regs.x.di = tbOff;
        __dpmi_int(0x10, &regs);

        if (regs.x.ax != VBE_SUCCESS) {
            printf("  FAILED: No VBE modes available\n");
            waitKey();
            return 1;
        }
    }

    dosmemget(tbAddr, sizeof(VbeModeInfoBlock), &modeInfo);

    printf("  Resolution: %dx%d\n", modeInfo.xResolution, modeInfo.yResolution);
    printf("  BPP: %d\n", modeInfo.bitsPerPixel);
    printf("  Attributes: 0x%04X\n", modeInfo.modeAttributes);
    printf("  PhysBasePtr: 0x%08X\n", modeInfo.physBasePtr);
    printf("  BytesPerScanLine: %d\n", modeInfo.bytesPerScanLine);
    printf("  LFB available: %s\n", (modeInfo.modeAttributes & VBE_ATTR_LFB_AVAIL) ? "YES" : "NO");
    printf("  OK: Mode info retrieved.\n\n");

    //-------------------------------------------------------------------------
    // Step 4: Set VBE Mode (INT 10h AX=4F02h)
    //-------------------------------------------------------------------------
    printf("Step 4: Setting VBE mode 0x%03X with LFB...\n", testMode);
    waitKey();

    std::memset(&regs, 0, sizeof(regs));
    regs.x.ax = 0x4F02;
    regs.x.bx = testMode | VBE_MODE_LFB;

    printf("  Calling INT 10h with BX=0x%04X...\n", regs.x.bx);
    __dpmi_int(0x10, &regs);

    printf("  Returned: AX=0x%04X\n", regs.x.ax);

    if (regs.x.ax != VBE_SUCCESS) {
        printf("  FAILED: Could not set VBE mode\n");
        // Return to text mode
        regs.h.ah = 0x00;
        regs.h.al = 0x03;
        __dpmi_int(0x10, &regs);
        waitKey();
        return 1;
    }
    printf("  OK: VBE mode set (screen may look garbled).\n\n");

    //-------------------------------------------------------------------------
    // Step 5: Map Physical Address
    //-------------------------------------------------------------------------
    printf("Step 5: Mapping physical LFB address...\n");

    unsigned long lfbSize = static_cast<unsigned long>(modeInfo.yResolution) *
                            static_cast<unsigned long>(modeInfo.bytesPerScanLine);

    printf("  PhysAddr: 0x%08X, Size: %lu bytes\n", modeInfo.physBasePtr, lfbSize);

    __dpmi_meminfo memInfo;
    memInfo.address = modeInfo.physBasePtr;
    memInfo.size = lfbSize;

    printf("  Calling __dpmi_physical_address_mapping()...\n");
    int mapResult = __dpmi_physical_address_mapping(&memInfo);

    printf("  Result: %d, Linear addr: 0x%08lX\n", mapResult, memInfo.address);

    if (mapResult != 0) {
        printf("  FAILED: Could not map physical address\n");
        // Return to text mode
        regs.h.ah = 0x00;
        regs.h.al = 0x03;
        __dpmi_int(0x10, &regs);
        waitKey();
        return 1;
    }
    printf("  OK: Physical address mapped.\n\n");

    //-------------------------------------------------------------------------
    // Step 6: Allocate LDT Descriptor
    //-------------------------------------------------------------------------
    printf("Step 6: Allocating LDT descriptor...\n");

    int selector = __dpmi_allocate_ldt_descriptors(1);
    printf("  Selector: %d (0x%04X)\n", selector, selector);

    if (selector < 0) {
        printf("  FAILED: Could not allocate LDT descriptor\n");
        __dpmi_free_physical_address_mapping(&memInfo);
        regs.h.ah = 0x00;
        regs.h.al = 0x03;
        __dpmi_int(0x10, &regs);
        waitKey();
        return 1;
    }

    printf("  Setting segment base to 0x%08lX...\n", memInfo.address);
    __dpmi_set_segment_base_address(selector, memInfo.address);

    printf("  Setting segment limit to %lu...\n", lfbSize - 1);
    __dpmi_set_segment_limit(selector, lfbSize - 1);

    printf("  OK: LDT descriptor configured.\n\n");

    //-------------------------------------------------------------------------
    // Step 7: Write to LFB using selector
    //-------------------------------------------------------------------------
    printf("Step 7: Writing test pattern to LFB...\n");

    // Write a simple test pattern - fill first few rows with white
    unsigned int rowBytes = modeInfo.bytesPerScanLine;
    unsigned char testData[1024];
    std::memset(testData, 0xFF, sizeof(testData));  // White pixels

    printf("  Writing %d bytes to first row using movedata()...\n", rowBytes > 1024 ? 1024 : rowBytes);

    unsigned int srcOffset = reinterpret_cast<unsigned int>(testData);
    movedata(_my_ds(), srcOffset, selector, 0, rowBytes > 1024 ? 1024 : rowBytes);

    printf("  OK: Data written to LFB.\n\n");

    //-------------------------------------------------------------------------
    // Step 8: Verify and cleanup
    //-------------------------------------------------------------------------
    printf("Step 8: Test complete! Press any key to return to text mode...\n");
    getch();

    // Cleanup
    __dpmi_free_ldt_descriptor(selector);
    __dpmi_free_physical_address_mapping(&memInfo);

    // Return to text mode
    regs.h.ah = 0x00;
    regs.h.al = 0x03;
    __dpmi_int(0x10, &regs);

    printf("\n=== All VBE steps completed successfully! ===\n");
    printf("The VBE implementation should work.\n");
    waitKey();

    return 0;
}
