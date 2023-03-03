#ifndef RV64_OS_HALT_H
#define RV64_OS_HALT_H

#include "printk.h"

static inline void halt() {
    printk("HALT!\n");
    while (1)
        ;
}



extern void halt();

#endif