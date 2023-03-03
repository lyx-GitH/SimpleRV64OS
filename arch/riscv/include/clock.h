#ifndef RISCV_OS_CLOCK_H
#define RISCV_OS_CLOCK_H

unsigned long get_cycles();

void clock_set_next_event();

static inline void clock_init() { clock_set_next_event(); }

extern void clock_init();

#endif