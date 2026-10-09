#include "manifest.h"

#include "../core/logger.h"

#include <SDL3/SDL.h>

static char *YVNE_Trim(char *text);
static void YVNE_StripComment(char *text);
static bool YVNE_IsValidUTF8(const char *data, size_t size);
static bool YVNE_ParseU32(const char *value, u32 *result);
static bool YVNE_ParseString(const char *value, size_t limit, string *result);
static YVNEAssetType YVNE_ParseAssetType(const char *value);
static bool YVNE_IsValidProjectId(const string *project_id);
static bool YVNE_IsValidRelativePath(const string *path);
static bool YVNE_HasPathCollision(const YVNE_Manifest *manifest);

bool YVNE_ManifestLoadProject(const char *path, YVNE_Manifest *manifest)
{
    if (path == NULL || manifest == NULL) {
        return false;
    }

    *manifest = (YVNE_Manifest){0};

    SDL_PathInfo info;
    if (!SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_DIRECTORY) {
        YVNE_LOG_ERROR(
            "UNABLE TO FIND PROJECT BY PATH %s. SDL ERROR: %s",
            path,
            SDL_GetError()
        );
        return false;
    }

    char *manifest_path = NULL;
    if (SDL_asprintf(&manifest_path, "%s/project.toml", path) < 0) {
        YVNE_LOG_ERROR("FAILED TO CREATE MANIFEST PATH: %s", SDL_GetError());
        return false;
    }

    SDL_IOStream *manifest_file = SDL_IOFromFile(manifest_path, "rb");
    if (manifest_file == NULL) {
        YVNE_LOG_ERROR(
            "UNABLE TO READ MANIFEST %s. SDL ERROR: %s",
            manifest_path,
            SDL_GetError()
        );
        SDL_free(manifest_path);
        return false;
    }
    SDL_free(manifest_path);

    const Sint64 file_size = SDL_GetIOSize(manifest_file);
    if (file_size < 0 ||
        (uint64_t)file_size > YVNE_MANIFEST_MAX_BYTES ||
        (uint64_t)file_size > SIZE_MAX - 1) {
        YVNE_LOG_ERROR("INVALID MANIFEST SIZE: %lld", (long long)file_size);
        SDL_CloseIO(manifest_file);
        return false;
    }

    const size_t size = (size_t)file_size;
    char *source = SDL_malloc(size + 1);
    if (source == NULL) {
        YVNE_LOG_ERROR("FAILED TO ALLOCATE MANIFEST BUFFER");
        SDL_CloseIO(manifest_file);
        return false;
    }

    size_t total = 0;
    while (total < size) {
        const size_t read = SDL_ReadIO(manifest_file, source + total, size - total);
        if (read == 0) {
            YVNE_LOG_ERROR("FAILED TO READ MANIFEST: %s", SDL_GetError());
            SDL_CloseIO(manifest_file);
            SDL_free(source);
            return false;
        }
        total += read;
    }

    SDL_CloseIO(manifest_file);
    source[size] = '\0';

    const bool parsed = YVNE_ManifestParse(source, size, manifest);
    SDL_free(source);
    return parsed;
}

