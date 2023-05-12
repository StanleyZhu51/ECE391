#include "exceptions.h"
#include "exception_linkage.h"
#include "lib.h"

/*
FOR ALL FUNCTIONS:
exception_handler
Input: none
Output: none
Side Effects: prints the exception that is generated\n
*/

void divide_error_exception_handler(void){
    printf("DIVIDE_ERROR_EXCEPTION %x GENERATED\n", DE);
    halt(-1);
}

void debug_exception_handler(void){
    printf("DEBUG_EXCEPTION %x GENERATED\n", DBE);
    halt(-1);
}

void nmi_interrupt_handler(void){
    printf("NMI_INTERRUPT %x GENERATED\n", NMI);
    halt(-1);
}

void breakpoint_exception_handler(void){
    printf("BREAKPOINT_EXCEPTION %x GENERATED\n", BPE);
    halt(-1);
}

void overflow_exception_handler(void){
    printf("OVERFLOW_EXCEPTION %x GENERATED\n", OFE);
    halt(-1);
}

void bound_range_exceeded_exception_handler(void){
    printf("BOUND_RANGE_EXCEEDED %x GENERATED\n", BRE);
    halt(-1);
}

void invalid_opcode_exception_handler(void){
    printf("INVALID_OPCODE_EXCEPTION %x GENERATED\n", IVO);
    halt(-1);
}

void device_not_available_exception_handler(void){
    printf("DEVICE_NOT_AVAILABLE_EXCEPTION %x GENERATED\n", DNE);
    halt(-1);
}

void double_fault_exception_handler(void){
    printf("DOUBLE_FAULT_EXCEPTION %x GENERATED\n", DF);
    halt(-1);
}

void coprocessor_segment_overrun_handler(void){
    printf("COPROCESSOR_SEGMENT_OVERRUN %x GENERATED\n", CSO);
    halt(-1);
}

void invalid_tss_exception_handler(void){
    printf("INVALID_TSS_EXCEPTION %x GENERATED\n", IVT);
    halt(-1);
}

void segment_not_present_handler(void){
    printf("SEGMENT_NOT_PRESENT %x GENERATED\n", SNP);
    halt(-1);
}

void stack_fault_exception_handler(void){
    printf("STACK_FAULT_EXCEPTION %x GENERATED\n", SFE);
    halt(-1);
}

void general_protection_exception_handler(void){
    printf("GENERAL_PROTECTION_EXCEPTION %x GENERATED\n", GPE);
    halt(-1);
}

void page_fault_exception_handler(void){
    uint32_t val;
    asm volatile ("             \n\
            movl %%cr2, %0       \n\
            "
            : "=a"(val)
            : 
            : "memory"
    );
    printf("addr: %x\n", val);
    printf("PAGE_FAULT_EXCEPTION %x GENERATED\n", PFE);
    halt(-1);
}

void x87_FPU_floating_point_error_handler(void){
    printf("x87_FPU_FLOATING_POINT_ERROR %x GENERATED\n", X87);
    halt(-1);
}

void alignment_check_exception_handler(void){
    printf("ALIGNMENT_CHECK_EXCEPTION %x GENERATED\n", ACE);
    halt(-1);
}

void machine_check_exception_handler(void){
    printf("MACHINE_CHECK_EXCEPTION %x GENERATED\n", MCE);
    halt(-1);
}

void SIMD_floating_point_exception_handler(void){
    printf("SIMD_FLOATING_POINT_EXCEPTION %x GENERATED\n", SIMD);
    halt(-1);
}

void reserved_exception_handler(void){
    printf("reserved_exception %x GENERATED\n", 0x0F);
    halt(-1);
}
