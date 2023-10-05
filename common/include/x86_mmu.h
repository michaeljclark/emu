#pragma once

#include "types.h"

/*
 * PML4E    - Bits 51:12 are from CR3        47:39; offset[11:3] are va[47:39]
 * PDPTE    - Bits 51:12 are from the PML4E  47:30; offset[11:3] are va[38:30]
 * PDE      - Bits 51:12 are from the PDPTE  47:21; offset[11:3] are va[29:21]
 * PTE      - Bits 51:12 are from the PTE    47:12; offset[11:3] are va[11:3]
 */

#define PAGE_SIZE 4096
#define PAGE_MASK 4095
#define PAGE_SHIFT 12

#define PTE_P   (1 << 0) /* Present */
#define PTE_RW  (1 << 1) /* Read-write */
#define PTE_U   (1 << 2) /* User */
#define PTE_PWT (1 << 3) /* Page write-through */
#define PTE_PCD (1 << 4) /* Page cache disable */
#define PTE_A   (1 << 5) /* Accessed */
#define PTE_D   (1 << 6) /* Dirty */
#define PTE_SZ  (1 << 7) /* Page Size */
#define PTE_G   (1 << 8) /* Global */

typedef struct {
    union {
        struct {
            ullong present  : 1;   /* [0]     entry is present */
            ullong write    : 1;   /* [1]     writes are not allowed */
            ullong user     : 1;   /* [2]     user accesses are allowed */
            ullong pwt      : 1;   /* [3]     page-level write-through */
            ullong pcd      : 1;   /* [4]     page-level cache disable */
            ullong accessed : 1;   /* [5]     entry has been accessed */
            ullong dirty    : 1;   /* [6]     dirty (applies to leaf pages) */
            ullong pagesize : 1;   /* [7]     leaf (PAT for 4K pages) */
            ullong global   : 1;   /* [8]     global (applies to 4K PTE) */
            ullong ignored1 : 3;   /* [9-11]  ignored */
            ullong pfn      : 39;  /* [50:12] page frame number */
            ullong reserved : 1;   /* [51]    must be 0 */
            ullong ignored2 : 7;   /* [58:52] ignored */
            ullong pkey     : 4;   /* [62:59] protection key */
            ullong nx       : 1;   /* [63]    no execute */
        } fields;
        ullong value;
    };
} x86_pte;

#define GDT_TYPE_X       0x8 // Executable segment
#define GDT_TYPE_E       0x4 // Expand down (non-executable segments)
#define GDT_TYPE_C       0x4 // Conforming code segment (executable segments)
#define GDT_TYPE_W       0x2 // Writeable (non-executable segments)
#define GDT_TYPE_R       0x2 // Readable (executable segments)
#define GDT_TYPE_A       0x1 // Accessed

typedef struct {
    uint limit_15_0  : 16;
    uint addr_15_0   : 16;
    uint addr_23_16  : 8;
    uint type        : 4; /* 0:r, 2:rw, 8:x, 10:rx  */
    uint kind        : 1; /* 1 for code/data */
    uint dpl         : 2;
    uint present     : 1; /* 1 */
    uint limit_19_16 : 4;
    uint avail_sys   : 1;
    uint long_mode   : 1; /* 1 for 64-bit code segments */
    uint big_dfl     : 1; /* 1 for 32-bit segments */
    uint granularity : 1; /* 1 for 4K granularity */
    uint addr_31_24  : 8;
} x86_gdt_desc;

typedef struct {
    uint limit_15_0  : 16;
    uint addr_15_0   : 16;
    uint addr_23_16  : 8;
    uint type        : 4; /* 9 for tss segments */
    uint kind        : 1; /* 0 for system/tss */
    uint dpl         : 2;
    uint present     : 1; /* 1 */
    uint limit_19_16 : 4;
    uint avail_sys   : 1;
    uint reserved_1  : 1; /* 0 */
    uint reserved_2  : 1; /* 0 */
    uint granularity : 1;
    uint addr_31_24  : 8;
    uint addr_63_32  : 32;
    uint reserved_3  : 32;
} x86_gdt_tss_64_desc;

typedef struct {
    uint reserved_1[1];
    uint rsp0[2];
    uint rsp1[2];
    uint rsp2[2];
    uint reserved_2[2];
    uint ist1[2];
    uint ist2[2];
    uint ist3[2];
    uint ist4[2];
    uint ist5[2];
    uint ist6[2];
    uint ist7[2];
    uint reserved_3[2];
    ushort reserved_4;
    ushort iomapbase; /* set to limit to disable */
} x86_tss_64;

typedef struct {
    uint addr_15_0   : 16;
    uint seg_sel     : 16;
    uint ist         : 3;
    uint reserved_1  : 5;
    uint type        : 4;
    uint reserved_2  : 1;
    uint dpl         : 2;
    uint present     : 1;
    uint addr_31_16  : 16;
    uint addr_63_32  : 32;
    uint reserved_3  : 32;
} x86_64_idt_desc;

typedef struct {
    ushort limit;
    ushort addr[4];
} x86_64_gdt_reg;

typedef struct {
    ushort limit;
    ushort addr[4];
} x86_64_idt_reg;
