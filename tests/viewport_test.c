#include "../src/render_gl/viewport.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>

static void check_viewport(int canvas_width, int canvas_height,
                           int framebuffer_width, int framebuffer_height,
                           int x, int y, int width, int height) {
  YVNE_Viewport viewport;
  assert(YVNE_ViewportCalculate(&viewport, canvas_width, canvas_height,
                                framebuffer_width, framebuffer_height));
  assert(viewport.x == x && viewport.y == y);
  assert(viewport.width == width && viewport.height == height);
  assert(viewport.canvas_width == canvas_width && viewport.canvas_height == canvas_height);
}

int main(void) {
  check_viewport(1280, 720, 1280, 720, 0, 0, 1280, 720);
  check_viewport(1280, 720, 1920, 1080, 0, 0, 1920, 1080);
  check_viewport(1920, 1080, 1280, 720, 0, 0, 1280, 720);
  check_viewport(1280, 720, 800, 600, 0, 75, 800, 450);
  check_viewport(1280, 720, 1920, 810, 240, 0, 1440, 810);
  check_viewport(1280, 720, 600, 900, 0, 281, 600, 337);
  check_viewport(1280, 720, 801, 601, 0, 75, 801, 450);
  check_viewport(INT_MAX, INT_MAX, INT_MAX, INT_MAX, 0, 0, INT_MAX, INT_MAX);

  YVNE_Viewport viewport;
  assert(YVNE_ViewportCalculate(&viewport, 1280, 720, 1600, 1200));
  float x, y;
  assert(YVNE_ViewportWindowToCanvas(&viewport, 800, 600, 400, 300, &x, &y));
  assert(x == 640.0f && y == 360.0f);
  assert(YVNE_ViewportWindowToCanvas(&viewport, 800, 600, 0, 75, &x, &y));
  assert(x == 0.0f && y == 0.0f);
  assert(YVNE_ViewportWindowToCanvas(&viewport, 800, 600, 799.5f, 524.5f, &x, &y));
  assert(x < 1280.0f && y < 720.0f);

  const float rejected[][2] = {
      {400, 74.5f}, {400, 525}, {800, 300}, {-1, 300}, {NAN, 300}, {400, INFINITY},
  };
  for (size_t i = 0; i < sizeof rejected / sizeof rejected[0]; ++i) {
    x = y = -1;
    assert(!YVNE_ViewportWindowToCanvas(&viewport, 800, 600,
                                        rejected[i][0], rejected[i][1], &x, &y));
    assert(x == -1 && y == -1);
  }
  assert(!YVNE_ViewportWindowToCanvas(&viewport, 0, 600, 0, 0, &x, &y));
  assert(!YVNE_ViewportWindowToCanvas(&viewport, 800, 600, 0, 0, NULL, &y));
  assert(!YVNE_ViewportWindowToCanvas(NULL, 800, 600, 0, 0, &x, &y));

  assert(YVNE_ViewportCalculate(&viewport, 1280, 720, 1920, 810));
  assert(!YVNE_ViewportWindowToCanvas(&viewport, 1920, 810, 239, 405, &x, &y));
  assert(YVNE_ViewportWindowToCanvas(&viewport, 1920, 810, 960, 405, &x, &y));
  assert(x == 640.0f && y == 360.0f);

  assert(!YVNE_ViewportCalculate(&viewport, 1280, 720, 0, 0));
  assert(viewport.width == 0 && viewport.canvas_width == 0);
  assert(!YVNE_ViewportWindowToCanvas(&viewport, 800, 600, 400, 300, &x, &y));
  assert(!YVNE_ViewportCalculate(&viewport, 0, 720, 800, 600));
  assert(!YVNE_ViewportCalculate(&viewport, 1280, 720, -1, 600));
  assert(!YVNE_ViewportCalculate(&viewport, 1280, 720, 1, 1));
  assert(!YVNE_ViewportCalculate(NULL, 1280, 720, 800, 600));
  return 0;
}
