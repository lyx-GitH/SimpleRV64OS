
.extern start_kernel

    .section .text.init
    .globl _start
_start:
    # Here is for lab4
    
    

    la sp, boot_stack   # 设置栈顶
    li a0, 4096         # 设置栈大小 4096B
    add sp, sp, a0      # 向上扩展栈空间


    call setup_vm       # 设置早期页表
    call relocate       # 重定位栈

    ##########################
    # 以下所有行为均在虚拟地址区 #
    ##########################

    # set stvec = _traps
    la a0, _traps
    csrw stvec, a0
    
    # set sie[STIE] = 1
    li a0, 1<<5
    csrw sie, a0
    
    # set sstatus[SIE] = 1, not use for lab5
    li a0, 2
    csrrs x0, sstatus, a0

    call start_kernel   # 执行内核启动函数
    

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    add ra, ra, t0
    add sp, sp, t0
   

    # set satp with early_pgtbl
    li t0, 8
    slli t0, t0, 60         # t0 = 8 << 60 as sv39 mode
    la t1, early_pgtbl      # r1 = &(early_pgtbl)
    srli t1, t1, 12         # r1 = &(early_pgtbl) >> 12
    or t0, t0, t1           # t0 = t0 | t1
    csrw satp, t0           # set satp
    

    # flush tlb
    sfence.vma zero, zero
    ret



    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top:
