#include "defs.h"
#include "elf.h"
#include "mm.h"
#include "vm.h"
#include "vma.h"

// map the elf data into the pagetable

void alloc_elf_mem(uint64_t file_seg_start, uint64_t file_size,
                          uint64_t mem_size, uint64_t where_to_map,
                          page_table_t pgtbl, uint64_t perm);
                        

uint64_t perm_elf2vm(uint64_t elf_perm) ;

uint64_t perm_elf2pte(uint64_t elf_perm);

