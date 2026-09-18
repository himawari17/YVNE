#ifndef _TIMER_H
#define _TIMER_H
#include "types.h"
#include <SDL3/SDL_timer.h>

typedef enum{
  YVNE_TIMER_IDLE=0,
  YVNE_TIMER_RUNNING,
  YVNE_TIMER_PAUSED,
  YVNE_TIMER_STOPPED,
} TimerState;

typedef enum{
  YVNE_PAUSE_USER,
  YVNE_PAUSE_FOCUS_LOST,
  YVNE_PAUSE_OS,
  YVNE_PAUSE_LOADING,
  YVNE_PAUSE_COUNT,
} TimerPauseReason;

typedef struct{
  u64 started_at;
  u64 current_ticks;
  u64 previous_ticks;

  u64 sdl_init_time;
  u64 elapsed;

  u64 paused_start;
  u64 paused_end;
  u64 on_pause;

  TimerState state;

} YVNE_Timer;

void YVNE_TimerInit(YVNE_Timer *timer, u64 global_time);
u64 YVNE_TimerElapsed(const YVNE_Timer *timer, u64 global_time);
void YVNE_TimerPause(YVNE_Timer *timer, u64 global_time);
void YVNE_TimerResume(YVNE_Timer *timer, u64 global_time);
void YVNE_TimerFinish(YVNE_Timer *timer, u64 global_time);


#endif
