#include "core/engine.h"
#include "core/logger.h"

int main(void) {

  Engine *YVNEngine = YVNE_EngineInit();
  if (!YVNEngine) {
    YVNE_LOG_ERROR("FAILED TO CREATE ENGINE INSTANCE!");
    return EXIT_FAILURE;
  }
  const bool success = YVNE_EngineRun(YVNEngine);
  if (!success) {
    YVNE_LOG_ERROR("FAILED WHILE RUNNING ENGINE!");
  }
  YVNE_EngineDestroy(&YVNEngine);
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
