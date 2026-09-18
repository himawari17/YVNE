#ifndef _ENGINE_H
#define _ENGINE_H
#include <stdlib.h>

typedef struct Engine Engine;

Engine* YVNE_EngineInit(void);
void YVNE_EngineDestroy(Engine **engine);
bool YVNE_EngineRun(Engine *engine);

#endif
