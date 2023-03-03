
    .section .text.entry
    .align 2
    .global _traps 
_traps:
    # YOUR CODE HERE
    # -----------
        # here the sp reg is in user mode, and the sscartch is in the kernel mode
       # swap sp and ssc, to get stack into the kernel state
      
    
        # addi sp, sp, -8
        # sd t0, 0(sp)

        # csrr t0, sscratch # t0 = ssc
        # csrw sscratch, sp # ssc = sp0-8
        # mv sp, t0         #sp = ssc


        # // sscratch += 8
        # csrr t0, sscratch # t0 = ssc = sp0-8
        # addi t0, t0, 8      # t0 = sp0
        # csrw sscratch, t0  # ssc = sp0

        # addi t0, t0, -8     # t0 = sp0 -8 = sp after stack op
        # ld t0, 0(t0)        # restore stack and t0 reg

        csrr t0, sscratch
        csrw sscratch, sp
        mv sp, t0

        bne sp, x0, save_regs # if this is a kernel exp, swap them back

        csrr t0, sscratch
        csrw sscratch, sp
        mv sp, t0

        



        # 1. save 32 registers and sepc to stack
    save_regs:
        addi  sp, sp, -264
        sd    x1, 0(sp)
        sd    x2, 8(sp)
        sd    x3, 16(sp)
        sd    x4, 24(sp)
        sd    x5, 32(sp)
        sd    x6, 40(sp)
        sd    x7, 48(sp)
        sd    x8, 56(sp)
        sd    x9, 64(sp)
        sd    x10, 72(sp)
        sd    x11, 80(sp)
        sd    x12, 88(sp)
        sd    x13, 96(sp)
        sd    x14, 104(sp)
        sd    x15, 112(sp)
        sd    x16, 120(sp)
        sd    x17, 128(sp)
        sd    x18, 136(sp)
        sd    x19, 144(sp)
        sd    x20, 152(sp)
        sd    x21, 160(sp)
        sd    x22, 168(sp)
        sd    x23, 176(sp)
        sd    x24, 184(sp)
        sd    x25, 192(sp)
        sd    x26, 200(sp)
        sd    x27, 208(sp)
        sd    x28, 216(sp)
        sd    x29, 224(sp)
        sd    x30, 232(sp)
        sd    x31, 240(sp)
        csrrs a1, sepc, x0      # a1 = sepc
        sd    a1,  248(sp)      
        csrrs t0, sstatus, x0  # t0 = sscratch
        sd    t0, 256(sp)
    # -----------

        # 2. call trap_handler
        csrrs a0, scause, x0    # a0 = scause
        mv a2, sp               # a2 = &trap_frame
        
        call trap_handler

    # -----------
        .global __ret_from_fork
    __ret_from_fork:
        # 3. restore sepc and 32 registers (x2(sp) should be restore last) from stack
        
        ld    t0, 248(sp)
        csrw  sepc, t0
        ld    t0, 256(sp)
        csrw  sstatus, t0
        ld    x1, 0(sp)
        ld    x3, 16(sp)
        ld    x4, 24(sp)
        ld    x5, 32(sp)
        ld    x6, 40(sp)
        ld    x7, 48(sp)
        ld    x8, 56(sp)
        ld    x9, 64(sp)
        ld    x10, 72(sp)
        ld    x11, 80(sp)
        ld    x12, 88(sp)
        ld    x13, 96(sp)
        ld    x14, 104(sp)
        ld    x15, 112(sp)
        ld    x16, 120(sp)
        ld    x17, 128(sp)
        ld    x18, 136(sp)
        ld    x19, 144(sp)
        ld    x20, 152(sp)
        ld    x21, 160(sp)
        ld    x22, 168(sp)
        ld    x23, 176(sp)
        ld    x24, 184(sp)
        ld    x25, 192(sp)
        ld    x26, 200(sp)
        ld    x27, 208(sp)
        ld    x28, 216(sp)
        ld    x29, 224(sp)
        ld    x30, 232(sp)
        ld    x31, 240(sp)
        ld    sp, 8(sp)
        

        addi  sp, sp, 264

        # swap sp and ssc again
         csrr t0, sscratch
        csrw sscratch, sp
        mv sp, t0

        bne sp, x0, jump_back

        csrr t0, sscratch
        csrw sscratch, sp
        mv sp, t0



    # -----------
    jump_back:
        # 4. return from trap
        sret

    # -----------


    .global __dummy
__dummy:
    # Swap sp and sscratch 
    mv t0, sp               # t0 = sp
    csrr sp, sscratch       # sp = ssc
    csrw sscratch, t0       # ssc = t0
    
   
    sret


# struct thread_struct {
#     uint64 ra;
#     uint64 sp;
#     uint64 s[12];
#     uint64 sepc;
#     uint64 sstatus;
#     uint64 sscratch;
# };

# __switch_to(prev.s, next.s, next.pgtbl)

   .global __switch_to
__switch_to:
    sd ra, 0(a0)
    sd sp, 8(a0)
    sd s0, 16(a0)
    sd s1, 24(a0)
    sd s2, 32(a0)
    sd s3, 40(a0)
    sd s4, 48(a0)
    sd s5, 56(a0)
    sd s6, 64(a0)
    sd s7, 72(a0)
    sd s8, 80(a0)
    sd s9, 88(a0)
    sd s10, 96(a0)
    sd s11, 104(a0)

    csrr t0, sepc
    sd t0, 112(a0)

    csrr t0, sstatus
    sd t0, 120(a0)

    csrr t0, sscratch
    sd t0, 128(a0)
    

    ld ra, 0(a1)
    ld sp, 8(a1)
    ld s0, 16(a1)
    ld s1, 24(a1)
    ld s2, 32(a1)
    ld s3, 40(a1)
    ld s4, 48(a1)
    ld s5, 56(a1)
    ld s6, 64(a1)
    ld s7, 72(a1)
    ld s8, 80(a1)
    ld s9, 88(a1)
    ld s10, 96(a1)
    ld s11, 104(a1)

    ld t0, 112(a1)
    csrw sepc, t0

    ld t0, 120(a1)
    csrw sstatus, t0

    ld t0, 128(a1)
    csrw sscratch, t0

    csrw satp, a2                   # set page table

    sfence.vma zero, zero

    ret

    