#include <stdint.h>

// libultra
extern "C" {
#include <PR/sched.h>
}

// game code
#include <init.h>
#include <task_sched.h>

static OSSched scheduler;
static OSScClient gfxClient;
static OSMesgQueue schedQueue;
static OSMesg schedMesg;

static u64 schedStack[OS_SC_STACKSIZE / sizeof(u64)];

void scheduler_init(void)
{
    u8 mode;
    switch (osTvType)
    {
        case OS_TV_PAL:
            #ifdef HIGH_RES
            #ifdef INTERLACED
                mode = OS_VI_FPAL_HAN1;
            #else
                mode = OS_VI_FPAL_HAF1;
            #endif
            #else
            #ifdef INTERLACED
                mode = OS_VI_FPAL_LAF1;
            #else
                mode = OS_VI_FPAL_LAN1;
            #endif
            #endif
            break;
        case OS_TV_MPAL:
            #ifdef HIGH_RES
            #ifdef INTERLACED
                mode = OS_VI_MPAL_HAN1;
            #else
                mode = OS_VI_MPAL_HAF1;
            #endif
            #else
            #ifdef INTERLACED
                mode = OS_VI_MPAL_LAF1;
            #else
                mode = OS_VI_MPAL_LAN1;
            #endif
            #endif
            break;
        case OS_TV_NTSC:
        default:
            #ifdef HIGH_RES
            #ifdef INTERLACED
                mode = OS_VI_NTSC_HAN1;
            #else
                mode = OS_VI_NTSC_HAF1;
            #endif
            #else
            #ifdef INTERLACED
                mode = OS_VI_NTSC_LAF1;
            #else
                mode = OS_VI_NTSC_LAN1;
            #endif
            #endif
            break;
    }
    #ifdef FPS30
    osCreateScheduler(&scheduler, &schedStack[OS_SC_STACKSIZE / sizeof(u64)], SCHEDULER_PRI, mode, 2);
    #else
    osCreateScheduler(&scheduler, &schedStack[OS_SC_STACKSIZE / sizeof(u64)], SCHEDULER_PRI, mode, 1);
    #endif
    osCreateMesgQueue(&schedQueue, &schedMesg, 1);
    osScAddClient(&scheduler, &gfxClient, &schedQueue);
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON | OS_VI_GAMMA_ON | OS_VI_GAMMA_DITHER_OFF | OS_VI_DIVOT_ON);
    if (osTvType == OS_TV_PAL)
    {
        osViSetYScale(0.833f); // Thanks pyoro
    }
}

void schedule_task(OSScTask *task)
{
    osSendMesg(&scheduler.cmdQ, (OSMesg)task, OS_MESG_NOBLOCK);
}
