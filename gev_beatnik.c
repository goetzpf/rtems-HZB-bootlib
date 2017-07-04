#include <stdlib.h>
#include <string.h>

#define GEV_START        (0xf1100000 + 0x10000 + 0x70F8)

int write_gev(const char *src, size_t length)
{
    memcpy((void*)GEV_START, src, length);
    return 0;
}

int read_gev(char *dest, size_t length)
{
    memcpy(dest, (void*)GEV_START, length);
    return 0;
}
