#include "../src/core/engine.h"
#include "../src/platform/window.h"
#include "../src/platform/platform.h"

#include <assert.h>

int main(void)
{
    Engine *engine = NULL;
    YVNE_EngineDestroy(NULL);
    YVNE_EngineDestroy(&engine);
    YVNE_WindowDestroy(NULL);

    assert(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    assert(YVNE_SDLInit());
    assert(SDL_WasInit(SDL_INIT_VIDEO) == SDL_INIT_VIDEO);
    assert(YVNE_WindowCreate(320, 240, "failure test") == NULL);
    YVNE_SDLQuit();
    YVNE_SDLQuit();
    assert(SDL_WasInit(0) == 0);

    // The dummy driver initializes video but cannot create an OpenGL window.
    assert(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    engine = YVNE_EngineInit();
    assert(engine == NULL);
    assert(SDL_WasInit(0) == 0);

    assert(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "yvne-invalid-driver", SDL_HINT_OVERRIDE));
    assert(!YVNE_SDLInit());
    assert(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "yvne-invalid-driver", SDL_HINT_OVERRIDE));
    engine = YVNE_EngineInit();
    assert(engine == NULL);
    assert(SDL_WasInit(0) == 0);
    YVNE_SDLQuit();
    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    return 0;
}
