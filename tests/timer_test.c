#include "../src/core/timer.h"

#include <assert.h>

int main(void)
{
    YVNE_Timer timer;
    YVNE_TimerInit(&timer, 100);
    assert(timer.state == YVNE_TIMER_RUNNING);
    assert(YVNE_TimerElapsed(&timer, 100) == 0);
    assert(YVNE_TimerElapsed(&timer, 150) == 50);

    YVNE_TimerResume(&timer, 150);
    assert(YVNE_TimerElapsed(&timer, 150) == 50);
    YVNE_TimerPause(&timer, 160);
    assert(timer.state == YVNE_TIMER_PAUSED);
    assert(YVNE_TimerElapsed(&timer, 500) == 60);
    YVNE_TimerPause(&timer, 200);
    assert(YVNE_TimerElapsed(&timer, 500) == 60);

    YVNE_TimerResume(&timer, 260);
    assert(timer.state == YVNE_TIMER_RUNNING);
    assert(YVNE_TimerElapsed(&timer, 280) == 80);
    YVNE_TimerPause(&timer, 300);
    assert(YVNE_TimerElapsed(&timer, 500) == 100);
    YVNE_TimerResume(&timer, 350);
    assert(YVNE_TimerElapsed(&timer, 400) == 150);
    YVNE_TimerFinish(&timer, 400);
    assert(timer.state == YVNE_TIMER_STOPPED);
    assert(YVNE_TimerElapsed(&timer, 900) == 150);
    YVNE_TimerFinish(&timer, 900);
    YVNE_TimerPause(&timer, 900);
    YVNE_TimerResume(&timer, 900);
    assert(timer.state == YVNE_TIMER_STOPPED);
    assert(YVNE_TimerElapsed(&timer, 1000) == 150);

    YVNE_TimerInit(&timer, 1000);
    assert(YVNE_TimerElapsed(&timer, 1000) == 0);
    YVNE_TimerPause(&timer, 1020);
    YVNE_TimerResume(&timer, 1120);
    YVNE_TimerPause(&timer, 1140);
    YVNE_TimerFinish(&timer, 2000);
    assert(timer.state == YVNE_TIMER_STOPPED);
    assert(YVNE_TimerElapsed(&timer, 3000) == 40);
    return 0;
}
