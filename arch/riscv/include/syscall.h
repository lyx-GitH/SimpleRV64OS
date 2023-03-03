#ifndef RV64_OS_SYSCALL_H
#define RV64_OS_SYSCALL_H

#include "defs.h"
#include "halt.h"
#include "printk.h"
#include "proc.h"
#include "string.h"
#include "trap.h"
#include "vm.h"
#include "vma.h"

#define SYS_WRITE 64
#define SYS_GETPID 172
#define SYS_CLONE 220

int sys_write(unsigned int fd, const char* buf, unsigned int size);

int sys_getpid();

uint64_t sys_fork(struct pt_regs* regs);

#endif