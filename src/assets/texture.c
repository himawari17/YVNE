#include "texture.h"

#define STBI_NO_STDIO
#define STBI_MAX_DIMENSIONS YVNE_TEXTURE_MAX_DIMENSION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static SDL_Surface *YVNE_CreatePlaceholder(void);

typedef struct {
  YVNE_Stream *stream;
  bool failed;
} YVNE_ImageIO;

static int YVNE_ImageRead(void *user, char *data, int size) {
  YVNE_ImageIO *io = user;
  const YVNE_StreamReadResult read = YVNE_StreamRead(io->stream, data, size);
  io->failed |= read.status == YVNE_STREAM_READ_ERROR;
  return (int)read.bytes_read;
}

static void YVNE_ImageSkip(void *user, int offset) {
  YVNE_ImageIO *io = user;
  if (!YVNE_StreamSeek(io->stream, offset, YVNE_STREAM_SEEK_CURRENT, NULL)) {
    io->failed = true;
  }
}

static int YVNE_ImageEOF(void *user) {
  YVNE_ImageIO *io = user;
  int64_t position;
  if (io->failed ||
      !YVNE_StreamSeek(io->stream, 0, YVNE_STREAM_SEEK_CURRENT, &position)) {
    io->failed = true;
    return 1;
  }
  return position == YVNE_StreamGetSize(io->stream);
}

static const stbi_io_callbacks YVNE_ImageCallbacks = {
    .read = YVNE_ImageRead,
    .skip = YVNE_ImageSkip,
    .eof = YVNE_ImageEOF,
};

YVNE_Texture *YVNE_TextureLoad(const YVNE_DirectoryMount *mount,
                               YVNE_AssetId asset_id) {
  YVNE_Texture *texture = SDL_calloc(1, sizeof(YVNE_Texture));
  if (!texture) {
    YVNE_LOG_ERROR("UNABLE TO ALLOCATE TEXTURE STRUCT. SDL_Error: %s",
                   SDL_GetError());

    return NULL;
  }

  SDL_Surface *surface = YVNE_CreatePlaceholder();
  if (!surface) {
    SDL_free(texture);
    YVNE_LOG_ERROR("UNABLE TO CREATE PLACEHOLDER. ASSET ID: %u", asset_id);
    return NULL;
  }
  texture->id = asset_id;
  texture->surface = surface;

  if (YVNE_DirectoryMountGetAssetType(mount, asset_id) != YVNE_RESOURCE_IMAGE) {
    YVNE_LOG_ERROR("ASSET: %u IS NOT AN IMAGE!", asset_id);
    return texture;
  }
  YVNE_Stream *stream = YVNE_DirectoryMountOpen(mount, asset_id);
  if (!stream) {
    YVNE_LOG_ERROR("UNABLE TO OPEN ASSET: %u", asset_id);
    return texture;
  }
  YVNE_ImageIO io = {.stream = stream};
  int width = 0;
  int height = 0;
  int channels = 0;
  if (!stbi_info_from_callbacks(&YVNE_ImageCallbacks, &io,
                                &width, &height, &channels) ||
      io.failed || width <= 0 || height <= 0 ||
      width > (int)YVNE_TEXTURE_MAX_DIMENSION ||
      height > (int)YVNE_TEXTURE_MAX_DIMENSION) {
    YVNE_LOG_ERROR("INVALID IMAGE HEADER OR DIMENSIONS. ASSET ID: %u", asset_id);
    YVNE_StreamClose(&stream);
    return texture;
  }
  if (!YVNE_StreamSeek(stream, 0, YVNE_STREAM_SEEK_BEGIN, NULL)) {
    YVNE_LOG_ERROR("UNABLE TO REWIND IMAGE %u: %s", asset_id, SDL_GetError());
    YVNE_StreamClose(&stream);
    return texture;
  }
  int decoded_width = 0;
  int decoded_height = 0;
  stbi_uc *pixels = stbi_load_from_callbacks(
      &YVNE_ImageCallbacks, &io, &decoded_width, &decoded_height,
      NULL, STBI_rgb_alpha);
  YVNE_StreamClose(&stream);
  if (!pixels || io.failed || decoded_width != width || decoded_height != height) {
    YVNE_LOG_ERROR("UNABLE TO DECODE ASSET: %u", asset_id);
    stbi_image_free(pixels);
    return texture;
  }
  SDL_Surface *converted = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
  if (!converted) {
    stbi_image_free(pixels);
    YVNE_LOG_WARN("UNABLE TO CREATE SURFACE FOR IMAGE %u: %s",
                  asset_id, SDL_GetError());
    return texture;
  }
  for (int row = 0; row < height; ++row) {
    SDL_memcpy((u8 *)converted->pixels + row * converted->pitch,
               pixels + (size_t)row * width * 4, (size_t)width * 4);
  }
  stbi_image_free(pixels);
  if (!SDL_PremultiplySurfaceAlpha(converted, false)) {
    YVNE_LOG_WARN("UNABLE TO PREMULTIPLY IMAGE %u: %s", asset_id, SDL_GetError());
    SDL_DestroySurface(converted);
    return texture;
  }
  SDL_DestroySurface(surface);
  texture->surface = converted;
  return texture;
}

void YVNE_TextureDestroy(YVNE_Texture **asset) {
  if (!asset || !*asset) {
    return;
  }
  SDL_DestroySurface((*asset)->surface);
  SDL_free(*asset);
  *asset = NULL;
}
static SDL_Surface *YVNE_CreatePlaceholder(void) {
  SDL_Surface *surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_RGBA32);
  if (surface == NULL) {
    return NULL;
  }

  static const u8 pixels[2][2][4] = {
      {{255, 0, 255, 255}, {0, 0, 0, 255}},
      {{0, 0, 0, 255}, {255, 0, 255, 255}},
  };
  for (int row = 0; row < 2; ++row) {
    SDL_memcpy((u8 *)surface->pixels + row * surface->pitch, pixels[row],
               sizeof pixels[row]);
  }
  return surface;
}
