#include "printk.h"
#include "clock.h"
#include "sbi.h"
#include "defs.h"
#include "proc.h"
#include "vm.h"
#include "trap.h"

extern void test();

int start_kernel()
{
    printk("kernel booting ...\n");

    mm_init();

    vm_init();

    task_init();

    clock_init();

    puts("Hello RISC-V!, start schedule:\n");

    schedule();

    test(); // DO NOT DELETE !!!

    return 0;
}
