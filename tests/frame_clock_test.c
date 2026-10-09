#include "../src/core/frame_clock.h"

#include <assert.h>

int main(void)
{
    YVNE_FrameClock clock;
    u64 now = UINT64_C(1000000000);
    YVNE_FrameClockInit(&clock, now);
    assert(YVNE_FrameClockTick(&clock, now) == 0);
    now += UINT64_C(16666667);
    assert(YVNE_FrameClockTick(&clock, now) == UINT64_C(16666667));
    now += YVNE_FRAME_CLOCK_MAX_DELTA;
    assert(YVNE_FrameClockTick(&clock, now) == YVNE_FRAME_CLOCK_MAX_DELTA);
    now += UINT64_C(5000000000);
    assert(YVNE_FrameClockTick(&clock, now) == YVNE_FRAME_CLOCK_MAX_DELTA);
    assert(clock.previous_frame == now);
    assert(YVNE_FrameClockTick(&clock, now + 1) == 1);

    YVNE_FrameClockInit(&clock, 0);
    assert(YVNE_FrameClockTick(&clock, 1) == 1);
    return 0;
}
