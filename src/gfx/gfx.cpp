#include <ultra64.h>

#include <gfx.h>
#include <mem.h>
#include <task_sched.h>

extern std::array<std::array<u16, screen_width * screen_height>, num_frame_buffers> g_frameBuffers;
extern std::array<u16, screen_width * screen_height> g_depthBuffer;

struct GfxContext g_gfxContexts[num_frame_buffers];

std::array<OSScTask, num_frame_buffers> gfxTasks;

std::array<u64, SP_DRAM_STACK_SIZE64> taskStack;
std::array<u64, output_buff_len> taskOutputBuffer;
std::array<u64, OS_YIELD_DATA_SIZE / sizeof(u64)> taskYieldBuffer;

Gfx *g_dlist_head;
Gfx *g_gui_dlist_head;

// The index of the context for the task being constructed
u32 g_curGfxContext;

uint8_t dram_stack[SP_DRAM_STACK_SIZE8] alignas(16);
uint8_t output_buff[32] alignas(16);
uint8_t data_buff[32] alignas(16);
uint8_t yield_buff[OS_YIELD_DATA_SIZE] alignas(16);

void gfx_init(void)
{
    unsigned int i;

    // Set up the graphics tasks
    for (i = 0; i < num_frame_buffers; i++)
    {
        // Set up OSScTask fields

        // Set up fifo task, configure it to automatically swap buffers after completion
        gfxTasks[i].flags = OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_SWAPBUFFER | OS_SC_LAST_TASK;

        gfxTasks[i].framebuffer = g_frameBuffers[i].data();
        gfxTasks[i].msgQ = &g_gfxContexts[i].taskDoneQueue;
        osCreateMesgQueue(&g_gfxContexts[i].taskDoneQueue, &g_gfxContexts[i].taskDoneMesg, 1);

        // Set up OSTask fields

        // Make this a graphics task
        gfxTasks[i].list.t.type = M_GFXTASK;
        gfxTasks[i].list.t.flags = OS_TASK_DP_WAIT;

        // Set up the gfx task boot microcode pointer and size
        gfxTasks[i].list.t.ucode_boot = (u64*) rspbootTextStart;
        gfxTasks[i].list.t.ucode_boot_size = (u32)rspbootTextEnd - (u32)rspbootTextStart;

        // // Set up the gfx task gfx microcode text pointer and size
        // gfxTasks[i].list.t.ucode = (u64*) testTextStart;
        // gfxTasks[i].list.t.ucode_size = (u32)testTextEnd - (u32)testTextStart;
        gfxTasks[i].list.t.ucode = (u64*) gspF3DEX2_fifoTextStart;
        gfxTasks[i].list.t.ucode_size = (u32)gspF3DEX2_fifoTextEnd - (u32)gspF3DEX2_fifoTextStart;
        // gfxTasks[i].list.t.ucode = (u64*) gspF3DEX2_Rej_fifoTextStart;
        // gfxTasks[i].list.t.ucode_size = (u32)gspF3DEX2_Rej_fifoTextEnd - (u32)gspF3DEX2_Rej_fifoTextStart;

        // // Set up the gfx task gfx microcode data pointer and size
        // gfxTasks[i].list.t.ucode_data = (u64*) testDataStart;
        // gfxTasks[i].list.t.ucode_data_size = (u32)testDataEnd - (u32)testDataStart;
        gfxTasks[i].list.t.ucode_data = (u64*) gspF3DEX2_fifoDataStart;
        gfxTasks[i].list.t.ucode_data_size = (u32)gspF3DEX2_fifoDataEnd - (u32)gspF3DEX2_fifoDataStart;
        // gfxTasks[i].list.t.ucode_data = (u64*) gspF3DEX2_Rej_fifoDataStart;
        // gfxTasks[i].list.t.ucode_data_size = (u32)gspF3DEX2_Rej_fifoDataEnd - (u32)gspF3DEX2_Rej_fifoDataStart;

        gfxTasks[i].list.t.dram_stack = &taskStack[0];
        gfxTasks[i].list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;

        gfxTasks[i].list.t.output_buff = &taskOutputBuffer[0];
        gfxTasks[i].list.t.output_buff_size = &taskOutputBuffer[output_buff_len];

        gfxTasks[i].list.t.data_ptr = (u64*)&g_gfxContexts[i].dlist_buffer[0];

        gfxTasks[i].list.t.yield_data_ptr = &taskYieldBuffer[0];
        gfxTasks[i].list.t.yield_data_size = OS_YIELD_DATA_SIZE;
    }

    // Send a dummy complete message to the last task, so the first one can run
    osSendMesg(gfxTasks[num_frame_buffers - 1].msgQ, gfxTasks[num_frame_buffers - 1].msg, OS_MESG_BLOCK);

    // Set the gfx context index to 0
    g_curGfxContext = 0;
}

