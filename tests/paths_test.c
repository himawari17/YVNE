#include "../src/platform/paths.h"

#include <SDL3/SDL.h>
#include <assert.h>

int main(void)
{
    YVNE_Paths paths = {0};
    YVNE_PathsGet(&paths);
    assert(paths.resourse_root != NULL && paths.resourse_root[0] != '\0');
    assert(SDL_strcmp(paths.resourse_root, SDL_GetBasePath()) == 0);
    SDL_PathInfo info;
    assert(SDL_GetPathInfo(paths.resourse_root, &info));
    assert(info.type == SDL_PATHTYPE_DIRECTORY);
    YVNE_PathsGet(&paths);
    assert(SDL_strcmp(paths.resourse_root, SDL_GetBasePath()) == 0);
    return 0;
}
