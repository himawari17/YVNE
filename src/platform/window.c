#include "window.h"
#include "../core/logger.h"
#include "platform.h"
#include <glad/gl.h>

static GLADapiproc YVNE_GLGetProc(const char *name) {
  return (GLADapiproc)SDL_GL_GetProcAddress(name);
}

YVNEWindow *YVNE_WindowCreate(u16 w, u16 h, const char *t) {
  YVNEWindow *window = calloc(1, sizeof *window);
  if (!window) {
    YVNE_LOG_ERROR("UNABLE TO ALLOCATE WINDOW MEMORY!");
    free(window);
    YVNE_SDLQuit();
    return nullptr;
  }
  window->window = SDL_CreateWindow(
      t, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (!window->window) {
    YVNE_LOG_ERROR("UNABLE TO ATTACH SDL WINDOW! SDL ERROR: %s",
                   SDL_GetError());
    SDL_DestroyWindow(window->window);
    free(window);
    return nullptr;
  }
  SDL_GLContext gl_context = SDL_GL_CreateContext(window->window);
  if (!gl_context) {
    SDL_DestroyWindow(window->window);
    free(window);
    YVNE_LOG_ERROR("UNABLE TO CREATE GL_CONTEXT! SDL ERROR: %s",
                   SDL_GetError());
    return nullptr;
  }
  window->gl_context = gl_context;
  if (!SDL_GL_MakeCurrent(window->window, window->gl_context)) {
    YVNE_LOG_ERROR("FAILED TO ATTACH GL CONTEXT TO WINDOW! SDL ERROR: %s",
                   SDL_GetError());
    if (!SDL_GL_DestroyContext(window->gl_context)) {
      YVNE_LOG_ERROR("UNABLE TO DESTROY GL CONTEXT! SDL ERROR: %s",
                     SDL_GetError());
    }
    SDL_DestroyWindow(window->window);

    free(window);
    return nullptr;
  }

  const u64 gl_version = gladLoadGL(YVNE_GLGetProc);

  if (gl_version == 0 || !GLAD_GL_VERSION_3_3) {
    YVNE_LOG_ERROR("OPENGL 3.3 CORE IS NOT AVAILABLE");
    YVNE_WindowDestroy(window);
    return nullptr;
  }
  YVNE_GetPlatformInfo();
  SDL_SetWindowPosition(window->window, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);

  window->height = h;
  window->width = w;

  YVNE_LOG_INFO("WINDOW READY: %dx%d, RESIZABLE", w, h);
  return window;
}

void YVNE_WindowDestroy(YVNEWindow *w) {
  if (!w) {
    YVNE_LOG_WARN("TRYING TO DESTROY EMPTY WINDOW");
    return;
  }
  if (w->gl_context) {
    if (!SDL_GL_DestroyContext(w->gl_context)) {
      YVNE_LOG_ERROR("UNABLE TO DESTROY GL CONTEXT! SDL ERROR: %s",
                     SDL_GetError());
    }
  }
  if (w->window) {
    SDL_DestroyWindow(w->window);
  }
  free(w);
  YVNE_LOG_INFO("WINDOW DESTROYED SUCCESSFULLY");
}

void YVNE_WindowUpdate(YVNEWindow *w);