constexpr s32 default_geometry_mode = G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK | G_LIGHTING;

void resetGfxFrame(void)
{
    // Set up the master displaylist head
    g_dlist_head = &g_gfxContexts[g_curGfxContext].dlist_buffer[0];
    g_gui_dlist_head = &g_gfxContexts[g_curGfxContext].gui_dlist_buffer[0];
}

void sendGfxTask(void)
{
    gfxTasks[g_curGfxContext].list.t.data_size = (u32)g_dlist_head - (u32)&g_gfxContexts[g_curGfxContext].dlist_buffer[0];

    // Writeback cache for graphics task data
    osWritebackDCacheAll();

    // Wait for the previous RSP task to complete
    osRecvMesg(gfxTasks[(g_curGfxContext + (num_frame_buffers - 1)) % num_frame_buffers].msgQ, nullptr, OS_MESG_BLOCK);

    schedule_task(&gfxTasks[g_curGfxContext]);
    
    // Switch to the next context
    g_curGfxContext = (g_curGfxContext + 1) % num_frame_buffers;
}

const Gfx rdpInitDL[] = {
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_1CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_NONE | G_ZS_PIXEL | G_RM_OPA_SURF | G_RM_OPA_SURF2),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, border_height, screen_width, screen_height - border_height),
    gsDPSetCombineLERP(ENVIRONMENT, 0, SHADE, 0, 0, 0, 0, 1, ENVIRONMENT, 0, SHADE, 0, 0, 0, 0, 1),
    gsDPSetEnvColor(0xFF, 0xFF, 0xFF, 0xFF),
    gsDPSetPrimColor(0, 0, 0x00, 0xFF, 0x00, 0xFF),
    gsSPEndDisplayList(),
};

const Gfx clearDepthBuffer[] = {
	gsDPSetCycleType(G_CYC_FILL),
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, screen_width, g_depthBuffer.data()),

    gsDPSetFillColor(GPACK_ZDZ(G_MAXFBZ, 0) << 16 | GPACK_ZDZ(G_MAXFBZ, 0)),
    gsDPFillRectangle(0, 0, screen_width - 1, screen_height - 1),
    gsDPPipeSync(),
    gsDPSetDepthImage(g_depthBuffer.data()),
    gsSPEndDisplayList(),
};

u32 fillColor = GPACK_RGBA5551(0, 0, 0, 1) << 16 | GPACK_RGBA5551(0, 0, 0, 1);

void start_frame(void)
{
    resetGfxFrame();

    Gfx *dlist = g_dlist_head;

    gSPSegment(dlist++, 0x00, 0x00000000);
    gSPSegment(dlist++, BUFFER_SEGMENT, g_frameBuffers[g_curGfxContext].data());
    gSPDisplayList(dlist++, rdpInitDL);
    gSPDisplayList(dlist++, clearDepthBuffer);
    
    gDPSetCycleType(dlist++, G_CYC_FILL);
    gDPSetColorImage(dlist++, G_IM_FMT_RGBA, G_IM_SIZ_16b, screen_width, BUFFER_SEGMENT << 24);
    gDPSetFillColor(dlist++, fillColor);
    gDPFillRectangle(dlist++, 0, 0, screen_width - 1, screen_height - 1);
    gDPPipeSync(dlist++);
    
    gDPPipeSync(dlist++);
    gDPSetCycleType(dlist++, G_CYC_1CYCLE);

    g_dlist_head = dlist;
}

void end_frame()
{
    // Disable perspective correction
    gSPPerspNormalize(g_dlist_head++, 0xFFFF);
    // Turn off z buffering
    gSPClearGeometryMode(g_dlist_head++, G_ZBUFFER);
    // Terminate and append the gui displaylist
    gSPEndDisplayList(g_gui_dlist_head++);
    gSPDisplayList(g_dlist_head++, &g_gfxContexts[g_curGfxContext].gui_dlist_buffer[0]);

    gDPFullSync(g_dlist_head++);
    gSPEndDisplayList(g_dlist_head++);

    sendGfxTask();
}
