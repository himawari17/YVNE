#include "viewport.h"

#include <math.h>
#include <stdint.h>

bool YVNE_ViewportCalculate(YVNE_Viewport *viewport,
                            int canvas_width, int canvas_height,
                            int framebuffer_width, int framebuffer_height) {
  if (!viewport) {
    return false;
  }
  *viewport = (YVNE_Viewport){0};
  if (canvas_width <= 0 || canvas_height <= 0 ||
      framebuffer_width <= 0 || framebuffer_height <= 0) {
    return false;
  }

  int width, height;
  if ((int64_t)framebuffer_width * canvas_height <=
      (int64_t)framebuffer_height * canvas_width) {
    width = framebuffer_width;
    height = (int)((int64_t)width * canvas_height / canvas_width);
  } else {
    height = framebuffer_height;
    width = (int)((int64_t)height * canvas_width / canvas_height);
  }
  if (width == 0 || height == 0) {
    return false;
  }

  *viewport = (YVNE_Viewport){
      .canvas_width = canvas_width,
      .canvas_height = canvas_height,
      .framebuffer_width = framebuffer_width,
      .framebuffer_height = framebuffer_height,
      .x = (framebuffer_width - width) / 2,
      .y = (framebuffer_height - height) / 2,
      .width = width,
      .height = height,
  };
  return true;
}

bool YVNE_ViewportWindowToCanvas(const YVNE_Viewport *viewport,
                                 int window_width, int window_height,
                                 float window_x, float window_y,
                                 float *canvas_x, float *canvas_y) {
  if (!viewport || !canvas_x || !canvas_y ||
      window_width <= 0 || window_height <= 0 ||
      viewport->canvas_width <= 0 || viewport->canvas_height <= 0 ||
      viewport->framebuffer_width <= 0 || viewport->framebuffer_height <= 0 ||
      viewport->width <= 0 || viewport->height <= 0 ||
      viewport->width > viewport->framebuffer_width ||
      viewport->height > viewport->framebuffer_height ||
      viewport->x < 0 || viewport->y < 0 ||
      viewport->x > viewport->framebuffer_width - viewport->width ||
      viewport->y > viewport->framebuffer_height - viewport->height ||
      !isfinite(window_x) || !isfinite(window_y)) {
    return false;
  }

  const double pixel_x = (double)window_x * viewport->framebuffer_width / window_width;
  const double pixel_y = (double)window_y * viewport->framebuffer_height / window_height;
  if (pixel_x < viewport->x || pixel_y < viewport->y ||
      pixel_x >= viewport->x + viewport->width ||
      pixel_y >= viewport->y + viewport->height) {
    return false;
  }

  *canvas_x = (float)((pixel_x - viewport->x) * viewport->canvas_width / viewport->width);
  *canvas_y = (float)((pixel_y - viewport->y) * viewport->canvas_height / viewport->height);
  return true;
}
