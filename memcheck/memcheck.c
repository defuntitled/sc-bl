/*
 * Copyright (C) 2023, Syntacore Ltd.
 * All Rights Reserved.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __riscv64
#define __riscv_xlen 64
#define uintptr_t uint64_t
#else
#define __riscv_xlen 32
#define uintptr_t uint32_t
#endif

#include "mrt.h"
#include "pmp.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define error(...)                                                             \
    do {                                                                       \
        fprintf(stderr, "memcheck: " __VA_ARGS__);                             \
        dump_memory_map();                                                     \
        exit(1);                                                               \
    } while (0)

const scr_mem_region_info mem_regions[] = {
#if defined(PLF_MEM_MAP)
    PLF_MEM_MAP
#endif // PLF_MEM_MAP
};

void dump_memory_map()
{
    fprintf(stderr, "\nMemory map dump (most prioritized region last):\n");
    fprintf(stderr, "  %-20s %-20s   %s\n", "NAME", "BASE", "SIZE");
    for (const scr_mem_region_info *p = mem_regions; p < mem_regions + ARRAY_SIZE(mem_regions); ++p) {
        fprintf(stderr, "  %-20s 0x%-20lx 0x%lx\n", p->name, (uint64_t)p->base,
                (uint64_t)p->size);
    }
    fprintf(stderr, "\n");
}

bool is_power_of_two(uintptr_t x) { return ((x & (x - 1)) == 0); }

void check_region(const scr_mem_region_info *region)
{
    if (region->size < 64) {
        error("memory region %s is too small (region must be at least 64 bytes)\n",
              region->name);
    }
    if (!is_power_of_two(region->size))
        error("size of memory region %s is not power of two\n", region->name);
    if (region->base % region->size)
        error("base address of memory region %s is not NAPOT\n", region->name);
}

void check_region_nestedness(const scr_mem_region_info *low, const scr_mem_region_info *high)
{
    // using a trick to avoid possible overflow
    uintptr_t low_max = low->size - 1 + low->base;
    uintptr_t high_max = high->size - 1 + high->base;
    if (high->base <= low->base && high_max >= low_max) {
        error("memory region %s is fully overlapped by larger region %s with higher priority\n",
              low->name, high->name);
    }
}

int main()
{
    const unsigned region_count = ARRAY_SIZE(mem_regions);
    for (const scr_mem_region_info *p = mem_regions; p < mem_regions + region_count; ++p)
        check_region(p);
    for (const scr_mem_region_info *low = mem_regions; low < mem_regions + region_count; ++low)
        for (const scr_mem_region_info *high = low + 1; high < mem_regions + region_count; ++high)
            check_region_nestedness(low, high);
    return 0;
}
