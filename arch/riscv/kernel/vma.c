#include "vma.h"

void alloc_annoymous_vm(struct vm_area_struct* vma, page_table_t pgtbl) {
    // align the vm to 0x1000
    uint64_t va = PGROUNDDOWN(vma->vm_start);
    uint64_t mem_size = PGROUNDUP(vma->vm_end) - va;
    uint64_t mem_pages = PG_NUM(mem_size);
    uint64_t real_va = alloc_pages(mem_pages);
    // clear this vm
    memset((void*)real_va, 0, mem_pages * PGSIZE);
    uint64_t pa = real_va - PA2VA_OFFSET;

    uint64_t pte_perm = map_vm_perm(vma->vm_flags) | PTE_U | PTE_V;
    // do pgtbl mapping
    for (int i = 0; i < mem_pages; i++) {
        map_final_pgtbl(pgtbl, va + i * PGSIZE, pa + i * PGSIZE, pte_perm,
                        true);
    }
}

void copy_vma_mem(struct vm_area_struct* vma, page_table_t parent_table,
                  page_table_t child_table) {
    uint64_t vma_page_start = PGROUNDDOWN(vma->vm_start);
    for (uint64_t va = vma_page_start; va < vma->vm_end; va += PGSIZE) {
        if (is_va_mapped(va, parent_table)) {  // this page is actually alloced
            uint64_t real_va = kalloc();
            pgcpy((void*)real_va, (void*)va);  // copy data to this new page
            uint64_t pte_perm = map_vm_perm(vma->vm_flags) | PTE_U | PTE_V;
            map_final_pgtbl(child_table, va, real_va - PA2VA_OFFSET, pte_perm,
                            true);
        }
    }
}