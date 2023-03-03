#ifndef RISCV_OS_MM_H
#define RISCV_OS_MM_H

#include "stdint.h"
#include "types.h"

extern char _stext[];
extern char _etext[];
extern char _srodata[];
extern char _erodata[];
extern char uapp_start[];
extern char uapp_end[];

struct run {
    struct run *next;
};

void mm_init();

uint64 kalloc();
uint64 pkalloc();
void kfree(uint64);

struct buddy {
    uint64_t size;
    uint64_t *bitmap;
};

void buddy_init();
uint64_t buddy_alloc(uint64_t);
void buddy_free(uint64_t);

uint64_t alloc_pages(uint64_t);
uint64_t alloc_page();
void free_pages(uint64_t);

uint64 get_etext_start();
uint64 get_etext_end();
uint64 get_rodata_start();
uint64 get_rodata_end();
uint64 get_ekerel();
uint64 get_uapp_start();
uint64 get_uapp_end();
// uint64 get_allocated_pages();

#endif