bool YVNE_ManifestParse(const char *data, size_t size, YVNE_Manifest *manifest)
{
    if (data == NULL || manifest == NULL ||
        size > YVNE_MANIFEST_MAX_BYTES ||
        !YVNE_IsValidUTF8(data, size)) {
        return false;
    }

    *manifest = (YVNE_Manifest){0};

    char *text = SDL_strndup(data, size);
    if (text == NULL) {
        return false;
    }

    YVNE_AssetDescription *asset = NULL;
    bool has_manifest_version = false;
    bool has_project_id = false;
    bool has_asset_id = false;
    bool has_asset_type = false;
    bool has_asset_path = false;
    u32 line_number = 0;
    const char *error = NULL;

    for (char *line = text; line != NULL;) {
        ++line_number;

        char *next_line = SDL_strchr(line, '\n');
        if (next_line != NULL) {
            *next_line++ = '\0';
        }

        YVNE_StripComment(line);
        line = YVNE_Trim(line);

        if (*line == '\0') {
            line = next_line;
            continue;
        }

        if (SDL_strcmp(line, "[[assets]]") == 0) {
            if (asset != NULL &&
                (!has_asset_id || !has_asset_type || !has_asset_path)) {
                error = "previous asset is incomplete";
                goto parse_failed;
            }

            if (manifest->asset_count >= YVNE_MANIFEST_MAX_ASSETS) {
                error = "too many assets";
                goto parse_failed;
            }

            const size_t new_count = (size_t)manifest->asset_count + 1;
            if (new_count > SIZE_MAX / sizeof *manifest->assets) {
                error = "asset array size overflow";
                goto parse_failed;
            }

            YVNE_AssetDescription *new_assets = SDL_realloc(
                manifest->assets,
                new_count * sizeof *new_assets
            );
            if (new_assets == NULL) {
                error = "failed to allocate assets";
                goto parse_failed;
            }

            manifest->assets = new_assets;
            asset = &manifest->assets[manifest->asset_count++];
            *asset = (YVNE_AssetDescription){
                .type = YVNE_RESOURCE_UNKNOWN,
            };

            has_asset_id = false;
            has_asset_type = false;
            has_asset_path = false;
            line = next_line;
            continue;
        }

        char *equals = SDL_strchr(line, '=');
        if (equals == NULL) {
            error = "expected key = value";
            goto parse_failed;
        }

        *equals = '\0';
        char *key = YVNE_Trim(line);
        char *value = YVNE_Trim(equals + 1);

        if (asset == NULL && SDL_strcmp(key, "manifest_version") == 0) {
            if (has_manifest_version ||
                !YVNE_ParseU32(value, &manifest->manifest_version) ||
                manifest->manifest_version != YVNE_MANIFEST_VERSION) {
                error = "invalid or unsupported manifest_version";
                goto parse_failed;
            }
            has_manifest_version = true;
        } else if (asset == NULL && SDL_strcmp(key, "project_id") == 0) {
            if (has_project_id ||
                !YVNE_ParseString(
                    value,
                    YVNE_PROJECT_ID_MAX_BYTES,
                    &manifest->project_id
                ) ||
                !YVNE_IsValidProjectId(&manifest->project_id)) {
                error = "invalid or duplicate project_id";
                goto parse_failed;
            }
            has_project_id = true;
        } else if (asset == NULL) {
            error = "unknown manifest field";
            goto parse_failed;
        } else if (SDL_strcmp(key, "id") == 0) {
            if (has_asset_id || !YVNE_ParseU32(value, &asset->id)) {
                error = "invalid or duplicate asset id field";
                goto parse_failed;
            }

            for (u32 index = 0; index + 1 < manifest->asset_count; ++index) {
                if (manifest->assets[index].id == asset->id) {
                    error = "duplicate asset id";
                    goto parse_failed;
                }
            }
            has_asset_id = true;
        } else if (SDL_strcmp(key, "type") == 0) {
            if (has_asset_type ||
                (asset->type = YVNE_ParseAssetType(value)) == YVNE_RESOURCE_UNKNOWN) {
                error = "invalid or duplicate asset type";
                goto parse_failed;
            }
            has_asset_type = true;
        } else if (SDL_strcmp(key, "path") == 0) {
            if (has_asset_path ||
                !YVNE_ParseString(
                    value,
                    YVNE_ASSET_PATH_MAX_BYTES,
                    &asset->path
                ) ||
                !YVNE_IsValidRelativePath(&asset->path)) {
                error = "invalid or duplicate asset path";
                goto parse_failed;
            }
            has_asset_path = true;
        } else {
            error = "unknown asset field";
            goto parse_failed;
        }

        line = next_line;
    }

    if (!has_manifest_version) {
        error = "manifest_version is missing";
        goto parse_failed;
    }
    if (!has_project_id) {
        error = "project_id is missing";
        goto parse_failed;
    }
    if (asset != NULL && (!has_asset_id || !has_asset_type || !has_asset_path)) {
        error = "last asset is incomplete";
        goto parse_failed;
    }
    if (YVNE_HasPathCollision(manifest)) {
        error = "asset paths collide ignoring case";
        goto parse_failed;
    }

    SDL_free(text);
    return true;

parse_failed:
    YVNE_LOG_ERROR("INVALID MANIFEST AT LINE %u: %s", line_number, error);
    SDL_free(text);
    YVNE_ManifestDestroy(manifest);
    return false;
}

