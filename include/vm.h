#ifndef RISCV_OS_VM_H
#define RISCV_OS_VM_H

#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "string.h"
#include "types.h"

uint64 setup_pte(page_table_t page_table_base, uint64 pte_idx, uint64 perm);

void map_final_pgtbl(page_table_t pgtbl, uint64 va, uint64 pa, uint64 perm,
                     int alloc);

void setup_vm(void);

void setup_vm_final(void);

int is_va_mapped(uint64_t va, page_table_t pgtbl);

uint64 get_satp(page_table_t pgtbl);

page_table_t get_kernel_pgtbl();


static void vm_init() {
    setup_vm_final();
    uint64 satp_val = get_satp(get_kernel_pgtbl());
    asm volatile("csrw satp, %0" : : "r"(satp_val));
    asm volatile("sfence.vma zero, zero");
    printk("...vm_init done!\n");
}

extern void vm_init();

#endif
