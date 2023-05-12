#ifndef _PAGING_H
#define _PAGING_H 

#include "x86_desc.h"

#define TABLE_4KB_PD_OFFSET 12
#define TABLE_4MB_PD_OFFSET 22

#define VIDEO_MEM_POS 0xB8000

#define USER_MEM_INDEX 32
#define VID_MEM_IND 33
#define LAST_VPAGE 0xBC
#define PAGE_GAP 0xB9
#define KB_MAP 0xB1

#define PROGRAM_INSERT 0x48000

void paging_init();

#endif /* PAGING.H */
/*
Paging is controlled by three flags in the processor’s control registers:
• PG (paging) flag. Bit 31 of CR0 (available in all IA-32 processors beginning with the
Intel386 processor).
• PSE (page size extensions) flag. Bit 4 of CR4 (introduced in the Pentium processor).
• PAE (physical address extension) flag. Bit 5 of CR4 (introduced in the Pentium Pro
processors).
PG to 1 when paging is enabled
PSE to 0 so we can set 4KB pages
PAE to 0 because we arent using address extension 

To select the various table entries, the linear address is divided into three sections:
• Page-directory entry—Bits 22 through 31 provide an offset to an entry in the page
directory. The selected entry provides the base physical address of a page table.
• Page-table entry—Bits 12 through 21 of the linear address provide an offset to an entry in
the selected page table. This entry provides the base physical address of a page in physical
memory.
• Page offset—Bits 0 through 11 provides an offset to a physical address in the page.
Memory management software has the option of using one page directory for all programs and
tasks, one page directory for each task, or some combination of the two.

*/
