#pragma once

#include "types.h"
#include "defs.h"

void* memset(void *, int, uint64);

void* memcpy(void*, void*, uint64);

static inline void* pgcpy(void* dst ,void* src) {
    memcpy(dst, src, PGSIZE);
}

extern  void* pgcpy(void*, void*);
