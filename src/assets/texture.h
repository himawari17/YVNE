#ifndef YVNE_TEXTURE_H
#define YVNE_TEXTURE_H

#include "../core/logger.h"
#include "../resource/directory_mount.h"

#include <SDL3/SDL.h>

#define YVNE_TEXTURE_MAX_DIMENSION 4096u

typedef struct {
  YVNE_AssetId id;
  SDL_Surface *surface;
} YVNE_Texture;

/* Loads RGBA32 pixels with alpha premultiplied in sRGB space, or a placeholder. */
YVNE_Texture *YVNE_TextureLoad(const YVNE_DirectoryMount *mount,
                               YVNE_AssetId asset_id);

/* Destroys YVNE_Texture*/
void YVNE_TextureDestroy(YVNE_Texture **asset);

#endif
