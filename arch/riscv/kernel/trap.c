#include "trap.h"



static inline int is_annoy_vma(struct vm_area_struct* vma) {
    return !!(vma->vm_flags & VM_ANONYM);
}

void do_page_fault(struct pt_regs* trap_frame) {
    uint64_t err_addr = read_csr(stval);
    struct vm_area_struct* target_vma = find_vma(get_cur_task(), err_addr);
    // no such mem 
    if (!target_vma) {
        printk("error!, address %lx not found!\n", err_addr);
        halt();
    }

    if (is_annoy_vma(target_vma)) {
        alloc_annoymous_vm(target_vma, get_cur_task()->page_table);
    } else {
        uint64_t pte_perm = map_vm_perm(target_vma->vm_flags) | PTE_U | PTE_V;
        uint64_t file_seg_start =
            (uint64_t)uapp_start + target_vma->vm_content_offset_in_file;
        uint64_t local_offset = err_addr - target_vma->vm_start;
        uint64_t file_page_start = PGROUNDDOWN(file_seg_start + local_offset);

        alloc_elf_mem(file_page_start, PGSIZE, PGSIZE, PGROUNDDOWN(err_addr),
                      get_cur_task()->page_table, pte_perm);
    }
}

void trap_handler(unsigned long scause, unsigned long sepc,
                  struct pt_regs* trap_frame) {
    // 通过 `scause` 判断trap类型
    // 如果是interrupt 判断是否是timer interrupt
    // 如果是timer interrupt 则打印输出相关信息, 并通过 `clock_set_next_event()`
    // 设置下一次时钟中断 `clock_set_next_event()` 见 4.5 节 其他interrupt /
    // exception 可以直接忽略
    if (scause == TIMER_INTR) {
        printk("This is a timer interupt!\n");
        clock_set_next_event();
        do_timer();
    } else if (scause == INST_PAGE_FAULT) {
        printk("pc @ %lx hits a inst page fault, stval = %lx\n", read_csr(sepc),
               read_csr(stval));
        do_page_fault(trap_frame);
    } else if (scause == LOAD_PAGE_FAULT) {
        printk("pc @ %lx hits a load page fault, stval = %lx\n", read_csr(sepc),
               read_csr(stval));
        do_page_fault(trap_frame);

    } else if (scause == STORE_PAGE_FAULT) {
        printk("pc @ %lx hits a store page fault, stval = %lx\n",
               read_csr(sepc), read_csr(stval));
        do_page_fault(trap_frame);

    } else if (scause == ECALL_FROM_U_MODE) {
        
        uint64 syscall_id = trap_frame->regs[r_a7 - 1];
        // printk("ecall is caught at %lx with id %d, a7 is : %d, a0 is : %d\n",
        // sepc, syscall_id, read_register(a7), read_register(a0));
        if (syscall_id == SYS_GETPID) {
            uint64 pid = sys_getpid();
            trap_frame->regs[r_a0 - 1] = pid;
        } else if (syscall_id == SYS_WRITE) {
            unsigned int fd = (unsigned int)trap_frame->regs[r_a0 - 1];
            const char* buf = (const char*)(trap_frame->regs[r_a1 - 1]);
            size_t size = (size_t)(trap_frame->regs[r_a2 - 1]);
            // printk("fd = %d, buf = %s, size = %d\n", fd, buf, size);
            trap_frame->regs[r_a0 - 1] = sys_write(fd, buf, size);
        } else if (syscall_id == SYS_CLONE) {
            uint64_t pid = sys_fork(trap_frame);
            trap_frame->regs[r_a0 - 1] = pid;
        }

        // move sepc forward
        trap_frame->sepc += 4;
    }
}
