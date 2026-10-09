#ifndef YVNE_GL_RENDERER_H
#define YVNE_GL_RENDERER_H
#include "texture.h"
#include "viewport.h"

typedef struct YVNE_Renderer YVNE_Renderer;

/* GL functions require the main thread and the owning context to be current. */
YVNE_Renderer *YVNE_GLRendererCreate(int canvas_width, int canvas_height);
/* Checks if we should draw or not. Clears buffer*/
bool YVNE_GLRendererOnFrame(YVNE_Renderer *renderer, int framebuffer_width,
                            int framebuffer_height);

const YVNE_Viewport *YVNE_GLRendererGetViewport(const YVNE_Renderer *renderer);

/* Main render function */
void YVNE_GLRendererDraw(YVNE_Renderer *renderer, const YVNE_GLTexture *texture,
                         float x, float y, float width, float height);
/* Destroys GL renderer */
void YVNE_GLRendererDestroy(YVNE_Renderer **renderer);
#endif
