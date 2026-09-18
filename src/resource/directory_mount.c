#if !defined(_WIN32)
#define _XOPEN_SOURCE 700
#endif

#include "directory_mount.h"

#include "../core/logger.h"

#include <SDL3/SDL.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

struct YVNE_DirectoryMount {
    char *root;
    const YVNE_Manifest *manifest;
    u64 maximum_asset_size;
};

#if defined(_WIN32)
static wchar_t *YVNE_UTF8ToWide(const char *text)
{
    const int length = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text,
        -1,
        NULL,
        0
    );
    if (length <= 0) {
        return NULL;
    }

    wchar_t *wide = SDL_malloc((size_t)length * sizeof *wide);
    if (wide == NULL || MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            text,
            -1,
            wide,
            length
        ) != length) {
        SDL_free(wide);
        return NULL;
    }
    return wide;
}

static char *YVNE_WideToUTF8(const wchar_t *text)
{
    const int length = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        text,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );
    if (length <= 0) {
        return NULL;
    }

    char *utf8 = SDL_malloc((size_t)length);
    if (utf8 == NULL || WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            text,
            -1,
            utf8,
            length,
            NULL,
            NULL
        ) != length) {
        SDL_free(utf8);
        return NULL;
    }
    return utf8;
}

static char *YVNE_CanonicalPath(const char *path)
{
    wchar_t *wide_path = YVNE_UTF8ToWide(path);
    if (wide_path == NULL) {
        SDL_SetError("failed to convert path to UTF-16");
        return NULL;
    }

    HANDLE handle = CreateFileW(
        wide_path,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );
    SDL_free(wide_path);

    if (handle == INVALID_HANDLE_VALUE) {
        SDL_SetError("failed to open path: Windows error %lu", GetLastError());
        return NULL;
    }

    const DWORD flags = FILE_NAME_NORMALIZED | VOLUME_NAME_DOS;
    const DWORD needed = GetFinalPathNameByHandleW(handle, NULL, 0, flags);
    if (needed == 0 || needed == MAXDWORD) {
        SDL_SetError("failed to resolve path: Windows error %lu", GetLastError());
        CloseHandle(handle);
        return NULL;
    }

    const DWORD capacity = needed + 1;
    wchar_t *resolved = SDL_malloc((size_t)capacity * sizeof *resolved);
    if (resolved == NULL) {
        SDL_SetError("failed to allocate resolved path");
        CloseHandle(handle);
        return NULL;
    }

    const DWORD written = GetFinalPathNameByHandleW(
        handle,
        resolved,
        capacity,
        flags
    );
    const DWORD resolve_error = written == 0
        ? GetLastError()
        : ERROR_INSUFFICIENT_BUFFER;
    CloseHandle(handle);

    if (written == 0 || written >= capacity) {
        SDL_SetError("failed to resolve path: Windows error %lu", resolve_error);
        SDL_free(resolved);
        return NULL;
    }

    char *utf8 = YVNE_WideToUTF8(resolved);
    SDL_free(resolved);
    if (utf8 == NULL) {
        SDL_SetError("failed to convert resolved path to UTF-8");
    }
    return utf8;
}
#else
static char *YVNE_CanonicalPath(const char *path)
{
    errno = 0;
    char *native_path = realpath(path, NULL);
    if (native_path == NULL) {
        SDL_SetError("failed to resolve %s: %s", path, strerror(errno));
        return NULL;
    }

    char *canonical_path = SDL_strdup(native_path);
    free(native_path);
    return canonical_path;
}
#endif

static bool YVNE_IsPathSeparator(char character)
{
#if defined(_WIN32)
    return character == '/' || character == '\\';
#else
    return character == '/';
#endif
}

static bool YVNE_IsInsideRoot(const char *root, const char *path)
{
    const size_t root_length = SDL_strlen(root);
    const size_t path_length = SDL_strlen(path);
    if (root_length == 0 || path_length <= root_length) {
        return false;
    }

#if defined(_WIN32)
    if (SDL_strncasecmp(root, path, root_length) != 0) {
        return false;
    }
#else
    if (SDL_strncmp(root, path, root_length) != 0) {
        return false;
    }
#endif

    return YVNE_IsPathSeparator(root[root_length - 1]) ||
           YVNE_IsPathSeparator(path[root_length]);
}

YVNE_DirectoryMount *YVNE_DirectoryMountCreate(
    const char *project_root,
    const YVNE_Manifest *manifest,
    u64 maximum_asset_size
) {
    if (project_root == NULL || manifest == NULL) {
        SDL_SetError("invalid directory mount");
        return NULL;
    }

    SDL_PathInfo info;
    if (!SDL_GetPathInfo(project_root, &info) ||
        info.type != SDL_PATHTYPE_DIRECTORY) {
        SDL_SetError("directory mount root is not a directory");
        return NULL;
    }

    char *root = YVNE_CanonicalPath(project_root);
    if (root == NULL) {
        return NULL;
    }

    YVNE_DirectoryMount *mount = SDL_malloc(sizeof *mount);
    if (mount == NULL) {
        SDL_SetError("failed to allocate directory mount");
        SDL_free(root);
        return NULL;
    }

    mount->root = root;
    mount->manifest = manifest;
    mount->maximum_asset_size = maximum_asset_size;
    return mount;
}

YVNE_Stream *YVNE_DirectoryMountOpen(
    const YVNE_DirectoryMount *mount,
    YVNE_AssetId asset_id
) {
    if (mount == NULL || mount->manifest == NULL) {
        SDL_SetError("invalid directory mount");
        return NULL;
    }

    const YVNE_AssetDescription *asset = YVNE_ManifestFindResource(
        mount->manifest,
        asset_id
    );
    if (asset == NULL) {
        SDL_SetError("asset %u is not in the manifest", asset_id);
        return NULL;
    }
    if (asset->path.str == NULL) {
        SDL_SetError("asset %u has no path", asset_id);
        return NULL;
    }

    char *joined_path = NULL;
    if (SDL_asprintf(
            &joined_path,
            "%s/%s",
            mount->root,
            asset->path.str
        ) < 0) {
        return NULL;
    }

#if defined(_WIN32)
    for (char *cursor = joined_path; *cursor != '\0'; ++cursor) {
        if (*cursor == '/') {
            *cursor = '\\';
        }
    }
#endif

    char *canonical_path = YVNE_CanonicalPath(joined_path);
    SDL_free(joined_path);
    if (canonical_path == NULL) {
        YVNE_LOG_ERROR("FAILED TO RESOLVE ASSET %u: %s", asset_id, SDL_GetError());
        return NULL;
    }

    if (!YVNE_IsInsideRoot(mount->root, canonical_path)) {
        SDL_SetError("asset %u escapes the project root", asset_id);
        SDL_free(canonical_path);
        return NULL;
    }

    YVNE_Stream *stream = YVNE_StreamOpenFile(
        canonical_path,
        mount->maximum_asset_size
    );
    if (stream == NULL) {
        YVNE_LOG_ERROR("FAILED TO OPEN ASSET %u: %s", asset_id, SDL_GetError());
    }

    SDL_free(canonical_path);
    return stream;
}

void YVNE_DirectoryMountDestroy(YVNE_DirectoryMount **mount)
{
    if (mount == NULL || *mount == NULL) {
        return;
    }

    SDL_free((*mount)->root);
    SDL_free(*mount);
    *mount = NULL;
}
