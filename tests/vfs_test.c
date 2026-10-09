#include "../src/resource/directory_mount.h"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    YVNE_Manifest manifest = {0};
    assert(YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR, &manifest));

    YVNE_DirectoryMount *mount = YVNE_DirectoryMountCreate(
        YVNE_TEST_PROJECT_DIR,
        &manifest,
        2u * 1024u * 1024u
    );
    assert(mount != NULL);

    YVNE_Stream *first = YVNE_DirectoryMountOpen(mount, 0);
    YVNE_Stream *second = YVNE_DirectoryMountOpen(mount, 0);
    assert(first != NULL);
    assert(second != NULL);
    assert(YVNE_StreamBorrowSDL(first) != NULL);

    static const char contents_prefix[] = "YVNE";
    char signature[sizeof contents_prefix - 1] = {0};
    YVNE_StreamReadResult read = YVNE_StreamRead(
        first,
        signature,
        sizeof signature
    );
    assert(read.bytes_read == sizeof signature);
    assert(read.status != YVNE_STREAM_READ_ERROR);
    assert(memcmp(signature, contents_prefix, sizeof signature) == 0);
    assert(YVNE_DirectoryMountGetAssetType(mount, 0) == YVNE_RESOURCE_SCRIPT);
    assert(YVNE_DirectoryMountGetAssetType(mount, 1) == YVNE_RESOURCE_IMAGE);
    assert(YVNE_DirectoryMountGetAssetType(mount, UINT32_MAX) == YVNE_RESOURCE_UNKNOWN);
    assert(YVNE_DirectoryMountOpen(mount, 1) == NULL);

    int64_t position = -1;
    assert(YVNE_StreamSeek(second, 0, YVNE_STREAM_SEEK_CURRENT, &position));
    assert(position == 0);
    assert(YVNE_StreamSeek(first, 0, YVNE_STREAM_SEEK_BEGIN, &position));
    assert(position == 0);
    assert(!YVNE_StreamSeek(first, -1, YVNE_STREAM_SEEK_BEGIN, NULL));
    assert(!YVNE_StreamSeek(first, 1, YVNE_STREAM_SEEK_END, NULL));

    const int64_t size = YVNE_StreamGetSize(first);
    assert(size > 0);
    assert((uint64_t)size <= SIZE_MAX);

    void *contents = malloc((size_t)size);
    assert(contents != NULL);
    read = YVNE_StreamRead(first, contents, (size_t)size);
    assert(read.bytes_read == (size_t)size);
    assert(read.status != YVNE_STREAM_READ_ERROR);
    free(contents);

    unsigned char byte = 0;
    read = YVNE_StreamRead(first, &byte, 1);
    assert(read.bytes_read == 0);
    assert(read.status == YVNE_STREAM_READ_EOF);

    assert(YVNE_DirectoryMountOpen(mount, UINT32_MAX) == NULL);

    YVNE_StreamClose(&first);
    assert(first == NULL);
    YVNE_StreamClose(&first);
    YVNE_StreamClose(&second);
    YVNE_DirectoryMountDestroy(&mount);
    assert(mount == NULL);
    YVNE_DirectoryMountDestroy(&mount);

    YVNE_DirectoryMount *small_mount = YVNE_DirectoryMountCreate(
        YVNE_TEST_PROJECT_DIR,
        &manifest,
        1
    );
    assert(small_mount != NULL);
    assert(YVNE_DirectoryMountOpen(small_mount, 0) == NULL);
    YVNE_DirectoryMountDestroy(&small_mount);

    char unsafe_path[] = "../outside.txt";
    YVNE_AssetDescription unsafe_asset = {
        .id = 7,
        .type = YVNE_RESOURCE_SCRIPT,
        .path = {
            .size = (u16)(sizeof unsafe_path - 1),
            .str = unsafe_path,
        },
    };
    YVNE_Manifest unsafe_manifest = {
        .manifest_version = YVNE_MANIFEST_VERSION,
        .asset_count = 1,
        .project_id = manifest.project_id,
        .assets = &unsafe_asset,
    };
    YVNE_DirectoryMount *unsafe_mount = YVNE_DirectoryMountCreate(
        YVNE_TEST_PROJECT_DIR,
        &unsafe_manifest,
        2u * 1024u * 1024u
    );
    assert(unsafe_mount != NULL);
    assert(YVNE_DirectoryMountOpen(unsafe_mount, 7) == NULL);
    assert(strstr(SDL_GetError(), "escapes the project root") != NULL);
#ifdef YVNE_TEST_SYMLINKS
    unsafe_asset.path = (string){.str = "assets/escape.txt", .size = 17};
    assert(YVNE_DirectoryMountOpen(unsafe_mount, 7) == NULL);
    assert(strstr(SDL_GetError(), "escapes the project root") != NULL);
    unsafe_asset.path = (string){.str = "assets/linked.txt", .size = 17};
    first = YVNE_DirectoryMountOpen(unsafe_mount, 7);
    assert(first != NULL);
    read = YVNE_StreamRead(first, signature, sizeof signature);
    assert(read.bytes_read == sizeof signature);
    assert(memcmp(signature, contents_prefix, sizeof signature) == 0);
    YVNE_StreamClose(&first);
#endif
    YVNE_DirectoryMountDestroy(&unsafe_mount);

    assert(YVNE_DirectoryMountCreate(NULL, &manifest, 10) == NULL);
    assert(YVNE_DirectoryMountCreate(YVNE_TEST_PROJECT_DIR, NULL, 10) == NULL);
    assert(YVNE_DirectoryMountCreate(YVNE_TEST_PROJECT_DIR "/missing", &manifest, 10) == NULL);
    assert(YVNE_DirectoryMountCreate(YVNE_TEST_PROJECT_DIR "/project.toml", &manifest, 10) == NULL);
    assert(YVNE_DirectoryMountOpen(NULL, 0) == NULL);
    assert(YVNE_DirectoryMountGetAssetType(NULL, 0) == YVNE_RESOURCE_UNKNOWN);
    YVNE_DirectoryMountDestroy(NULL);
    YVNE_ManifestDestroy(&manifest);
    return 0;
}
