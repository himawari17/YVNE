#include "../src/assets/texture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <assert.h>
#include <stdio.h>

static void create_alpha_png(const char *path)
{
    const u8 pixels[] = {200, 100, 50, 128, 10, 20, 30, 0};
    assert(stbi_write_png(path, 1, 2, 4, pixels, 4));
}

static void create_wide_images(const char *png_path, const char *jpeg_path,
                               const char *limit_path)
{
    const int width = YVNE_TEXTURE_MAX_DIMENSION + 1;
    u8 *pixels = SDL_calloc(width, 4);
    assert(pixels != NULL);
    assert(stbi_write_png(png_path, width, 1, 4, pixels, width * 4));
    assert(stbi_write_jpg(jpeg_path, width, 1, 4, pixels, 90));
    assert(stbi_write_png(limit_path, width - 1, 1, 4, pixels, (width - 1) * 4));
    SDL_free(pixels);
}

static void create_truncated_png(const char *source_path, const char *path)
{
    // Keep IHDR and the IDAT header so metadata succeeds but decoding fails.
    u8 header[41];
    SDL_IOStream *source = SDL_IOFromFile(source_path, "rb");
    assert(source != NULL);
    assert(SDL_ReadIO(source, header, sizeof header) == sizeof header);
    assert(SDL_CloseIO(source));
    SDL_IOStream *file = SDL_IOFromFile(path, "wb");
    assert(file != NULL);
    assert(SDL_WriteIO(file, header, sizeof header) == sizeof header);
    assert(SDL_CloseIO(file));
}

int main(void)
{
    char names[][20] = {
        "alpha.png", "oversized.png", "missing.png", "color.jpg",
        "truncated.png", "limit.png", "oversized.jpg",
    };
    char paths[7][1024];
    for (size_t i = 0; i < 7; ++i) {
        const int length = snprintf(paths[i], sizeof paths[i], "%s/%s",
                                    YVNE_TEST_BINARY_DIR, names[i]);
        assert(length > 0 && (size_t)length < sizeof paths[i]);
    }
    create_alpha_png(paths[0]);
    create_wide_images(paths[1], paths[6], paths[5]);
    const u8 jpeg_pixel[] = {200, 100, 50};
    assert(stbi_write_jpg(paths[3], 1, 1, 3, jpeg_pixel, 100));
    create_truncated_png(paths[0], paths[4]);

    YVNE_AssetDescription descriptions[] = {
        {.id = 1, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[0]), .str = names[0]}},
        {.id = 2, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[1]), .str = names[1]}},
        {.id = 3, .type = YVNE_RESOURCE_AUDIO,
         .path = {.size = SDL_strlen(names[0]), .str = names[0]}},
        {.id = 4, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[2]), .str = names[2]}},
        {.id = 5, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[3]), .str = names[3]}},
        {.id = 6, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[4]), .str = names[4]}},
        {.id = 7, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[5]), .str = names[5]}},
        {.id = 8, .type = YVNE_RESOURCE_IMAGE,
         .path = {.size = SDL_strlen(names[6]), .str = names[6]}},
    };
    YVNE_Manifest manifest = {
        .manifest_version = YVNE_MANIFEST_VERSION,
        .asset_count = sizeof descriptions / sizeof descriptions[0],
        .assets = descriptions,
    };
    YVNE_DirectoryMount *mount = YVNE_DirectoryMountCreate(
        YVNE_TEST_BINARY_DIR,
        &manifest,
        1024u * 1024u
    );
    assert(mount != NULL);

    YVNE_Texture *asset = YVNE_TextureLoad(mount, 1);
    assert(asset != NULL && asset->id == 1);
    assert(asset->surface->format == SDL_PIXELFORMAT_RGBA32);
    assert(asset->surface->w == 1 && asset->surface->h == 2);
    const u8 *pixel = asset->surface->pixels;
    assert(pixel[0] == 100 && pixel[1] == 50 && pixel[2] == 25);
    assert(pixel[3] == 128);
    const u8 *second_row = pixel + asset->surface->pitch;
    assert(second_row[0] == 0 && second_row[1] == 0 && second_row[2] == 0);
    assert(second_row[3] == 0);
    YVNE_TextureDestroy(&asset);
    assert(asset == NULL);
    YVNE_TextureDestroy(&asset);
    YVNE_TextureDestroy(NULL);

    asset = YVNE_TextureLoad(mount, 5);
    assert(asset != NULL && asset->id == 5);
    assert(asset->surface->format == SDL_PIXELFORMAT_RGBA32);
    assert(asset->surface->w == 1 && asset->surface->h == 1);
    pixel = asset->surface->pixels;
    for (size_t i = 0; i < sizeof jpeg_pixel; ++i) {
        assert(SDL_abs((int)pixel[i] - jpeg_pixel[i]) <= 5);
    }
    assert(pixel[3] == 255);
    YVNE_TextureDestroy(&asset);

    asset = YVNE_TextureLoad(mount, 7);
    assert(asset != NULL);
    assert(asset->surface->w == (int)YVNE_TEXTURE_MAX_DIMENSION);
    assert(asset->surface->h == 1);
    YVNE_TextureDestroy(&asset);

    const YVNE_AssetId rejected[] = {2, 3, 4, 6, 8, UINT32_MAX};
    for (size_t i = 0; i < sizeof rejected / sizeof rejected[0]; ++i) {
        asset = YVNE_TextureLoad(mount, rejected[i]);
        assert(asset != NULL && asset->id == rejected[i]);
        assert(asset->surface->format == SDL_PIXELFORMAT_RGBA32);
        assert(asset->surface->w == 2 && asset->surface->h == 2);
        pixel = asset->surface->pixels;
        assert(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255);
        assert(pixel[3] == 255);
        YVNE_TextureDestroy(&asset);
    }

    YVNE_DirectoryMountDestroy(&mount);
    asset = YVNE_TextureLoad(NULL, 1);
    assert(asset != NULL && asset->id == 1);
    assert(asset->surface->w == 2 && asset->surface->h == 2);
    YVNE_TextureDestroy(&asset);
    for (size_t i = 0; i < 7; ++i) {
        if (i != 2) {
            assert(SDL_RemovePath(paths[i]));
        }
    }
    return 0;
}
