#include "../src/resource/manifest.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse(const char *source, YVNE_Manifest *manifest)
{
    return YVNE_ManifestParse(source, strlen(source), manifest);
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
        "project_id = \"dev.тест#1\"\n"
        "[[assets]]\n"
        "id = 0\n"
        "type = \"image\"\n"
        "path = \"assets/Метка.png\"\n";

    YVNE_Manifest manifest = {0};
    assert(parse(valid, &manifest));
    assert(manifest.manifest_version == YVNE_MANIFEST_VERSION);
    assert(manifest.asset_count == 1);
    assert(YVNE_ManifestFindResource(&manifest, 0) != NULL);
    assert(YVNE_ManifestFindResource(&manifest, 1) == NULL);
    YVNE_ManifestDestroy(&manifest);
    assert(manifest.assets == NULL);

    static const char duplicate_id[] =
        "schema_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"a.png\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"b.png\"\n";
    assert(!parse(duplicate_id, &manifest));

    static const char case_collision[] =
        "schema_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"A.png\"\n"
        "[[assets]]\nid = 1\ntype = \"image\"\npath = \"a.PNG\"\n";
    assert(!parse(case_collision, &manifest));

    static const char traversal[] =
        "schema_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"assets/../a.png\"\n";
    assert(!parse(traversal, &manifest));

    static const char reserved_name[] =
        "schema_version = 1\nproject_id = \"dev.test\"\n"
        "[[assets]]\nid = 0\ntype = \"image\"\npath = \"assets/CON.png\"\n";
    assert(!parse(reserved_name, &manifest));

    static const char unknown_field[] =
        "schema_version = 1\nproject_id = \"dev.test\"\nextra = 1\n";
    assert(!parse(unknown_field, &manifest));

    static const char wrong_version[] =
        "schema_version = 2\nproject_id = \"dev.test\"\n";
    assert(!parse(wrong_version, &manifest));

    static const char malformed_utf8[] = {
        's', 'c', 'h', 'e', 'm', 'a', '_', 'v', 'e', 'r', 's', 'i', 'o', 'n',
        ' ', '=', ' ', '1', '\n', (char)0xc0, (char)0xaf
    };
    assert(!YVNE_ManifestParse(
        malformed_utf8,
        sizeof malformed_utf8,
        &manifest
    ));

    assert(!YVNE_ManifestParse(
        "",
        YVNE_MANIFEST_MAX_BYTES + 1u,
        &manifest
    ));

    assert(YVNE_ManifestLoadProject(YVNE_TEST_PROJECT_DIR, &manifest));
    assert(YVNE_ManifestFindResource(&manifest, 0) != NULL);
    YVNE_ManifestDestroy(&manifest);
    return 0;
}
