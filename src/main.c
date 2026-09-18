#include "core/engine.h"
#include "core/logger.h"


int main(int argc, char **argv){

#ifdef YVNE_DEBUG
  const char *project_id = "dev.test";
#endif



  Engine *YVNEngine = YVNE_EngineInit();
  if(!YVNEngine){
	YVNE_LOG_ERROR("FAILED TO CREATE ENGINE INSTANCE!");
	return EXIT_FAILURE;
  }
  if(!YVNE_EngineRun(YVNEngine)){
	YVNE_LOG_ERROR("FAILEDM WHILE RUNNING ENGINE!");
  }
  YVNE_EngineDestroy(&YVNEngine);
  return EXIT_SUCCESS;
}
