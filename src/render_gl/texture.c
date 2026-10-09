#include "texture.h"
#include "../assets/texture.h"
#include "../core/logger.h"

YVNE_GLTexture *YVNE_GLTextureCreate(const SDL_Surface *surface) {
  if (!surface || !surface->pixels || surface->format != SDL_PIXELFORMAT_RGBA32 ||
      surface->w <= 0 || surface->h <= 0 ||
      surface->w > (int)YVNE_TEXTURE_MAX_DIMENSION ||
      surface->h > (int)YVNE_TEXTURE_MAX_DIMENSION ||
      surface->pitch <= 0 || surface->pitch % 4 != 0 ||
      (size_t)surface->pitch < (size_t)surface->w * 4 ||
      (SDL_MUSTLOCK(surface) && !(surface->flags & SDL_SURFACE_LOCKED))) {
    YVNE_LOG_ERROR("INVALID OR INACCESSIBLE RGBA32 SURFACE");
    return NULL;
  }
  if (!SDL_GL_GetCurrentContext() || !GLAD_GL_VERSION_3_3) {
    YVNE_LOG_ERROR("NO CURRENT OPENGL 3.3 CONTEXT");
    return NULL;
  }
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    YVNE_LOG_ERROR("GL ERROR BEFORE TEXTURE CREATION: 0x%x", error);
    return NULL;
  }
  GLint maximum_size;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximum_size);
  if (surface->w > maximum_size || surface->h > maximum_size) {
    YVNE_LOG_ERROR("TEXTURE EXCEEDS OPENGL SIZE LIMIT");
    return NULL;
  }
  YVNE_GLTexture *texture = SDL_calloc(1, sizeof(YVNE_GLTexture));
  if (!texture) {
    YVNE_LOG_ERROR("UNABLE TO ALLOCATE YVNE_GLTEXTURE!");
    return NULL;
  }

  GLint old_texture, old_buffer;
  GLint old_alignment, old_row_length, old_skip_pixels, old_skip_rows;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_texture);
  glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &old_buffer);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_alignment);
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &old_row_length);
  glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &old_skip_pixels);
  glGetIntegerv(GL_UNPACK_SKIP_ROWS, &old_skip_rows);

  glGenTextures(1, &texture->handle);
  glBindTexture(GL_TEXTURE_2D, texture->handle);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / 4);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, surface->pixels);

  glPixelStorei(GL_UNPACK_ALIGNMENT, old_alignment);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, old_row_length);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, old_skip_pixels);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, old_skip_rows);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)old_buffer);
  glBindTexture(GL_TEXTURE_2D, (GLuint)old_texture);

  error = glGetError();
  if (!texture->handle || error != GL_NO_ERROR) {
    YVNE_LOG_ERROR("TEXTURE UPLOAD FAILED. GL ERROR: 0x%x", error);
    YVNE_GLTextureDestroy(&texture);
    return NULL;
  }

  texture->width = surface->w;
  texture->height = surface->h;
  YVNE_LOG_DEBUG("GL TEXTURE UPLOADED: HANDLE %u, %dx%d",
                  texture->handle, texture->width, texture->height);
  return texture;
}

void YVNE_GLTextureDestroy(YVNE_GLTexture **texture) {
  if (!texture || !*texture) {
    return;
  }
  glDeleteTextures(1, &(*texture)->handle);
  SDL_free(*texture);
  *texture = NULL;
}
