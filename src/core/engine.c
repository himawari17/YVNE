#include "engine.h"
#include "../platform/input.h"
#include "../platform/platform.h"
#include "../platform/window.h"
#include "../render_gl/renderer.h"
#include "frame_clock.h"
#include "logger.h"
#include "timer.h"
#include <glad/gl.h>

struct Engine {
  YVNEWindow *window;

  YVNE_Input *input;
  YVNE_Renderer *renderer;

  // Timer that starts at the beginning of the program;
  YVNE_Timer global_timer;

  // Timer that starts at the beginning of the game;
  YVNE_Timer game_timer;

  YVNE_FrameClock frame_clock;
  bool is_runnign;
};

Engine *YVNE_EngineInit(void) {

  Engine *engine = calloc(1, sizeof *engine);
  if (engine == NULL) {
    YVNE_LOG_ERROR("FAILED TO ALLOCATE MEMORY FOR ENGINE");
    return NULL;
  }

  if (!YVNE_SDLInit()) {
    YVNE_LOG_ERROR("FAILED TO INITIALIZE SDL. SDL ERROR: %s", SDL_GetError());
    free(engine);
    return nullptr;
  }
  engine->is_runnign = true;

  engine->window = YVNE_WindowCreate(800, 600, "Kohaku Interpreter");
  if (!engine->window) {
    YVNE_LOG_ERROR("FAILED TO ATTACH WINDOW TO ENGINE.");
    YVNE_SDLQuit();
    free(engine);
    return nullptr;
  }

  engine->input = YVNE_InputInit();
  if (!engine->input) {
    YVNE_LOG_ERROR("FAILED TO ATTACH INPUT SYSTEM TO ENGINE!");
    YVNE_WindowDestroy(engine->window);
    YVNE_SDLQuit();
    free(engine);
    return nullptr;
  }

  engine->renderer = YVNE_GLRendererCreate(1280, 720);
  if (!engine->renderer) {
    YVNE_InputDestroy(engine->input);
    YVNE_WindowDestroy(engine->window);
    YVNE_SDLQuit();
    free(engine);
    return NULL;
  }

  const u64 global_time = SDL_GetTicksNS();
  YVNE_TimerInit(&engine->global_timer, global_time);
  // YVNE_TimerInit(&engine->game_timer, global_time);

  YVNE_FrameClockInit(&engine->frame_clock, global_time);

  YVNE_LOG_INFO("ENGINE INITIALIZATED SUCCESSFULLY");
  return engine;
}

bool YVNE_EngineRun(Engine *engine) {
  if (!engine) {
    return false;
  }
  YVNE_LOG_INFO("STARTING MAIN LOOP");
  while (engine->is_runnign == true) {
    engine->global_timer.current_ticks = SDL_GetTicksNS();
    YVNE_InputOnFrame(engine->input);
    // u64 global_time = SDL_GetTicksNS();
    // u64 frame_delta =
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT && engine->is_runnign) {
        YVNE_LOG_INFO("QUIT REQUESTED; STOPPING MAIN LOOP");
        engine->is_runnign = false;
      }
      YVNE_InputHandle(engine->input, &event);
    }

    if (!engine->is_runnign) {
      break;
    }
    const u64 now = engine->global_timer.current_ticks;
    YVNE_FrameClockTick(&engine->frame_clock, now);

    int framebuffer_width, framebuffer_height;
    if (!SDL_GetWindowSizeInPixels(engine->window->window,
                                   &framebuffer_width, &framebuffer_height)) {
      YVNE_LOG_ERROR("UNABLE TO GET FRAMEBUFFER SIZE: %s", SDL_GetError());
      return false;
    }
    if (!YVNE_GLRendererOnFrame(engine->renderer, framebuffer_width, framebuffer_height)) {
      if (framebuffer_width > 0 && framebuffer_height > 0) {
        return false;
      }
      SDL_Delay(16);
      continue;
    }

    if (!SDL_GL_SwapWindow(engine->window->window)) {
      YVNE_LOG_ERROR("UNABLE TO PRESENT FRAME: %s", SDL_GetError());
      return false;
    }
  }
  return true;
}

void YVNE_EngineDestroy(Engine **engine) {

  if (!engine || !*engine) {
    YVNE_LOG_WARN("TRYING TO DESTROY EMTPY ENGINE STRUCTURE");
    return;
  }
  Engine *exemplar = *engine;
  YVNE_GLRendererDestroy(&exemplar->renderer);
  if (exemplar->window) {
    YVNE_InputDestroy(exemplar->input);
    YVNE_WindowDestroy(exemplar->window);
  }
  YVNE_SDLQuit();
  free(exemplar);
  *engine = NULL;
  YVNE_LOG_INFO("ENGINE DESTROYED SUCCESSFULLY");
}
