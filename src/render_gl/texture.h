/* This code works with OpenGL. It converts our YVNE_Texture
 * to OpenGL textures, store them and sent to renderer */
#ifndef YVNE_GL_TEXTURE_H
#define YVNE_GL_TEXTURE_H
#include <SDL3/SDL.h>
#include <glad/gl.h>

/* This needs for OpenGL correct work
 * we convert our YVNE_Texture to OpenGL texture
 * params: width height handle*/
typedef struct {
  int width;
  int height;
  GLuint handle;
} YVNE_GLTexture;

/* Uploads accessible RGBA32 pixels, with premultiplied alpha in sRGB space.
 * Requires the main thread and the owning GL context to be current. */
YVNE_GLTexture *YVNE_GLTextureCreate(const SDL_Surface *surface);

void YVNE_GLTextureDestroy(YVNE_GLTexture **texture);
#endif
