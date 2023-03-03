#ifndef RV64_OS_VMA_H
#define RV64_OS_VMA_H

#include "defs.h"
#include "stdint.h"
#include "types.h"
#include "vm.h"

#define VM_X_MASK 0x0000000000000008
#define VM_W_MASK 0x0000000000000004
#define VM_R_MASK 0x0000000000000002
#define VM_ANONYM 0x0000000000000001

struct vm_area_struct {
    uint64_t vm_start; /* VMA 对应的用户态虚拟地址的开始 */
    uint64_t vm_end;   /* VMA 对应的用户态虚拟地址的结束 */
    uint64_t vm_flags; /* VMA 对应的 flags */
    /* uint64_t file_offset_on_disk */ /* 原本需要记录对应的文件在磁盘上的位置，但是我们只有一个文件
                                          uapp，所以暂时不需要记录 */
    uint64_t
        vm_content_offset_in_file; /* 如果对应了一个文件，那么这块 VMA
                                      起始地址对应的文件内容相对文件起始位置的偏移量，也就是
                                      ELF 中各段的 p_offset 值 */
    uint64_t
        vm_content_size_in_file; /* 对应的文件内容的⻓度。思考为什么还需要这个域?和
                                    (vm_end-vm_start)一比，不是冗余了吗? */
};

// vm flags to rv64 pte perms
static uint64_t map_vm_perm(uint64_t vm_perm) {
    return !!(vm_perm & VM_R_MASK) * PTE_R | !!(vm_perm & VM_W_MASK) * PTE_W |
           !!(vm_perm & VM_X_MASK) * PTE_X;
}

// alloc a block of annoy vma mems.
void alloc_annoymous_vm(struct vm_area_struct* vma, page_table_t pgtb);

// assume that vmas has been aligned, but memroy is not
void copy_vma_mem(struct vm_area_struct* vma, page_table_t parent_table,
                  page_table_t child_table);

extern uint64_t map_vm_perm(uint64_t vm_perm);

#endif