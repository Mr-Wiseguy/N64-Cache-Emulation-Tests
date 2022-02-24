#include <cstdint>
#include <ultra64.h>
#include <text.h>

struct CacheTestData {
    uint8_t testHitInvalidate1;
    uint8_t testHitInvalidate2;
    uint8_t testHitInvalidate3;
    uint8_t testLineInvalidate1;
    uint8_t testLineInvalidate2;
    uint8_t testLineInvalidate3;
    uint8_t testDmaHitInvalidate1;
    uint8_t testDmaHitInvalidate2;
    uint8_t testDmaLineInvalidate1;
    uint8_t testDmaLineInvalidate2;
};

struct CacheTestData cacheTestResults;
extern OSPiHandle *g_romHandle;

extern "C" int cachefunc();

void do_cache_tests() {
    // Call once to put into icache
    // n64::disableInterrupts();
    OSMesgQueue dmaMesgQueue;
    OSMesg buf;
    OSIoMesg ioMesg;
    extern u8 _cachefuncDMASegmentRomStart[];
    ioMesg.devAddr = (u32)_cachefuncDMASegmentRomStart;
    ioMesg.dramAddr = (void*)&cachefunc;
    ioMesg.hdr.pri = OS_MESG_PRI_NORMAL;
    ioMesg.hdr.retQueue = &dmaMesgQueue;
    ioMesg.size = 16;
    osCreateMesgQueue(&dmaMesgQueue, &buf, 1);
    cachefunc();

    // Test with hit_invalidate
    // Overwrite second instruction of cachefunc with `addiu $v0, $zero, 2`
    ((u32*)&cachefunc)[1] = 0x24020002;
    cacheTestResults.testHitInvalidate1 = cachefunc(); // Should still return 1 because dcache hasn't been flushed and icache hasn't been invalidated
    osWritebackDCache((void*)&cachefunc, 8); // Writeback the bytes written to cachefunc
    cacheTestResults.testHitInvalidate2 = cachefunc(); // Should still return 1 because icache hasn't been invalidated
    osInvalICache((void*)&cachefunc, 8); // Invalidate the bytes of cachefunc in the icache
    cacheTestResults.testHitInvalidate3 = cachefunc(); // Should now return 2

    // Reset the function back to the original instruction
    ((u32*)&cachefunc)[1] = 0x24020001;
    osWritebackDCache((void*)&cachefunc, 8); // Writeback the bytes written to cachefunc
    osInvalICache((void*)&cachefunc, 8); // Invalidate the bytes of cachefunc in the icache

    // Call once to put into icache
    cachefunc();

    // Test with line_invalidate
    // Overwrite second instruction of cachefunc with `addiu $v0, $zero, 2`
    ((u32*)&cachefunc)[1] = 0x24020002;
    cacheTestResults.testLineInvalidate1 = cachefunc(); // Should still return 1 because dcache hasn't been flushed and icache hasn't been invalidated
    osWritebackDCache((void*)&cachefunc, 8); // Writeback the bytes written to cachefunc
    cacheTestResults.testLineInvalidate2 = cachefunc(); // Should still return 1 because icache hasn't been invalidated
    osInvalICache((void*)&cachefunc, 32768); // Invalidate the bytes of cachefunc in the icache, using a large value so line_invalidate is used instead
    cacheTestResults.testLineInvalidate3 = cachefunc(); // Should now return 2

    // Reset the function back to the original instruction
    ((u32*)&cachefunc)[1] = 0x24020001;
    osWritebackDCache((void*)&cachefunc, 8); // Writeback the bytes written to cachefunc
    osInvalICache((void*)&cachefunc, 8); // Invalidate the bytes of cachefunc in the icache

    // Call once to put into icache
    cachefunc();

    // Test with hit_invalidate and PI DMA
    // Overwrite second instruction of cachefunc with `addiu $v0, $zero, 2`
    osEPiStartDma(g_romHandle, &ioMesg, OS_READ);
    osRecvMesg(&dmaMesgQueue, NULL, OS_MESG_BLOCK);
    cacheTestResults.testDmaHitInvalidate1 = cachefunc(); // Should still return 1 because icache hasn't been invalidated
    osInvalICache((void*)&cachefunc, 8); // Invalidate the bytes of cachefunc in the icache
    cacheTestResults.testDmaHitInvalidate2 = cachefunc(); // Should now return 2

    // Reset the function back to the original instruction
    ((u32*)&cachefunc)[1] = 0x24020001;
    osWritebackDCache((void*)&cachefunc, 8); // Writeback the bytes written to cachefunc
    osInvalICache((void*)&cachefunc, 8); // Invalidate the bytes of cachefunc in the icache

    // Call once to put into icache
    cachefunc();

    // Test with line_invalidate and PI DMA
    // Overwrite second instruction of cachefunc with `addiu $v0, $zero, 2`
    osEPiStartDma(g_romHandle, &ioMesg, OS_READ);
    osRecvMesg(&dmaMesgQueue, NULL, OS_MESG_BLOCK);
    cacheTestResults.testDmaLineInvalidate1 = cachefunc(); // Should still return 1 because icache hasn't been invalidated
    osInvalICache((void*)&cachefunc, 32768); // Invalidate the bytes of cachefunc in the icache
    cacheTestResults.testDmaLineInvalidate2 = cachefunc(); // Should now return 2

    // n64::enableInterrupts();
}

void print_pass_fail(int y, int got, int expected) {
    char buf[16];
    if (got == expected) {
        set_text_color(0x00, 0x7F, 0x00, 0xFF);
        print_text(10 + 31 * 6, y, "PASS");
    } else {
        set_text_color(0x7F, 0x00, 0x00, 0xFF);
        print_text(10 + 31 * 6, y, "FAIL");
    }
    set_text_color(0xFF, 0xFF, 0xFF, 0xFF);
    sprintf(buf, "(got %d)", got);
    print_text(10 + (31 + 5) * 6, y, buf);

}

void print_cache_results() {
    set_text_color(255, 255, 255, 255);
    print_text(10, 10, "Cache Integrity Test 1:");
    print_pass_fail(10, cacheTestResults.testHitInvalidate1, 1);

    print_text(10, 20, "Cache Integrity Test 2:");
    print_pass_fail(20, cacheTestResults.testHitInvalidate2, 1);

    print_text(10, 30, "Hit Invalidate Test:");
    print_pass_fail(30, cacheTestResults.testHitInvalidate3, 2);
    
    print_text(10, 40, "Cache Integrity Test 3:");
    print_pass_fail(40, cacheTestResults.testLineInvalidate1, 1);

    print_text(10, 50, "Cache Integrity Test 4:");
    print_pass_fail(50, cacheTestResults.testLineInvalidate2, 1);
    
    print_text(10, 60, "Line Invalidate Test:");
    print_pass_fail(60, cacheTestResults.testLineInvalidate3, 2);

    print_text(10, 70, "PI DMA Cache Integrity Test 1:");
    print_pass_fail(70, cacheTestResults.testDmaHitInvalidate1, 1);

    print_text(10, 80, "PI DMA Hit Invalidate Test:");
    print_pass_fail(80, cacheTestResults.testDmaHitInvalidate2, 2);

    print_text(10, 90, "PI DMA Cache Integrity Test 2:");
    print_pass_fail(90, cacheTestResults.testDmaLineInvalidate1, 1);

    print_text(10, 100, "PI DMA Line Invalidate Test:");
    print_pass_fail(100, cacheTestResults.testDmaLineInvalidate2, 2);
}
