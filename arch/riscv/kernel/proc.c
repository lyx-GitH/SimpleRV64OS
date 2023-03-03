#include "proc.h"

extern void __dummy();

struct task_struct *idle;     // idle process
struct task_struct *current;  // 指向当前运行线程的 `task_struct`
struct task_struct *task[NR_TASKS];  // 线程数组，所有的线程都保存在此

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS - 1]

struct task_struct *get_cur_task() { return current; }

void print_task(struct task_struct *taskStruct) {
    printk("PID : [%d], counter: [%d], prio: [%d], sp:[%lx], sepc:[%lx]\n",
           taskStruct->pid, taskStruct->counter, taskStruct->priority,
           taskStruct->thread.sp, taskStruct->thread.sepc);
}

int add_task(struct task_struct *task_struct) {
    int i;
    for (i = 2; i < NR_TASKS && task[i] != NULL; i++)
        ;
    if (i != NR_TASKS && task[i] == NULL) {
        task[i] = task_struct;
        return i;
    } else
        return -1;
}

// assign the pagetable for the user thread
void setup_user_pgtbl(struct task_struct *taskStruct) {
    // alloc a page for pgtbl and copy the kernel pgtbl to the new pgtbl
    taskStruct->page_table = (page_table_t)(kalloc());

    for (int i = 0; i < 512; i++) {
        taskStruct->page_table[i] = get_kernel_pgtbl()[i];
    }

    // const uint64 user_perm = PTE_V | PTE_W | PTE_R | PTE_U;

    // uint64 pa = pkalloc();          // alloc a page for user stack
    // uint64 va = USER_END - PGSIZE;  // the final page of the user area
    // map_final_pgtbl(taskStruct->page_table, va, pa, user_perm, true);
    do_mmap(taskStruct, USER_END - PGSIZE, PGSIZE,
            VM_R_MASK | VM_W_MASK | VM_ANONYM, 0, 0);
}

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle
    idle = (struct task_struct *)(kalloc());

    idle->state = TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;
    idle->pid = 0;
    current = idle;
    task[0] = idle;

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用
    // rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和
    // `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为
    // 该线程申请的物理页的高地址

    uint64 dummy_addr = (uint64)(&__dummy);

    for (int i = 1; i < 2; i++) {
        task[i] = (struct task_struct *)(kalloc());
        task[i]->pid = i;
        task[i]->state = TASK_RUNNING;
        task[i]->counter = DEFUALT_COUNTER;
        task[i]->priority = rand() % PRIORITY_MAX + 1;
        task[i]->thread.ra = dummy_addr;

        task[i]->thread.sp = (uint64)task[i] + PGSIZE - 1;
        setup_user_pgtbl(task[i]);  // map user stcak for this task
        load_program(task[i]);      // load elf program
        task[i]->thread.sstatus = SSTATUS_SPIE | SSTATUS_SUM;
        task[i]->thread.sscratch = USER_END;

        print_task(task[i]);
    }

    for (int i = 2; i < NR_TASKS; i++) {
        task[i] = NULL;
    }
    printk("...proc_init done!\n");
}

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while (1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. auto_inc_local_var = %d\n",
                   current->pid, auto_inc_local_var);
        }
    }
}

extern void __switch_to(void *prev, void *next, uint64 next_phy_pgtbl);

void switch_to(struct task_struct *next) {
    print_task(next);
    struct task_struct *old_current = current;
    current = next;
    printk("from [%d] switch to [%d]\n", old_current->pid, next->pid);
    uint64 next_satp = get_satp(next->page_table);
    __switch_to(&(old_current->thread), &(next->thread), next_satp);
}

void do_timer(void) {
    /* 1. 如果当前线程是 idle 线程 直接进行调度
    /* 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间任然大于0
    则直接返回 否则进行调度*/
    if (current == idle)
        schedule();
    else if (current->counter == 0)
        schedule();
    else if (--current->counter == 0)
        schedule();
}

