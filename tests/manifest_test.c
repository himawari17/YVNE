#include "../src/resource/manifest.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse(const char *source, YVNE_Manifest *manifest)
{
    return YVNE_ManifestParse(source, strlen(source), manifest);
}

static void reject(const char *source)
{
    YVNE_Manifest manifest = {0};
    assert(!parse(source, &manifest));
    assert(manifest.assets == NULL && manifest.asset_count == 0);
    assert(manifest.project_id.str == NULL);
    YVNE_ManifestDestroy(&manifest);
}

static int dump_manifest(const char *project_path)
{
    static const char *const type_names[] = {
        "image",
        "audio",
        "video",
        "script",
        "unknown",
    };

    YVNE_Manifest manifest = {0};
    if (!YVNE_ManifestLoadProject(project_path, &manifest)) {
        return EXIT_FAILURE;
    }

    printf("manifest_version: %u\n", manifest.manifest_version);
    printf("project_id: %s\n", manifest.project_id.str);
    printf("assets: %u\n", manifest.asset_count);

    for (u32 index = 0; index < manifest.asset_count; ++index) {
        const YVNE_AssetDescription *asset = &manifest.assets[index];
        printf(
            "  [%u] id=%u type=%s path=%s\n",
            index,
            asset->id,
            type_names[asset->type],
            asset->path.str
        );
    }

    YVNE_ManifestDestroy(&manifest);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        return dump_manifest(argv[1]);
    }

    static const char valid[] =
        "manifest_version = 1 # current format\n"
        "project_id = \"dev.тест#1\"\r\n"
        "[[assets]]\n"
        "id = 0\n"
        "type = \"image\"\n"
        "path = \"assets/Метка#1.png\"\n"
        "[[assets]]\nid = 4294967295\ntype = \"audio\"\npath = \"audio.ogg\"\n"
        "[[assets]]\nid = 2\ntype = \"video\"\npath = \"video.mp4\"\n"
        "[[assets]]\nid = 3\ntype = \"script\"\npath = \"story.txt\"";

    YVNE_Manifest manifest = {0};
    assert(parse(valid, &manifest));
    assert(manifest.manifest_version == YVNE_MANIFEST_VERSION);
    assert(manifest.asset_count == 4);
    assert(strcmp(manifest.project_id.str, "dev.тест#1") == 0);
    assert(manifest.project_id.size == strlen(manifest.project_id.str));
    const YVNE_AssetDescription *asset = YVNE_ManifestFindResource(&manifest, 0);
    assert(asset != NULL && asset->type == YVNE_RESOURCE_IMAGE);
    assert(strcmp(asset->path.str, "assets/Метка#1.png") == 0);
    assert(asset->path.size == strlen(asset->path.str));
    assert(YVNE_ManifestFindResource(&manifest, UINT32_MAX)->type == YVNE_RESOURCE_AUDIO);
    assert(YVNE_ManifestFindResource(&manifest, 2)->type == YVNE_RESOURCE_VIDEO);
    assert(YVNE_ManifestFindResource(&manifest, 3)->type == YVNE_RESOURCE_SCRIPT);
    assert(YVNE_ManifestFindResource(&manifest, 1) == NULL);
    assert(YVNE_ManifestFindResource(NULL, 0) == NULL);
    YVNE_ManifestDestroy(&manifest);
    assert(manifest.assets == NULL);
    assert(manifest.asset_count == 0 && manifest.project_id.str == NULL);
    YVNE_ManifestDestroy(&manifest);
    YVNE_ManifestDestroy(NULL);

    assert(parse("manifest_version = 1\nproject_id = \"empty\"\n", &manifest));
    assert(manifest.asset_count == 0);
    YVNE_ManifestDestroy(&manifest);

    static const char duplicate_id[] =
        "manifest_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"a.png\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"b.png\"\n";
    reject(duplicate_id);

    static const char case_collision[] =
        "manifest_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"A.png\"\n"
        "[[assets]]\nid = 1\ntype = \"image\"\npath = \"a.PNG\"\n";
    reject(case_collision);

    static const char traversal[] =
        "manifest_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"assets/../a.png\"\n";
    reject(traversal);

    static const char reserved_name[] =
        "manifest_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"assets/CON.png\"\n";
    reject(reserved_name);

    static const char unknown_field[] =
        "manifest_version = 1\nproject_id = \"dev.test\"\nextra = 1\n";
    reject(unknown_field);

    static const char wrong_version[] =
        "manifest_version = 2\nproject_id = \"dev.test\"\n";
    reject(wrong_version);

    static const char *const invalid_headers[] = {
        "", "project_id = \"test\"\n", "manifest_version = 1\n",
        "manifest_version = 1\nproject_id = \"\"\n",
        "manifest_version = 1\nmanifest_version = 1\nproject_id = \"test\"\n",
        "manifest_version = 1\nproject_id = \"a\"\nproject_id = \"b\"\n",
        "manifest_version = 4294967296\nproject_id = \"test\"\n",
        "schema_version = 1\nproject_id = \"test\"\n",
    };
    for (size_t i = 0; i < sizeof invalid_headers / sizeof invalid_headers[0]; ++i) {
        reject(invalid_headers[i]);
    }
    static const char *const invalid_assets[] = {
        "[[assets]]\nid = 1\ntype = \"image\"\n",
        "[[assets]]\nid = 1\n[[assets]]\n",
        "[[assets]]\nid = -1\ntype = \"image\"\npath = \"a.png\"\n",
        "[[assets]]\nid = 4294967296\ntype = \"image\"\npath = \"a.png\"\n",
        "[[assets]]\nid = 1\nid = 2\ntype = \"image\"\npath = \"a.png\"\n",
        "[[assets]]\nid = 1\ntype = \"font\"\npath = \"a.png\"\n",
        "[[assets]]\nid = 1\ntype = \"image\"\npath = \"a.png\"\nextra = 1\n",
    };
    char source[2048];
    for (size_t i = 0; i < sizeof invalid_assets / sizeof invalid_assets[0]; ++i) {
        const int length = snprintf(source, sizeof source,
            "manifest_version = 1\nproject_id = \"test\"\n%s", invalid_assets[i]);
        assert(length > 0 && (size_t)length < sizeof source);
        reject(source);
    }
    static const char *const invalid_paths[] = {
        "", "/absolute.png", "./a.png", "a//b.png", "a/", "a/../b.png",
        "C:/a.png", "a\\\\b.png", "assets/NUL.txt", "COM1.png", "LPT9.txt",
        "a?.png", "a*.png", "a|b.png", "a:b.png", "a<.png", "a>.png",
        "assets./a.png", "assets /a.png", "a\\n.png",
    };
    for (size_t i = 0; i < sizeof invalid_paths / sizeof invalid_paths[0]; ++i) {
        const int length = snprintf(source, sizeof source,
            "manifest_version = 1\nproject_id = \"test\"\n"
            "[[assets]]\nid = 0\ntype = \"image\"\npath = \"%s\"\n", invalid_paths[i]);
        assert(length > 0 && (size_t)length < sizeof source);
        reject(source);
    }

    char long_string[YVNE_ASSET_PATH_MAX_BYTES + 2];
    memset(long_string, 'a', sizeof long_string - 1);
    long_string[sizeof long_string - 1] = '\0';
    int length = snprintf(source, sizeof source,
        "manifest_version = 1\nproject_id = \"test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"%s\"\n", long_string);
    assert(length > 0 && (size_t)length < sizeof source);
    reject(source);
    long_string[YVNE_ASSET_PATH_MAX_BYTES] = '\0';
    length = snprintf(source, sizeof source,
        "manifest_version = 1\nproject_id = \"test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"%s\"\n", long_string);
    assert(length > 0 && (size_t)length < sizeof source);
    assert(parse(source, &manifest));
    assert(manifest.assets[0].path.size == YVNE_ASSET_PATH_MAX_BYTES);
    YVNE_ManifestDestroy(&manifest);
    long_string[YVNE_PROJECT_ID_MAX_BYTES + 1] = '\0';
    length = snprintf(source, sizeof source,
        "manifest_version = 1\nproject_id = \"%s\"\n", long_string);
    assert(length > 0 && (size_t)length < sizeof source);
    reject(source);
    long_string[YVNE_PROJECT_ID_MAX_BYTES] = '\0';
    length = snprintf(source, sizeof source,
        "manifest_version = 1\nproject_id = \"%s\"\n", long_string);
    assert(length > 0 && (size_t)length < sizeof source);
    assert(parse(source, &manifest));
    assert(manifest.project_id.size == YVNE_PROJECT_ID_MAX_BYTES);
    YVNE_ManifestDestroy(&manifest);

    static const char malformed_utf8[] =
        "manifest_version = 1\nproject_id = \"dev.\xc0\xaf\"\n";
    assert(!YVNE_ManifestParse(
        malformed_utf8,
        sizeof malformed_utf8 - 1,
        &manifest
    ));
    static const char embedded_nul[] = "manifest_version = 1\nproject_id = \"test\"\n\0ignored";
    assert(!YVNE_ManifestParse(embedded_nul, sizeof embedded_nul - 1, &manifest));
    assert(!YVNE_ManifestParse(NULL, 0, &manifest));
    assert(!YVNE_ManifestParse(valid, sizeof valid - 1, NULL));

    assert(!YVNE_ManifestParse(
        "",
        YVNE_MANIFEST_MAX_BYTES + 1u,
        &manifest
    ));

    assert(YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR, &manifest));
    assert(manifest.asset_count == 2);
    assert(strcmp(manifest.project_id.str, "dev.test") == 0);
    assert(YVNE_ManifestFindResource(&manifest, 0) != NULL);
    YVNE_ManifestDestroy(&manifest);
    assert(!YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR "/missing", &manifest));
    assert(!YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR "/assets", &manifest));
    assert(!YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR "/project.toml", &manifest));
    assert(!YVNE_ManifestLoadProject(NULL, &manifest));
    assert(!YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR, NULL));
    return 0;
}
