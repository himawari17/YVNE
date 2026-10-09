#include "../src/core/engine.h"
#include "../src/platform/platform.h"
#include "../src/platform/window.h"

#include <assert.h>

int main(void)
{
    assert(YVNE_SDLInit());
    YVNEWindow *window = YVNE_WindowCreate(320, 240, "YVNE smoke test");
    assert(window != NULL);
    assert(SDL_HideWindow(window->window));
    assert(window->width == 320 && window->height == 240);
    assert(SDL_GL_GetCurrentWindow() == window->window);
    assert(SDL_GL_GetCurrentContext() == window->gl_context);
    assert(GLAD_GL_VERSION_3_3);
    int width, height, profile;
    assert(SDL_GetWindowSize(window->window, &width, &height));
    assert(width == 320 && height == 240);
    assert(SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile));
    assert(profile == SDL_GL_CONTEXT_PROFILE_CORE);

    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    u8 pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    assert(glGetError() == GL_NO_ERROR);
    assert(SDL_abs((int)pixel[0] - 64) <= 2);
    assert(SDL_abs((int)pixel[1] - 128) <= 2);
    assert(SDL_abs((int)pixel[2] - 191) <= 2);
    assert(SDL_GL_SwapWindow(window->window));
    YVNE_WindowDestroy(window);
    assert(SDL_GL_GetCurrentContext() == NULL);
    assert(SDL_GL_GetCurrentWindow() == NULL);
    YVNE_SDLQuit();
    assert(SDL_WasInit(0) == 0);

    for (int cycle = 0; cycle < 2; ++cycle) {
        Engine *engine = YVNE_EngineInit();
        assert(engine != NULL);
        assert(SDL_HideWindow(SDL_GL_GetCurrentWindow()));
        SDL_Event event = {.type = SDL_EVENT_QUIT};
        assert(SDL_PushEvent(&event));
        assert(YVNE_EngineRun(engine));
        assert(glGetError() == GL_NO_ERROR);
        YVNE_EngineDestroy(&engine);
        assert(engine == NULL);
        assert(SDL_WasInit(0) == 0);
        YVNE_EngineDestroy(&engine);
    }
    return 0;
}
