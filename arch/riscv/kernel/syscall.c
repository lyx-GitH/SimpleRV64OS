#include "syscall.h"

extern void __ret_from_fork();

static inline int is_annoy_vma(struct vm_area_struct* vma) {
    return !!(vma->vm_flags & VM_ANONYM);
}

int sys_write(unsigned int fd, const char* buf, unsigned int size) {
    int cnt = 0;
    if (fd == STDOUT) {
        printk("$ ");
        for (cnt = 0; cnt < size && buf[cnt]; ++cnt) putc(buf[cnt]);
    }
    return cnt;
}

int sys_getpid() { return get_cur_task()->pid; }

uint64_t sys_fork(struct pt_regs* regs) {
    uint64_t cur_user_stack = read_csr(sscratch);
    struct task_struct* child_task = kalloc();  // alloc a page to the child
    int pid = add_task(child_task);

    if (pid == -1) {
        printk("add task failed!\n");
        halt();
    }

    memcpy(child_task, get_cur_task(), PGSIZE);  // copy everything to child

    uint64_t regs_offset = (char*)regs - (char*)get_cur_task();

    struct pt_regs* child_regs =
        (struct pt_regs*)((char*)child_task + regs_offset);

    // assign attrs for the child proc
    child_task->pid = pid;
    child_task->priority = rand() % PRIORITY_MAX + 1;
    child_task->thread.sp = (uint64_t)child_regs;
    child_task->thread.ra = (uint64_t)__ret_from_fork;
    child_task->thread.sscratch = cur_user_stack;

    child_regs->regs[r_a0 - 1] = 0;
    // printk("before sp %lx\n", child_regs->regs[r_sp -1]);
    child_regs->regs[r_sp - 1] = child_task->thread.sp;
    // printk("after sp %lx\n", child_regs->regs[r_sp -1]);
    child_regs->sepc += 4;

    // shallow copy the kernel pgtbl
    child_task->page_table = kalloc();
    pgcpy(child_task->page_table, get_kernel_pgtbl());

    // deep copy the user page table
    for (int i = 0; i < child_task->vma_cnt; i++) {
        struct vm_area_struct* target_vma = &(child_task->vmas[i]);
        copy_vma_mem(target_vma, get_cur_task()->page_table,
                     child_task->page_table);
    }

    // context swicth
    switch_to(child_task);

    return sys_getpid();  // return value as parent's pid
}