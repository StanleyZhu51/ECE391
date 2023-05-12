#include "idt.h"

/*
idt_init
Input: none
Output: none
Side Effects: initializes the idt
*/
void idt_init(){
    int i;
    
    for(i = 0; i < NUM_VEC; i++){
        idt[i].present = 0;
        if(i >= 0x20){
            //idt[i].dpl = 3; // user space for system calls
            idt[i].reserved3 = 1;
        }
        idt[i].dpl = 0; // 0 for kernel space
        idt[i].size = 1; //All handlers 32 bit
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].seg_selector = KERNEL_CS; // appendix d is god
    }
    idt[INT_VECTOR].dpl = 3;
   
    
    //EXCEPTIONS
    idt[DE].present = 1;
    SET_IDT_ENTRY(idt[DE], &divide_error_exception);
    
    idt[DBE].present = 1;
    SET_IDT_ENTRY(idt[DBE], &debug_exception);

    idt[NMI].present = 1;
    SET_IDT_ENTRY(idt[NMI], &nmi_interrupt);

    idt[BPE].present = 1;
    SET_IDT_ENTRY(idt[BPE], &breakpoint_exception);

    idt[OFE].present = 1;
    SET_IDT_ENTRY(idt[OFE], &overflow_exception);

    idt[BRE].present = 1;
    SET_IDT_ENTRY(idt[BRE], &bound_range_exceeded_exception);

    idt[IVO].present = 1;
    SET_IDT_ENTRY(idt[IVO], &invalid_opcode_exception);

    idt[DNE].present = 1;
    SET_IDT_ENTRY(idt[DNE], &device_not_available_exception);

    idt[DF].present = 1;
    SET_IDT_ENTRY(idt[DF], &double_fault_exception);

    idt[CSO].present = 1;
    SET_IDT_ENTRY(idt[CSO], &coprocessor_segment_overrun);

    idt[IVT].present = 1;
    SET_IDT_ENTRY(idt[IVT], &invalid_tss_exception);

    idt[SNP].present = 1;
    SET_IDT_ENTRY(idt[SNP], &segment_not_present);

    idt[SFE].present = 1;
    SET_IDT_ENTRY(idt[SFE], &stack_fault_exception);

    idt[GPE].present = 1;
    SET_IDT_ENTRY(idt[GPE], &general_protection_exception);

    idt[PFE].present = 1;
    SET_IDT_ENTRY(idt[PFE], &page_fault_exception);

    //idt[0x0F].present = 1;
    //SET_IDT_ENTRY(idt[0x0F], &reserved_exception);

    idt[X87].present = 1;
    SET_IDT_ENTRY(idt[X87], &x87_FPU_floating_point_error);

    idt[ACE].present = 1;
    SET_IDT_ENTRY(idt[ACE], &alignment_check_exception);

    idt[MCE].present = 1;
    SET_IDT_ENTRY(idt[MCE], &machine_check_exception);

    idt[SIMD].present = 1;
    SET_IDT_ENTRY(idt[SIMD], &SIMD_floating_point_exception);
    // 0x14 to 0x1F taken by INTEL LOL

    //HARDWARE INTS
    idt[RTC_INTERRUPT_NUM].present = 1;
    SET_IDT_ENTRY(idt[RTC_INTERRUPT_NUM], rtc_handler_linkage);

    idt[KEYBOARD_INTERRUPT_NUM].present = 1;
    SET_IDT_ENTRY(idt[KEYBOARD_INTERRUPT_NUM], keyboard_handler_linkage);

    idt[PIT_INTERRUPT_NUM].present = 1;
    SET_IDT_ENTRY(idt[PIT_INTERRUPT_NUM], pit_handler_linkage);

    //SYSTEM CALLS
    idt[INT_VECTOR].present = 1;
    SET_IDT_ENTRY(idt[INT_VECTOR], &syscall_linker);
}
