#ifndef YVNE_VIEWPORT_H
#define YVNE_VIEWPORT_H

#include <stdbool.h>

typedef struct {
  int canvas_width, canvas_height;
  int framebuffer_width, framebuffer_height;
  int x, y, width, height;
} YVNE_Viewport;

/* Viewport origin is the top-left corner, in framebuffer pixels. */
bool YVNE_ViewportCalculate(YVNE_Viewport *viewport,
                            int canvas_width, int canvas_height,
                            int framebuffer_width, int framebuffer_height);

/* Returns false for invalid sizes, non-finite coordinates or clicks in bars. */
bool YVNE_ViewportWindowToCanvas(const YVNE_Viewport *viewport,
                                 int window_width, int window_height,
                                 float window_x, float window_y,
                                 float *canvas_x, float *canvas_y);

#endif