const YVNE_AssetDescription *YVNE_ManifestFindResource(
    const YVNE_Manifest *manifest,
    YVNE_AssetId asset_id
) {
    if (manifest == NULL) {
        return NULL;
    }

    for (u32 index = 0; index < manifest->asset_count; ++index) {
        if (manifest->assets[index].id == asset_id) {
            return &manifest->assets[index];
        }
    }
    return NULL;
}

void YVNE_ManifestDestroy(YVNE_Manifest *manifest)
{
    if (manifest == NULL) {
        return;
    }

    SDL_free(manifest->project_id.str);
    for (u32 index = 0; index < manifest->asset_count; ++index) {
        SDL_free(manifest->assets[index].path.str);
    }
    SDL_free(manifest->assets);
    *manifest = (YVNE_Manifest){0};
}

static char *YVNE_Trim(char *text)
{
    while (SDL_isspace((unsigned char)*text)) {
        ++text;
    }

    char *end = text + SDL_strlen(text);
    while (end > text && SDL_isspace((unsigned char)end[-1])) {
        --end;
    }
    *end = '\0';
    return text;
}

static void YVNE_StripComment(char *text)
{
    bool in_string = false;
    bool escaped = false;

    for (; *text != '\0'; ++text) {
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (*text == '\\') {
                escaped = true;
            } else if (*text == '"') {
                in_string = false;
            }
        } else if (*text == '"') {
            in_string = true;
        } else if (*text == '#') {
            *text = '\0';
            return;
        }
    }
}

static bool YVNE_IsValidUTF8(const char *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;

    for (size_t index = 0; index < size;) {
        const unsigned char first = bytes[index];
        size_t length;

        if (first == 0) {
            return false;
        } else if (first <= 0x7f) {
            length = 1;
        } else if (first >= 0xc2 && first <= 0xdf) {
            length = 2;
        } else if (first >= 0xe0 && first <= 0xef) {
            length = 3;
        } else if (first >= 0xf0 && first <= 0xf4) {
            length = 4;
        } else {
            return false;
        }

        if (length > size - index) {
            return false;
        }
        for (size_t offset = 1; offset < length; ++offset) {
            if ((bytes[index + offset] & 0xc0) != 0x80) {
                return false;
            }
        }

        if ((first == 0xe0 && bytes[index + 1] < 0xa0) ||
            (first == 0xed && bytes[index + 1] > 0x9f) ||
            (first == 0xf0 && bytes[index + 1] < 0x90) ||
            (first == 0xf4 && bytes[index + 1] > 0x8f)) {
            return false;
        }

        index += length;
    }
    return true;
}

static bool YVNE_ParseU32(const char *value, u32 *result)
{
    if (value == NULL || result == NULL || *value == '\0') {
        return false;
    }

    u32 number = 0;
    for (const unsigned char *cursor = (const unsigned char *)value;
         *cursor != '\0';
         ++cursor) {
        if (*cursor < '0' || *cursor > '9') {
            return false;
        }

        const u32 digit = (u32)(*cursor - '0');
        if (number > (UINT32_MAX - digit) / 10u) {
            return false;
        }
        number = number * 10u + digit;
    }

    *result = number;
    return true;
}

static bool YVNE_ParseString(const char *value, size_t limit, string *result)
{
    if (value == NULL || result == NULL) {
        return false;
    }

    const size_t length = SDL_strlen(value);
    if (length < 2 || value[0] != '"' || value[length - 1] != '"') {
        return false;
    }

    char *decoded = SDL_malloc(length);
    if (decoded == NULL) {
        return false;
    }

    size_t written = 0;
    for (size_t index = 1; index + 1 < length; ++index) {
        unsigned char character = (unsigned char)value[index];

        if (character == '"' || character < 0x20) {
            SDL_free(decoded);
            return false;
        }

        if (character == '\\') {
            if (++index + 1 >= length) {
                SDL_free(decoded);
                return false;
            }

            switch (value[index]) {
            case '"': character = '"'; break;
            case '\\': character = '\\'; break;
            case 'b': character = '\b'; break;
            case 't': character = '\t'; break;
            case 'n': character = '\n'; break;
            case 'f': character = '\f'; break;
            case 'r': character = '\r'; break;
            default:
                SDL_free(decoded);
                return false;
            }
        }

        if (written >= limit || written >= UINT16_MAX) {
            SDL_free(decoded);
            return false;
        }
        decoded[written++] = (char)character;
    }

    decoded[written] = '\0';
    result->str = decoded;
    result->size = (u16)written;
    return true;
}

