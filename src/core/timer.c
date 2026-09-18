#include "timer.h"
#include "logger.h"

u64 GetPauseTime(YVNE_Timer* timer, u64 global_time);


void YVNE_TimerInit(YVNE_Timer *timer, u64 global_time){
  *timer = (YVNE_Timer) {
	.started_at = global_time,
	.elapsed = 0,
	.on_pause = 0,
	.current_ticks = global_time,
	.state = YVNE_TIMER_RUNNING,
  };
  YVNE_LOG_INFO("TIMER INITIALIZATED SUCCESSFULLY");
}

u64 YVNE_TimerElapsed(const YVNE_Timer *timer, u64 global_time){
  if(timer->state == YVNE_TIMER_RUNNING){
	return (global_time - timer->started_at - timer->on_pause);
  }
  return timer->elapsed - timer->on_pause;
}

void YVNE_TimerPause(YVNE_Timer *timer, u64 global_time){
  if(timer->state != YVNE_TIMER_RUNNING){
	YVNE_LOG_WARN("TRYING TO PAUSE TIMER WHICH IS NOT RUNNING!");
	return;
  }
  timer->paused_start = global_time;
  timer->elapsed = timer->paused_start - timer->started_at;
  timer->state = YVNE_TIMER_PAUSED;
  YVNE_LOG_INFO("TIMER STOPED");
}

void YVNE_TimerResume(YVNE_Timer *timer, u64 global_time){
  if(timer->state != YVNE_TIMER_PAUSED){
	YVNE_LOG_WARN("TRYING TO RESUME TIMER WHICH IS NOT PAUSED!");
	return;
  }
  timer->paused_end = global_time;
  timer->on_pause += timer->paused_end - timer->paused_start;
  timer->state = YVNE_TIMER_RUNNING;
  YVNE_LOG_INFO("TIMER RESUMED");
}

void YVNE_TimerFinish(YVNE_Timer *timer, u64 global_time){
  if(timer->state == YVNE_TIMER_STOPPED){
	YVNE_LOG_WARN("TRYING TO FINISH TIMER WHICH IS NOT RUNNING!");
	return;
  }

  if(timer->state == YVNE_TIMER_RUNNING){
	timer->elapsed = global_time - timer->started_at;
  }

  timer->state = YVNE_TIMER_STOPPED;
  YVNE_LOG_INFO("TIMER FINISHED");
}

