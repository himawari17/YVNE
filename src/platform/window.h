#ifndef _WINDOW_H
#define _WINDOW_H

#include "../core/types.h"
#include <glad/gl.h>
#include "SDL3/SDL.h"
#include <stdlib.h>


typedef struct {
  u16 width;
  u16 height;
  SDL_Window *window;
  SDL_GLContext gl_context;
} YVNEWindow;

YVNEWindow* YVNE_WindowCreate(u16 w, u16 h, const char* t);

void YVNE_WindowDestroy(YVNEWindow *w);

void YVNE_WindowUpdate(YVNEWindow *w);

#endif