static YVNEAssetType YVNE_ParseAssetType(const char *value)
{
    string type = {0};
    if (!YVNE_ParseString(value, 16, &type)) {
        return YVNE_RESOURCE_UNKNOWN;
    }

    YVNEAssetType result = YVNE_RESOURCE_UNKNOWN;
    if (SDL_strcmp(type.str, "image") == 0) {
        result = YVNE_RESOURCE_IMAGE;
    } else if (SDL_strcmp(type.str, "audio") == 0) {
        result = YVNE_RESOURCE_AUDIO;
    } else if (SDL_strcmp(type.str, "video") == 0) {
        result = YVNE_RESOURCE_VIDEO;
    } else if (SDL_strcmp(type.str, "script") == 0) {
        result = YVNE_RESOURCE_SCRIPT;
    }

    SDL_free(type.str);
    return result;
}

static bool YVNE_IsValidProjectId(const string *project_id)
{
    if (project_id == NULL || project_id->str == NULL || project_id->size == 0) {
        return false;
    }

    for (u16 index = 0; index < project_id->size; ++index) {
        const unsigned char character = (unsigned char)project_id->str[index];
        if (character < 0x20 || character == 0x7f) {
            return false;
        }
    }
    return true;
}

static bool YVNE_EqualsASCII(const char *text, size_t length, const char *expected)
{
    const size_t expected_length = SDL_strlen(expected);
    if (length != expected_length) {
        return false;
    }

    for (size_t index = 0; index < length; ++index) {
        if (SDL_tolower((unsigned char)text[index]) != expected[index]) {
            return false;
        }
    }
    return true;
}

static bool YVNE_IsReservedWindowsName(const char *segment, size_t length)
{
    size_t stem_length = 0;
    while (stem_length < length && segment[stem_length] != '.') {
        ++stem_length;
    }

    if (YVNE_EqualsASCII(segment, stem_length, "con") ||
        YVNE_EqualsASCII(segment, stem_length, "prn") ||
        YVNE_EqualsASCII(segment, stem_length, "aux") ||
        YVNE_EqualsASCII(segment, stem_length, "nul")) {
        return true;
    }

    return stem_length == 4 &&
           segment[3] >= '1' && segment[3] <= '9' &&
           (YVNE_EqualsASCII(segment, 3, "com") ||
            YVNE_EqualsASCII(segment, 3, "lpt"));
}

static bool YVNE_IsValidRelativePath(const string *path)
{
    if (path == NULL || path->str == NULL || path->size == 0 ||
        path->str[0] == '/' || path->str[0] == '\\') {
        return false;
    }

    const char *segment = path->str;
    for (const char *cursor = path->str;; ++cursor) {
        const unsigned char character = (unsigned char)*cursor;
        if (*cursor == '/' || *cursor == '\0') {
            const size_t length = (size_t)(cursor - segment);
            if (length == 0 ||
                (length == 1 && segment[0] == '.') ||
                (length == 2 && segment[0] == '.' && segment[1] == '.') ||
                segment[length - 1] == '.' || segment[length - 1] == ' ' ||
                YVNE_IsReservedWindowsName(segment, length)) {
                return false;
            }

            if (*cursor == '\0') {
                break;
            }
            segment = cursor + 1;
            continue;
        }

        if (character < 0x20 || character == 0x7f ||
            character == '\\' || character == ':' ||
            character == '<' || character == '>' ||
            character == '"' || character == '|' ||
            character == '?' || character == '*') {
            return false;
        }
    }
    return true;
}

static bool YVNE_HasPathCollision(const YVNE_Manifest *manifest)
{
    for (u32 left = 0; left < manifest->asset_count; ++left) {
        for (u32 right = left + 1; right < manifest->asset_count; ++right) {
            if (SDL_strcasecmp(
                    manifest->assets[left].path.str,
                    manifest->assets[right].path.str
                ) == 0) {
                return true;
            }
        }
    }
    return false;
}
