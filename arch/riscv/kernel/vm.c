#include "vm.h"

unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

// 设置某页表的某一项为确定的值和权限
uint64 setup_pte(page_table_t page_table_base, uint64 pte_idx, uint64 perm) {
    uint64 page_pa = pkalloc();
    page_table_base[pte_idx] = ADDR2PTE(page_pa) | perm;
    return page_table_base[pte_idx];
}

// 早期页表单级映射
void map_early_pgtbl(uint64 va, uint64 pa, uint64 perm) {
    uint64 idx = (va >> 30) & EARLY_PT_INDEX_MASK;
    uint64 ppn = PG_NUM(pa);
    early_pgtbl[idx] = VPN(ppn) | perm;
}

// 最终页表的三级映射
void map_final_pgtbl(page_table_t pgtbl, uint64 va, uint64 pa, uint64 perm,
                     int alloc) {
    uint64 vpn[3] = {VPN_0(va), VPN_1(va), VPN_2(va)};

    uint64 pte;
    pte = pgtbl[vpn[2]];
    if (!PTE_VALID(pte)) {
        pte = setup_pte(pgtbl, vpn[2], PTE_V);
    }
    pgtbl = PTE2VA(pte);

    pte = pgtbl[vpn[1]];
    if (!PTE_VALID(pte)) {
        pte = setup_pte(pgtbl, vpn[1], PTE_V);
    }
    pgtbl = PTE2VA(pte);

    perm |= PTE_V;
    pgtbl[vpn[0]] = ADDR2PTE(pa) | perm;
}

int is_va_mapped(uint64_t va, page_table_t pgtbl) {
    uint64 vpn[3] = {VPN_0(va), VPN_1(va), VPN_2(va)};

    uint64 pte;
    pte = pgtbl[vpn[2]];
    if (!PTE_VALID(pte)) {
       return false;
    }
    pgtbl = PTE2VA(pte);

    pte = pgtbl[vpn[1]];
    if (!PTE_VALID(pte)) {
        return false;
    }
    pgtbl = PTE2VA(pte);
    pte = pgtbl[vpn[0]];
    return !!(PTE_VALID(pte)); // is valid bit 1
}



/*
    1. 如前文所说，只使用第一级页表
    2. 因此 VA 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间 9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为页内偏移
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
*/
void setup_vm(void) {
    
    memset(early_pgtbl, 0x0, PGSIZE);
    uint64 perm = PTE_V | PTE_R | PTE_W | PTE_X;  // 设置权限

    for (uint64 i = 0; i < VM_SIZE; i += EARLY_PGSIZE) {
        map_early_pgtbl(PHY_START + i, PHY_START + i, perm);  // 第一轮等值映射
        map_early_pgtbl(VM_START + i, PHY_START + i, perm);  // 第二轮平移映射
    }

    reg_write(t0, PA2VA_OFFSET);  // prepare offset for relocate
    printk("setup vm complete\n");
    return;
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

uint64 get_satp(page_table_t pgtbl) {
    uint64 phy_pgtbl = (uint64)(pgtbl)-PA2VA_OFFSET;
    return (SV39 << 60) | (phy_pgtbl >> 12);
}

page_table_t get_kernel_pgtbl() { return swapper_pg_dir; }

void setup_vm_final(void) {
    uint64 base = PHY_START + OPENSBI_SIZE;
    uint64 end = PHY_END;
    // uint64 ultra_perm = PTE_R | PTE_W | PTE_V | PTE_X;

    uint64 kernel_start = VM_START + OPENSBI_SIZE;
    uint64 text_end = PGROUNDUP((uint64)(_etext));
    uint64 rodata_end = PGROUNDUP((uint64)(_erodata));
    uint64 mapped_mem_end = PHY_END + PA2VA_OFFSET;

    uint64 va;

    // mapping kernel text X|-|R|V
    for (va = kernel_start; va < text_end; va += PGSIZE) {
        map_final_pgtbl(swapper_pg_dir, va, VA2PA(va), PTE_V | PTE_R | PTE_X,
                        true);
    }

    // mapping kernel rodata -|-|R|V
    for (va = text_end; va < rodata_end; va += PGSIZE) {
        map_final_pgtbl(swapper_pg_dir, va, VA2PA(va), PTE_V | PTE_R, true);
    }

    // mapping other memory -|W|R|V
    for (va = rodata_end; va < mapped_mem_end; va += PGSIZE) {
        map_final_pgtbl(swapper_pg_dir, va, VA2PA(va), PTE_W | PTE_R | PTE_V,
                        true);
    }
}

/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    printk("mapping : %lx, %lx, %d\n", va, pa, sz);
    uint64 base = PGROUNDUP(va);
    uint64 top = base + sz;

    for (va = base; va < top; va += PGSIZE, pa += PGSIZE)
        map_final_pgtbl(pgtbl, va, pa, perm, true);
}
