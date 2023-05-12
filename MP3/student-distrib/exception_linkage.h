#ifndef _EXCEPTION_LINKAGE_H
#define _EXCEPTION_LINKAGE_H

extern void divide_error_exception(void);

extern void debug_exception(void);

extern void nmi_interrupt(void);

extern void breakpoint_exception(void);

extern void overflow_exception(void);

extern void bound_range_exceeded_exception(void);

extern void invalid_opcode_exception(void);

extern void device_not_available_exception(void);

extern void double_fault_exception(void);

extern void coprocessor_segment_overrun(void);

extern void invalid_tss_exception(void);

extern void segment_not_present(void);

extern void stack_fault_exception(void);

extern void general_protection_exception(void);

extern void page_fault_exception(void);

extern void x87_FPU_floating_point_error(void);

extern void alignment_check_exception(void);

extern void machine_check_exception(void);

extern void SIMD_floating_point_exception(void);

extern void reserved_exception(void);

#endif /* EXCEPTION_LINKAGE.H */
