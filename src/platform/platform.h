#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <SDL3/SDL.h>

bool YVNE_SDLInit();
void YVNE_SDLQuit();

void YVNE_GetPlatformInfo(void);
#endif
