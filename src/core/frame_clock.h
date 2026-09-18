#ifndef _FRAME_CLOCK_H
#define _FRAME_CLOCK_H
#include "types.h"

#define YVNE_FRAME_CLOCK_MAX_DELTA UINT64_C(100000000)

typedef struct{
  u64 previous_frame;
} YVNE_FrameClock;


void YVNE_FrameClockInit(YVNE_FrameClock *clock, u64 now);
u64 YVNE_FrameClockTick(YVNE_FrameClock *clock, u64 now);

#endif
