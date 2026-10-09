#include "../src/platform/platform.h"
#include "../src/platform/window.h"
#include "../src/render_gl/renderer.h"
#include "../src/assets/texture.h"

#include <assert.h>
#include <limits.h>

static void check_pixel(int x, int y, const u8 expected[4]) {
  u8 pixel[4];
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  assert(glGetError() == GL_NO_ERROR);
  for (size_t channel = 0; channel < 4; ++channel) {
    assert(SDL_abs((int)pixel[channel] - expected[channel]) <= 2);
  }
}

static void check_canvas_pixel(const YVNE_Viewport *viewport, float x, float y,
                               const u8 expected[4]) {
  const int pixel_x = viewport->x + (int)(x * viewport->width / viewport->canvas_width);
  const int pixel_y = viewport->framebuffer_height - 1 - viewport->y -
                      (int)(y * viewport->height / viewport->canvas_height);
  check_pixel(pixel_x, pixel_y, expected);
}

int main(void) {
  assert(YVNE_GLRendererCreate(8, 4) == NULL);
  assert(YVNE_GLTextureCreate(NULL) == NULL);
  YVNE_GLRendererDestroy(NULL);
  YVNE_GLTextureDestroy(NULL);

  assert(YVNE_SDLInit());
  YVNEWindow *window = YVNE_WindowCreate(320, 240, "YVNE renderer test");
  assert(window != NULL);
  assert(SDL_HideWindow(window->window));
  assert(YVNE_GLRendererCreate(0, 4) == NULL);
  assert(YVNE_GLRendererCreate(8, -1) == NULL);

  YVNE_Renderer *renderer = YVNE_GLRendererCreate(8, 4);
  assert(renderer != NULL);
  assert(glGetError() == GL_NO_ERROR);

  const u8 red[] = {255, 0, 0, 255};
  const u8 green[] = {0, 255, 0, 255};
  const u8 blue[] = {0, 0, 255, 255};
  const u8 yellow[] = {255, 255, 0, 255};
  const u8 black[] = {0, 0, 0, 255};
  u8 pixels[] = {
      255, 0, 0, 255, 0, 255, 0, 255, 13, 14, 15, 16,
      0, 0, 255, 255, 255, 255, 0, 255, 17, 18, 19, 20,
  };
  SDL_Surface *surface = SDL_CreateSurfaceFrom(2, 2, SDL_PIXELFORMAT_RGBA32, pixels, 12);
  assert(surface != NULL);
  SDL_Surface invalid = *surface;
  invalid.pitch = 7;
  assert(YVNE_GLTextureCreate(&invalid) == NULL);
  invalid = *surface;
  invalid.w = YVNE_TEXTURE_MAX_DIMENSION + 1;
  assert(YVNE_GLTextureCreate(&invalid) == NULL);
  invalid = *surface;
  invalid.format = SDL_PIXELFORMAT_RGB24;
  assert(YVNE_GLTextureCreate(&invalid) == NULL);

  GLuint previous_texture, unpack_buffer;
  glGenTextures(1, &previous_texture);
  glBindTexture(GL_TEXTURE_2D, previous_texture);
  glGenBuffers(1, &unpack_buffer);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack_buffer);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, 128, NULL, GL_STATIC_DRAW);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 5);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 1);

  YVNE_GLTexture *texture = YVNE_GLTextureCreate(surface);
  assert(texture != NULL && texture->width == 2 && texture->height == 2);
  GLint value;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &value);
  assert((GLuint)value == previous_texture);
  glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &value);
  assert((GLuint)value == unpack_buffer);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &value);
  assert(value == 8);
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &value);
  assert(value == 5);
  glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &value);
  assert(value == 1);
  glGetIntegerv(GL_UNPACK_SKIP_ROWS, &value);
  assert(value == 1);
  SDL_DestroySurface(surface);

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glDeleteBuffers(1, &unpack_buffer);
  glDeleteTextures(1, &previous_texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glBindTexture(GL_TEXTURE_2D, texture->handle);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &value);
  assert(value == GL_LINEAR);
  u8 uploaded[16];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, uploaded);
  assert(SDL_memcmp(uploaded, pixels, 8) == 0);
  assert(SDL_memcmp(uploaded + 8, pixels + 12, 8) == 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  u8 alpha_pixel[] = {128, 0, 0, 128};
  surface = SDL_CreateSurfaceFrom(1, 1, SDL_PIXELFORMAT_RGBA32, alpha_pixel, 4);
  assert(surface != NULL);
  YVNE_GLTexture *alpha_texture = YVNE_GLTextureCreate(surface);
  assert(alpha_texture != NULL);
  SDL_DestroySurface(surface);

  for (int frame = 0; frame < 2; ++frame) {
    if (frame == 1) {
      assert(SDL_SetWindowSize(window->window, 401, 301));
      assert(SDL_SyncWindow(window->window));
      SDL_PumpEvents();
    }
    int width, height;
    assert(SDL_GetWindowSizeInPixels(window->window, &width, &height));
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    assert(YVNE_GLRendererOnFrame(renderer, width, height));
    const YVNE_Viewport *viewport = YVNE_GLRendererGetViewport(renderer);
    assert(viewport != NULL && viewport->canvas_width == 8 && viewport->canvas_height == 4);
    assert(viewport->height < height);
    YVNE_GLRendererDraw(renderer, texture, 0, 0, 8, 4);
    check_canvas_pixel(viewport, 2, 1, red);
    check_canvas_pixel(viewport, 6, 1, green);
    check_canvas_pixel(viewport, 2, 3, blue);
    check_canvas_pixel(viewport, 6, 3, yellow);
    check_pixel(0, 0, black);

    YVNE_GLRendererDraw(renderer, alpha_texture, 4, 0, 4, 2);
    const u8 blended[] = {128, 127, 0, 255};
    check_canvas_pixel(viewport, 6, 1, blended);
    assert(SDL_GL_SwapWindow(window->window));
  }

  assert(!YVNE_GLRendererOnFrame(renderer, 0, 0));
  assert(YVNE_GLRendererGetViewport(renderer)->width == 0);
  assert(!YVNE_GLRendererOnFrame(renderer, INT_MAX, INT_MAX));
  assert(YVNE_GLRendererGetViewport(renderer)->width == 0);
  assert(YVNE_GLRendererGetViewport(NULL) == NULL);
  const GLuint texture_handle = texture->handle;
  YVNE_GLTextureDestroy(&texture);
  assert(texture == NULL && !glIsTexture(texture_handle));
  YVNE_GLTextureDestroy(&texture);
  YVNE_GLTextureDestroy(&alpha_texture);

  GLint program, vao, vbo;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
  YVNE_GLRendererDestroy(&renderer);
  assert(renderer == NULL);
  assert(!glIsProgram((GLuint)program));
  assert(!glIsVertexArray((GLuint)vao));
  assert(!glIsBuffer((GLuint)vbo));
  YVNE_GLRendererDestroy(&renderer);
  assert(glGetError() == GL_NO_ERROR);

  renderer = YVNE_GLRendererCreate(1920, 1080);
  assert(renderer != NULL);
  assert(YVNE_GLRendererOnFrame(renderer, 1280, 720));
  assert(YVNE_GLRendererGetViewport(renderer)->width == 1280);
  YVNE_GLRendererDestroy(&renderer);
  YVNE_WindowDestroy(window);
  YVNE_SDLQuit();
  return 0;
}
