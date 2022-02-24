// libultra
#include <ultra64.h>

// game code
#include <gfx.h>
#include <task_sched.h>
#include <init.h>
#include <mem.h>
#include <platform.h>

#include <array>

u8 idleThreadStack[IDLE_THREAD_STACKSIZE] alignas(16);
u8 mainThreadStack[MAIN_THREAD_STACKSIZE] alignas(16);

static OSMesgQueue piMesgQueue;
static std::array<OSMesg, NUM_PI_MESSAGES> piMessages;

static std::array<OSThread, NUM_THREADS> g_threads;
OSPiHandle *g_romHandle;

extern "C" void init(void)
{
    bzero(_mainSegmentBssStart, (u32)_mainSegmentBssEnd - (u32)_mainSegmentBssStart);
    osInitialize();

    g_romHandle = osCartRomInit();

    osCreateThread(&g_threads[IDLE_THREAD_INDEX], IDLE_THREAD, idle, nullptr, idleThreadStack + IDLE_THREAD_STACKSIZE, 10);
    osStartThread(&g_threads[IDLE_THREAD_INDEX]);
}

int main(int, char **);

void mainThreadFunc(void *)
{
    main(0, nullptr);
}

void idle(__attribute__ ((unused)) void *arg)
{
    bzero(g_frameBuffers.data(), num_frame_buffers * screen_width * screen_height * sizeof(u16));

    scheduler_init();
    
    // Set up PI
    osCreatePiManager(OS_PRIORITY_PIMGR, &piMesgQueue, piMessages.data(), NUM_PI_MESSAGES);

    // Create the main thread
    osCreateThread(&g_threads[MAIN_THREAD_INDEX], MAIN_THREAD, mainThreadFunc, nullptr, mainThreadStack + MAIN_THREAD_STACKSIZE, MAIN_THREAD_PRI);
    // Start the main thread
    osStartThread(&g_threads[MAIN_THREAD_INDEX]);

    // Set this thread's priority to 0, making it the idle thread
    osSetThreadPri(nullptr, 0);

    // idle
    while (1);
}
