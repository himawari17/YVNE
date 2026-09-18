#include "../src/core/logger.h"

#include <assert.h>

int main(void)
{
    int debug_arguments_evaluated = 0;

    YVNE_LOG_DEBUG("debug value: %d", ++debug_arguments_evaluated);

#ifdef YVNE_DEBUG
    assert(debug_arguments_evaluated == 1);
#else
    assert(debug_arguments_evaluated == 0);
#endif

    YVNE_LOG_WARN("logger test");
    return 0;
}
