#include "frame_clock.h"


void YVNE_FrameClockInit(YVNE_FrameClock *clock, u64 now){
  clock->previous_frame = now;
}
u64 YVNE_FrameClockTick(YVNE_FrameClock *clock, u64 now){
  const u64 delta = now - clock->previous_frame;
  clock->previous_frame = now;
  if(delta > YVNE_FRAME_CLOCK_MAX_DELTA){
	return YVNE_FRAME_CLOCK_MAX_DELTA;
  }
  return delta;
}
