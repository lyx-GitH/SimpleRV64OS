#ifndef RISCV_OS_DEFS_H
#define RISCV_OS_DEFS_H

#define true 1
#define false 0

#define N_REGS 32

// aliases for registers
#define r_zero 0
#define r_ra 1
#define r_sp 2
#define r_gp 3
#define r_tp 4
#define r_t0 5
#define r_t1 6
#define r_t2 7
#define r_fp 8
#define r_s1 9
#define r_a0 10
#define r_a1 11
#define r_a2 12
#define r_a3 13
#define r_a4 14
#define r_a5 15
#define r_a6 16
#define r_a7 17
#define r_s2 18
#define r_s3 19
#define r_s4 20
#define r_s5 21
#define r_s6 22
#define r_s7 23
#define r_s8 24
#define r_s9 25
#define r_s10 26
#define r_s11 27
#define r_t3 28
#define r_t4 29
#define r_t5 30
#define r_t6 31

#define TIMER_INTR 0x8000000000000005  // Interrupt =1 && Exeption code = 5
#define INST_PAGE_FAULT 0x0c
#define LOAD_PAGE_FAULT 0x0d
#define STORE_PAGE_FAULT 0x0f
#define ECALL_FROM_U_MODE 0x08

#define read_csr(reg)                               \
    ({                                              \
        unsigned long __v;                          \
        asm volatile("csrr %0, " #reg : "=r"(__v)); \
        __v;                                        \
    })

#define read_register(reg)                        \
    ({                                            \
        unsigned long __v;                        \
        asm volatile("mv %0, " #reg : "=r"(__v)); \
        __v;                                      \
    })

#define csr_write(csr, val)                                        \
    ({                                                             \
        unsigned long __v = (unsigned long)(val);                  \
        asm volatile("csrw " #csr ", %0" : : "r"(__v) : "memory"); \
    })

#define reg_write(csr, val)                                      \
    ({                                                           \
        unsigned long __v = (unsigned long)(val);                \
        asm volatile("mv " #csr ", %0" : : "r"(__v) : "memory"); \
    })

#define MAX_TIME 10

#define PRIO_SCHED

#ifdef PRIO_SCHED
#define DEFUALT_COUNTER 0
#else
#define DEFUALT_COUNTER ~((unsigned long)0)
#endif

#define PHY_START 0x0000000080000000
#define PHY_SIZE 128 * 1024 * 1024  // 128MB， QEMU 默认内存大小
#define PHY_END (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000  // 4KB
#define EARLY_PGSIZE (1L << 30)
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))

#define OPENSBI_SIZE (0x200000)

#define VM_START (0xffffffe000000000)
#define VM_END (0xffffffff00000000)
#define VM_SIZE (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

#define SV39 (8L)
#define EARLY_PT_INDEX_MASK (0x1ff)
#define IDX_MASK EARLY_PT_INDEX_MASK
#define WIDE_IDX_MASK 0x3ffffff
#define PG_NUM(addr) ((uint64)(addr) >> 12)
#define VPN(pg_idx) ((uint64)(pg_idx) << 10)

#define VPN_0(va) (((va) >> 12) & IDX_MASK)
#define VPN_1(va) (((va) >> 21) & IDX_MASK)
#define VPN_2(va) (((va) >> 30) & IDX_MASK)

#define PPN_0(pte) (((va) >> 12) & IDX_MASK)
#define PPN_1(pte) (((va) >> 21) & IDX_MASK)
#define PPN_2(pte) (((va) >> 30) & WIDE_IDX_MASK)

#define PTE2ADDR(pte) (((uint64)(pte) >> 10) << 12)
#define ADDR2PTE(addr) (((uint64)(addr) >> 12) << 10)
#define VA2PTE(va) (ADDR2PTE((va)-PA2VA_OFFSET))
#define PTE2VA(pte) (PA2VA_OFFSET + PTE2ADDR(pte))


#define VA2PA(x) ((x - (uint64_t)PA2VA_OFFSET))
#define PA2VA(x) ((x + (uint64_t)PA2VA_OFFSET))
#define PFN2PHYS(x) (((uint64_t)(x) << 12) + PHY_START)
#define PHYS2PFN(x) ((((uint64_t)(x) - PHY_START) >> 12))


#define PTE_V (1L)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4)
#define PTE_G (1L << 5)
#define PTE_A (1L << 6)
#define PTE_D (1L << 7)
#define PTE_VALID(pte) ((pte)&PTE_V)

#define USER_START (0x0000000000000000)  // user space start virtual address
#define USER_END (0x0000004000000000)    // user space end virtual address

#define SSTATUS_SPP (1L << 8)
#define SSTATUS_SPIE (1L << 5)
#define SSTATUS_SUM (1L << 18)

#define STDOUT 1

#endif
