#ifndef _OSINT_H_
#define _OSINT_H_

/*
 * Internal scheduler declarations used by matched SDK translation units.
 * PROVENANCE: names and ABI follow public libultra headers in the permitted
 * JFG decompilation; layouts are limited to fields Mickey's code accesses.
 */

#include "PR/os_internal.h"
#include "PR/os_message.h"

typedef struct {
    OSMesgQueue *messageQueue;
    OSMesg message;
} __OSEventState;

typedef s32 OSEvent;
#define OS_NUM_EVENTS 16

typedef struct OSTimer_s {
    struct OSTimer_s *next;
    struct OSTimer_s *prev;
    OSTime interval;
    OSTime value;
    OSMesgQueue *mq;
    OSMesg msg;
} OSTimer;

extern struct __osThreadTail {
    OSThread *next;
    OSPri priority;
} __osThreadTail;

extern OSThread *__osRunningThread;
extern OSThread *__osActiveQueue;
extern OSThread *__osRunQueue;

void __osEnqueueAndYield(OSThread **queue);
void __osDequeueThread(OSThread **queue, OSThread *thread);
void __osEnqueueThread(OSThread **queue, OSThread *thread);
OSThread *__osPopThread(OSThread **queue);
void __osDispatchThread(void);

u32 __osProbeTLB(void *address);

extern OSTime __osCurrentTime;
extern u32 __osBaseCounter;
extern u32 __osViIntrCount;
extern u32 __osTimerCounter;
extern OSTimer *__osTimerList;

void __osSetTimerIntr(OSTime time);
void __osSetCompare(u32 value);
OSTime __osInsertTimer(OSTimer *timer);
OSTime osGetTime(void);
int osSetTimer(OSTimer *timer, OSTime countdown, OSTime interval,
               OSMesgQueue *mq, OSMesg msg);

#endif
