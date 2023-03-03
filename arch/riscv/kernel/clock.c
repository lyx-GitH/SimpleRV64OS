#include "clock.h"

#include "sbi.h"

unsigned long get_cycles() {
    // 使用 rdtime 编写内联汇编，获取 time 寄存器中 (也就是mtime 寄存器
    // )的值并返回
    unsigned long cycles = 0;
    __asm__ volatile("rdtime %[cycles]" : [cycles] "=r"(cycles) : : "memory");

    return cycles;
}

void clock_set_next_event() {
    unsigned long TIMECLOCK = 10000000;
    // 下一次 时钟中断 的时间点
    unsigned long next = get_cycles() + TIMECLOCK;
    // 使用 sbi_ecall 来完成对下一次时钟中断的设置
    sbi_ecall(0x00, 0x00, next, 0, 0, 0, 0, 0);
}
