#include "pit.h"

/*
void pit_init(void)
Input: void
Output: void
Side Effects: initializes pit
*/
void pit_init(void){
    scheduled_term = 0;
    timer_ticks = 0;
    seconds = 0;
    int divisor = DEF_DIVIDEND/PIT_HZ;       /* Calculate our divisor */
    outb(PIT_COMMAND, PIT_COMMAND_PORT);             /* Set our command byte 0x36 */
    outb(divisor & PIT_MASK, PIT_DATA_PORT);   /* Set low byte of divisor */
    outb(divisor >> RIGHT_SHIFT_8, PIT_DATA_PORT);     /* Set high byte of divisor */
    enable_irq(0);
}

/*
void pit_handler(void)
Input: void
Output: void
Side Effects: handles pit interrupt
*/
void pit_handler(void){
    // send eoi
    cli();
    send_eoi(0);
    sti();
    // schedule
    schedule();
    return;
}

/*
void schedule(void)
Input: void
Output: void
Side Effects: performs round robin scheduling between the 3 terminals
*/
void schedule(void){
    // grab next terminal number
    int32_t current_term = scheduled_term;
    scheduled_term = (scheduled_term+1) % 3;
    terminals[current_term].active = 0;
    terminals[scheduled_term].active = 1;
    
    // check next term is not the same as current
    if(scheduled_term == current_term){
        return;
    }

    // change paging for page table and vid map
    if(scheduled_term == terminal){
        page_table[VIRT_VID].address = VIRT_VID;
        vid_page_table[scheduled_term].address = VIRT_VID;
    }else{
        page_table[VIRT_VID].address = (VIDEO + FOUR_KB*(scheduled_term+VID_TERM_OFFSET)) >> TABLE_4KB_PD_OFFSET;
        vid_page_table[scheduled_term].address = (VIDEO + FOUR_KB*(scheduled_term+VID_TERM_OFFSET)) >> TABLE_4KB_PD_OFFSET;
    }

    // flush tlb
    asm volatile(
        "movl %%cr3, %%eax ;"
        "movl %%eax, %%cr3 ;"
        :
        :
        :"eax", "cc"
    );

    // boot new shell if terminal is not 'on'
    if(terminals[scheduled_term].on == 0){
        // check if processes are full
        uint32_t i;
        for(i = 0; i < FD_NUM; i++){
            if(pid_array[i] != 1){
                break;
            }
        }

        if(i == FD_NUM){
            printf("PROCESSES FULL \n");
            return;
        }

        // set up new terminal properties
        terminals[scheduled_term].base_pid = i;
        terminals[scheduled_term].on = 1;

        // save esp, ebp for task switch
        uint32_t temp_esp;
        uint32_t temp_ebp;

        asm volatile (
            "movl %%esp, %0"
            : "=r" (temp_esp)
            :
            :"memory", "cc"
        );

        asm volatile (
            "movl %%ebp, %0"
            : "=r" (temp_ebp)
            :
            :"memory", "cc"
        );

        // grab current pcb and set its values
        pcb_t* prev = (pcb_t*) get_curr_pcb(terminals[current_term].active_pid); 

        prev->ts_esp = temp_esp;
        prev->ts_ebp = temp_ebp;
        prev->tss_esp0 = tss.esp0;

        // launch shell for empty term
        execute((uint8_t*)"shell");
        return;
    }else{
        // get current and next pcbs
        pcb_t* next_pcb = (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid); 
        next_pcb->active = 1;

        pcb_t* prev = (pcb_t*) get_curr_pcb(terminals[current_term].active_pid); 
        prev->tss_esp0 = tss.esp0; 

        // task switch
        tss.ss0 = KERNEL_DS;
        tss.esp0 = next_pcb->tss_esp0;

        // remap paging for next process and flush tlb
        page_directory[USER_MEM_INDEX].p4MB.page_base_address = ((2 + next_pcb->process_id) * FOUR_MB) >> PAGING_4MB_SHIFT; 
        
        asm volatile(
            "movl %%cr3, %%eax ;"
            "movl %%eax, %%cr3 ;"
            :
            :
            :"eax", "cc"
        );

        // save and set esp, ebp
        asm volatile (
            "movl %%esp, %0"
            : "=r" (prev->ts_esp)
            :
            : "cc"
        );

        asm volatile (
            "movl %%ebp, %0"
            : "=r" (prev->ts_ebp)
            :
            :"cc"
        );

        asm volatile (
            "movl %0, %%esp ;"
            :
            : "r" (next_pcb->ts_esp)
            : "esp"
        );
        asm volatile (
            "movl %0, %%ebp ;"
            :
            : "r" (next_pcb->ts_ebp)
            : "ebp"
        );
        return;
    }
}
