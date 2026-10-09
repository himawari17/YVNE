#include "yvne/version.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    assert(strcmp(yvne_version(), YVNE_TEST_VERSION) == 0);
    return 0;
}
