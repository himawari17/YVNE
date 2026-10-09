#ifndef YVNE_MANIFEST_H
#define YVNE_MANIFEST_H

#include "../core/types.h"

#define YVNE_MANIFEST_VERSION 1u
#define YVNE_MANIFEST_MAX_BYTES (1024u * 1024u)
#define YVNE_MANIFEST_MAX_ASSETS 4096u
#define YVNE_PROJECT_ID_MAX_BYTES 128u
#define YVNE_ASSET_PATH_MAX_BYTES 1024u

typedef u32 YVNE_AssetId;

typedef enum{
    YVNE_RESOURCE_IMAGE,
    YVNE_RESOURCE_AUDIO,
    YVNE_RESOURCE_VIDEO,
    YVNE_RESOURCE_SCRIPT,
    YVNE_RESOURCE_UNKNOWN,
} YVNEAssetType;

typedef struct{
    YVNE_AssetId id;
    YVNEAssetType type;
    string path;
} YVNE_AssetDescription;

typedef struct{
    u32 manifest_version;
    u32 asset_count;
    string project_id;
    YVNE_AssetDescription *assets;
} YVNE_Manifest;

bool YVNE_ManifestParse(const char *data, size_t size, YVNE_Manifest *manifest);
bool YVNE_ManifestLoadProject(const char *path, YVNE_Manifest *manifest);
const YVNE_AssetDescription *YVNE_ManifestFindResource(
    const YVNE_Manifest *manifest,
    YVNE_AssetId asset_id
);
void YVNE_ManifestDestroy(YVNE_Manifest *manifest);

#endif
