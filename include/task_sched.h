#ifndef __SCHED_H__
#define __SCHED_H__

#include <PR/sched.h>

void scheduler_init(void);
void schedule_task(OSScTask* task);
void waitForGfxTaskComplete();

#endif