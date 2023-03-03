// arch/riscv/include/proc.h
#ifndef RISCV_OS_PROC_H
#define RISCV_OS_PROC_H

#include "defs.h"
#include "elf.h"
#include "elfutils.h"
#include "mm.h"
#include "printk.h"
#include "rand.h"
#include "string.h"
#include "types.h"
#include "vm.h"
#include "vma.h"

#define NR_TASKS (1 + 15)  // 用于控制 最大线程数量 （idle 线程 + 31 内核线程）

#define TASK_RUNNING 0  // 为了简化实验，所有的线程都只有一种状态

#define PRIORITY_MIN 1
#define PRIORITY_MAX 10

struct pt_regs{
    uint64 regs[N_REGS - 1];
    uint64 sepc;
    uint64 sstatus;
};

/* 用于记录 `线程` 的 `内核栈与用户栈指针` */
/* (lab3中无需考虑，在这里引入是为了之后实验的使用) */
struct thread_info {
    uint64 kernel_sp;
    uint64 user_sp;
};

/* 线程状态段数据结构 */
struct thread_struct {
    uint64 ra;
    uint64 sp;
    uint64 s[12];
    uint64 sepc;
    uint64 sstatus;
    uint64 sscratch;
};



/* 线程数据结构 */
struct task_struct {
    struct thread_info *thread_info;
    uint64 state;     // 线程状态
    uint64 counter;   // 运行剩余时间
    uint64 priority;  // 运行优先级 1最低 10最高
    uint64 pid;       // 线程id

    struct thread_struct thread;
    page_table_t page_table;

    uint64_t vma_cnt;
    struct vm_area_struct vmas[0];
};

/* 线程初始化 创建 NR_TASKS 个线程 */
void task_init();

/* 在时钟中断处理中被调用 用于判断是否需要进行调度 */
void do_timer();

/* 调度程序 选择出下一个运行的线程 */
void schedule();

/* 线程切换入口函数*/
void switch_to(struct task_struct *next);

/* dummy funciton: 一个循环程序，循环输出自己的 pid 以及一个自增的局部变量*/
void dummy();

struct task_struct *get_cur_task();

int add_task(struct task_struct* task);

void setup_user_pgtbl(struct task_struct *taskStruct);

void load_elf_program(struct task_struct* task, Elf64_Ehdr* ehdr);

void do_mmap(struct task_struct *task, uint64_t addr, uint64_t length,
             uint64_t flags, uint64_t vm_content_offset_in_file,
             uint64_t vm_content_size_in_file);

struct vm_area_struct *find_vma(struct task_struct *task, uint64_t addr);

static uint64_t load_program(struct task_struct *task) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)uapp_start;

    load_elf_program(task, ehdr);

    // set function address
    task->thread.sepc = ehdr->e_entry;
}




#endif
