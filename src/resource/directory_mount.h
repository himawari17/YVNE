#ifndef YVNE_DIRECTORY_MOUNT_H
#define YVNE_DIRECTORY_MOUNT_H

#include "manifest.h"
#include "stream.h"

typedef struct YVNE_DirectoryMount YVNE_DirectoryMount;

YVNE_DirectoryMount *YVNE_DirectoryMountCreate(
    const char *project_root,
    const YVNE_Manifest *manifest,
    u64 maximum_asset_size
);
YVNE_Stream *YVNE_DirectoryMountOpen(
    const YVNE_DirectoryMount *mount,
    YVNE_AssetId asset_id
);
void YVNE_DirectoryMountDestroy(YVNE_DirectoryMount **mount);

#endif
