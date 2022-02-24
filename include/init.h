#ifndef __N64_INIT_H__
#define __N64_INIT_H__

#define NUM_THREADS 3

#define IDLE_THREAD 1
#define IDLE_THREAD_INDEX (IDLE_THREAD - 1)
#define IDLE_THREAD_STACKSIZE 0x100

#define MAIN_THREAD 2
#define MAIN_THREAD_INDEX (MAIN_THREAD - 1)
#define MAIN_THREAD_STACKSIZE 0x4000
#define MAIN_THREAD_PRI 10

#define SCHEDULER_PRI 13

#define NUM_PI_MESSAGES 8

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)
extern "C" void init(void);
void idle(void*);
#endif

#endif