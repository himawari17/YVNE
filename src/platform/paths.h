#ifndef _PATHS_H
#define _PATHS_H
#include "../core/types.h"

typedef struct{
  const char *resourse_root;
  string user_root;
  string saves;
  string settings;
  string system_logs;
} YVNE_Paths;

void YVNE_PathsGet(YVNE_Paths *paths);


#endif