void assign_counter() {
    for (int i = 1; i < NR_TASKS; i++) {
        if (task[i] == NULL) continue;
        task[i]->counter = rand() % MAX_TIME + 1;
        printk("SET PID [%d] as counter [%d]\n", i, task[i]->counter);
    }
}

void schedule_short_first(void) {
    /* YOUR CODE HERE */
    current->counter = ~((unsigned long)0);  // hang this task
    int all_zero_flag = 0;
    int min_counter_i = 1;
start:

    for (int i = 1; i < NR_TASKS; i++) {
        if (task[i] == NULL) continue;
        min_counter_i =
            task[min_counter_i]->counter < task[i]->counter ? min_counter_i : i;
        all_zero_flag |= ~(task[i]->counter);
    }
    if (!all_zero_flag) {
        assign_counter();
        goto start;
    } else {
        switch_to(task[min_counter_i]);
    }
}

void schedule_priority(void)  // copied from the linux src code
{
    int i, next, c;
    struct task_struct **p;

    while (true) {
        c = 0;
        next = 0;
        for (i = NR_TASKS - 1; i > 0; i--) {
            if (task[i] == NULL) continue;

            if (task[i]->counter > c) {
                c = task[i]->counter;
                next = i;
            }
        }
        if (c > 0)
            break;  // since the priority is unsigned, c must be unsigned as
                    // well
        for (i = NR_TASKS - 1; i > 0; i--) {
            if (task[i] == NULL) continue;

            task[i]->counter = task[i]->priority + (task[i]->counter >> 1);
        }
    }
    switch_to(task[next]);
}

void schedule(void) {
#ifndef PRIO_SCHED
    schedule_short_first();
#else
    schedule_priority();
#endif
}

void do_mmap(struct task_struct *task, uint64_t addr, uint64_t length,
             uint64_t flags, uint64_t vm_content_offset_in_file,
             uint64_t vm_content_size_in_file) {
    struct vm_area_struct *p_last_vma =
        &(task->vmas[task->vma_cnt]);  // get the ptr to the latest vma
    task->vma_cnt += 1;                // add cnt
    // set vma attrs
    p_last_vma->vm_start = addr;
    p_last_vma->vm_end = addr + length;
    p_last_vma->vm_flags = flags;
    p_last_vma->vm_content_offset_in_file = vm_content_offset_in_file;
    p_last_vma->vm_content_size_in_file = vm_content_size_in_file;
}

struct vm_area_struct *find_vma(struct task_struct *task, uint64_t addr) {
    if (task->vma_cnt == 0) return NULL;

    int i;
    // traverse all the vmas
    for (i = 0; i < task->vma_cnt; i++) {
        uint64_t addr_s = task->vmas[i].vm_start;
        uint64_t addr_e = task->vmas[i].vm_end;
        if (addr >= addr_s && addr < addr_e) break;
    }

    return i == task->vma_cnt ? NULL : &(task->vmas[i]); // if not found, 
}

void load_elf_program(struct task_struct *task, Elf64_Ehdr *ehdr) {
    // User program, so PTE_U must be set
    uint64_t elf_perm = perm_elf2pte(ehdr->e_flags) | PTE_V | PTE_U;
    uint64_t phdr_start = (uint64_t)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;

    Elf64_Phdr *phdr;
    int load_phdr_cnt = 0;
    for (int i = 0; i < phdr_cnt; i++) {
        phdr = (Elf64_Phdr *)(phdr_start + sizeof(Elf64_Phdr) * i);
        if (phdr->p_type == PT_LOAD) {
            // start of the segement bin data in the file
            char *file_seg_start_addr = (char *)(ehdr) + phdr->p_offset;
            // alloc_elf_mem((uint64_t)file_seg_start_addr, phdr->p_filesz,
            // phdr->p_memsz, phdr->p_vaddr, pgtbl, elf_perm);
            // just add a vma to this task
            do_mmap(task, phdr->p_vaddr, phdr->p_memsz,
                    perm_elf2vm(phdr->p_flags), phdr->p_offset, phdr->p_filesz);
        }
    }
}
