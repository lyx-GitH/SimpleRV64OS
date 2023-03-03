#include "elfutils.h"

uint64_t perm_elf2vm(uint64_t elf_perm) {
    return !!(elf_perm | PF_W) * VM_W_MASK | !!(elf_perm | PF_R) * VM_R_MASK |
           !!(elf_perm | PF_X) * VM_X_MASK;
}

uint64_t perm_elf2pte(uint64_t elf_perm) {
    return !!(elf_perm | PF_W) * PTE_W | !!(elf_perm | PF_R) * PTE_R |
           !!(elf_perm | PF_X) * PTE_X;
}

typedef struct mem_section {
    uint64_t base;
    uint64_t top;
} MemSection;

// how many pages in a section?
uint64_t section_pages(MemSection mem_section) {
    return (mem_section.top - mem_section.base) / PGSIZE;
}

// addr[offset] = data
void set_byte(uint64_t addr, uint64_t offset, uint8_t data) {
    *((char*)(addr) + offset) = data;
}

// return addr[offset]
char get_byte(uint64_t addr, uint64_t offset) {
    return *((char*)(addr) + offset);
}

// map elf permitions to RV64 page-entry permisions
static uint64_t map_elf_perm(uint64_t elf_perm) {
    return !!(elf_perm | PF_W) * PTE_W | !!(elf_perm | PF_R) * PTE_R |
           !!(elf_perm | PF_X) * PTE_X;
}

// alloc chunk of memory for the elf sections
void alloc_elf_mem(uint64_t file_seg_start, uint64_t file_size,
                          uint64_t mem_size, uint64_t where_to_map,
                          page_table_t pgtbl, uint64_t perm) {
    MemSection mapped_mem_section = {PGROUNDDOWN(where_to_map),
                                     PGROUNDUP(where_to_map + mem_size)};

    uint64_t mem_pages = section_pages(mapped_mem_section);
    uint64_t phy_mem_secction_start = alloc_pages(mem_pages);
    memset((void*)phy_mem_secction_start, 0, mem_pages * PGSIZE);

    uint64_t prev_blanks = where_to_map - mapped_mem_section.base;

    /** copy and align
     *
     * |.......|xxxxxxxxxxxxxxxxxxxx|...........|........|
     * pg_s    data_s              data_e       mem_e    pg_end
     */
    for (int i = 0; i < file_size; i++) {
        set_byte(phy_mem_secction_start, prev_blanks + i,
                 get_byte(file_seg_start, i));
    }
    uint64_t va = mapped_mem_section.base;
    uint64_t pa = phy_mem_secction_start - PA2VA_OFFSET;
    for (; va < mapped_mem_section.top; va += PGSIZE, pa += PGSIZE) {
        map_final_pgtbl(pgtbl, va, pa, perm, true);
    }
}

// load progam to the mem


