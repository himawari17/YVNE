#include "../src/core/logger.h"

#include <assert.h>

int main(void)
{
    int debug_arguments_evaluated = 0;
    int info_arguments_evaluated = 0;

    YVNE_LOG_DEBUG("debug value: %d", ++debug_arguments_evaluated);
    YVNE_LOG_INFO("info value: %d", ++info_arguments_evaluated);

#ifdef YVNE_DEBUG
    assert(debug_arguments_evaluated == 1);
    assert(info_arguments_evaluated == 1);
#else
    assert(debug_arguments_evaluated == 0);
    assert(info_arguments_evaluated == 0);
#endif

    YVNE_LogWrite(YVNE_LOG_LEVEL_WARN, "path/to/logger_test.c", 42,
                  "logger value: %d", 7);
    YVNE_LogWrite(YVNE_LOG_LEVEL_ERROR, "C:\\src\\logger_test.c", 43, "error test");
    YVNE_LogWrite((YVNE_LogLevel)99, "logger_test.c", 44, "unknown level test");
    return 0;
}
