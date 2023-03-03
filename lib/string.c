#include "string.h"

void *memset(void *dst, int c, uint64 n) {
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; ++i) cdst[i] = c;

    return dst;
}

void* memcpy(void* dst, void* src, uint64 sz) {
    char* cdst = (char*) dst;
    char* csrc = (char*) src;
    for(int i=0; i< sz; i++) {
        cdst[i] = csrc[i];
    }
    return dst;
}
