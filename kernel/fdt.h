#ifndef __FDT_H__
#define __FDT_H__

#include "types.h"

/* FDT magic number */
#define FDT_MAGIC 0xd00dfeed

/* FDT token types */
#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

/* FDT header structure */
struct fdt_header {
    u32 magic;
    u32 totalsize;
    u32 off_dt_struct;
    u32 off_dt_strings;
    u32 off_mem_rsvmap;
    u32 version;
    u32 last_comp_version;
    u32 boot_cpuid_phys;
    u32 size_dt_strings;
    u32 size_dt_struct;
};

/* Convert big-endian to native (RISC-V is little-endian) */
static inline u32 fdt32_to_cpu(u32 x)
{
    return ((x & 0xff000000) >> 24) |
           ((x & 0x00ff0000) >> 8)  |
           ((x & 0x0000ff00) << 8)  |
           ((x & 0x000000ff) << 24);
}

/* Print the device tree in text format */
void fdt_print(void *fdt);

/* Get DTB pointer (set by entry.S) */
void *fdt_get_dtb(void);

#endif /* __FDT_H__ */
