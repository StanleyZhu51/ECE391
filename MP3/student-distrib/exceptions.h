#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#define DE 0x0
#define DBE 0x1
#define NMI 0x2
#define BPE 0x3
#define OFE 0x4
#define BRE 0x5
#define IVO 0x6
#define DNE 0x7
#define DF 0x8
#define CSO 0x9
#define IVT 0xA
#define SNP 0xB
#define SFE 0xC
#define GPE 0xD
#define PFE 0xE
#define X87 0x10
#define ACE 0x11
#define MCE 0x12
#define SIMD 0x13

void divide_error_exception_handler(void);

void debug_exception_handler(void);

void nmi_interrupt_handler(void);

void breakpoint_exception_handler(void);

void overflow_exception_handler(void);

void bound_range_exceeded_exception_handler(void);

void invalid_opcode_exception_handler(void);

void device_not_available_exception_handler(void);

void double_fault_exception_handler(void);

void coprocessor_segment_overrun_handler(void);

void invalid_tss_exception_handler(void);

void segment_not_present_handler(void);

void stack_fault_exception_handler(void);

void general_protection_exception_handler(void);

void page_fault_exception_handler(void);

void x87_FPU_floating_point_error_handler(void);

void alignment_check_exception_handler(void);

void machine_check_exception_handler(void);

void SIMD_floating_point_exception_handler(void);

void reserved_exception_handler(void);

#endif /* EXCEPTIONS.H*/
