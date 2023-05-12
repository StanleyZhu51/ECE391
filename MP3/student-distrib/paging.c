#include "paging.h"

/*
void paging_init()
Input: void
Output: void
Side Effects: initializes paging
*/
void paging_init(){
    // inits page table
    uint32_t i;
    for (i = 0; i < NUM_ENTRY; i++){
        if (i >= (VIDEO_MEM_POS >> TABLE_4KB_PD_OFFSET) && i <= LAST_VPAGE){
            if(i == PAGE_GAP){
                page_table[i].present = 0;
                continue;
            }
            page_table[i].present = 1;
        } else {
            page_table[i].present = 0;
        }

        if(i == KB_MAP){
            page_table[i].present = 1;
        }

        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disable = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_attribute_table = 0; 
        page_table[i].global = 0;
        page_table[i].available = 0;
        if(i == (VIDEO_MEM_POS >> TABLE_4KB_PD_OFFSET)){
            page_table[i].address = (VIDEO_MEM_POS >> TABLE_4KB_PD_OFFSET);
        } else{
            page_table[i].address = i;
        }
    }

    // sets up vid page table    
    page_table[KB_MAP].address = VIDEO_MEM_POS >> TABLE_4KB_PD_OFFSET;

    for (i = 0; i < NUM_ENTRY; i++){
        vid_page_table[i].present = 0;
        vid_page_table[i].read_write = 1;
        vid_page_table[i].user_supervisor = 0;
        vid_page_table[i].write_through = 0;
        vid_page_table[i].cache_disable = 0;
        vid_page_table[i].accessed = 0;
        vid_page_table[i].dirty = 0;
        vid_page_table[i].page_attribute_table = 0; // 0 ?
        vid_page_table[i].global = 0;
        vid_page_table[i].available = 0;
        vid_page_table[i].address = i;
    }

    page_directory[0].p4KB.present = 1;
    page_directory[0].p4KB.read_write = 1;
    page_directory[0].p4KB.user_supervisor = 0;
    page_directory[0].p4KB.write_through = 0;
    page_directory[0].p4KB.cache_disable = 0;
    page_directory[0].p4KB.accessed = 0;
    page_directory[0].p4KB.global = 0;
    page_directory[0].p4KB.available_bit = 0; // reserved so set to 0
    page_directory[0].p4KB.page_size = 0; // 0 for 4 KiB
    page_directory[0].p4KB.available = 0; // available for system programmer
    page_directory[0].p4KB.address = ((unsigned int) page_table >> TABLE_4KB_PD_OFFSET);

    page_directory[1].p4MB.present = 1;
    page_directory[1].p4MB.read_write = 1;
    page_directory[1].p4MB.user_supervisor = 0;
    page_directory[1].p4MB.write_through = 0;
    page_directory[1].p4MB.cache_disable = 0;
    page_directory[1].p4MB.accessed = 0;
    page_directory[1].p4MB.dirty = 0; // reserved so set to 0
    page_directory[1].p4MB.page_size = 1; // 0 for 4 KiB
    page_directory[1].p4MB.global = 1;
    page_directory[1].p4MB.available = 0; // available for system programmer
    page_directory[1].p4MB.page_table_attribute_index = 0;
    page_directory[1].p4MB.reserved = 0;
    page_directory[1].p4MB.page_base_address = 1;

    for (i = 2; i < NUM_ENTRY; i++){
        page_directory[i].p4MB.present = 0;
        if(i == USER_MEM_INDEX){
            page_directory[i].p4MB.present = 1;
        }
        page_directory[i].p4MB.read_write = 1;
        page_directory[i].p4MB.user_supervisor = 1;
        page_directory[i].p4MB.write_through = 0;
        page_directory[i].p4MB.cache_disable = 0;
        page_directory[i].p4MB.accessed = 0;
        page_directory[i].p4MB.dirty = 0; // reserved so set to 0
        page_directory[i].p4MB.page_size = 1; // 0 for 4 KiB
        page_directory[i].p4MB.global = 0;
        page_directory[i].p4MB.available = 0; // available for system programmer
        page_directory[i].p4MB.page_table_attribute_index = 0;
        page_directory[i].p4MB.reserved = 0;
        page_directory[i].p4MB.page_base_address = i;
    }

    page_directory[VID_MEM_IND].p4KB.present = 1;
    page_directory[VID_MEM_IND].p4KB.read_write = 1;
    page_directory[VID_MEM_IND].p4KB.user_supervisor = 1;
    page_directory[VID_MEM_IND].p4KB.write_through = 0;
    page_directory[VID_MEM_IND].p4KB.cache_disable = 0;
    page_directory[VID_MEM_IND].p4KB.accessed = 0;
    page_directory[VID_MEM_IND].p4KB.global = 0;
    page_directory[VID_MEM_IND].p4KB.available_bit = 0; // reserved so set to 0
    page_directory[VID_MEM_IND].p4KB.page_size = 0; // 0 for 4 KiB
    page_directory[VID_MEM_IND].p4KB.available = 0; // available for system programmer
    page_directory[VID_MEM_IND].p4KB.address = ((unsigned int) vid_page_table >> TABLE_4KB_PD_OFFSET);
    /* 
        Merged load page directory and enable paging into one
        also set mixed 4kib and 4mib to on
    */
    asm (
        "movl $page_directory, %%eax ;"
        "andl $0xFFFFFC00, %%eax ;"
        "movl %%eax, %%cr3 ;"
        "movl %%cr4, %%eax ;"
        "orl $0x00000010, %%eax ;"
        "movl %%eax, %%cr4 ;"
        "movl %%cr0, %%eax ;"
        "orl $0x80000001, %%eax ;"
        "movl %%eax, %%cr0 ;"
        :
        :
        :"eax", "cc");
    /*
    courtesy of: 
    https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html amd 
    https://wiki.osdev.org/Setting_Up_Paging
    */
    return;
}
