#include "paths.h"
#include "../core/logger.h"
#include <SDL3/SDL.h>


void YVNE_PathsGet(YVNE_Paths *paths){
  paths->resourse_root = SDL_GetBasePath();
  // paths->user_root.str = SDL_GetPrefPath("YVNE", project_id);
}
