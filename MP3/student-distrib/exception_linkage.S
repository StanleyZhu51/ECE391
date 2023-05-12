.globl divide_error_exception
divide_error_exception:
    pushal
    pushfl
    call divide_error_exception_handler
    popfl
    popal
    iret

.globl debug_exception
debug_exception:
    pushal
    pushfl
    call debug_exception_handler
    popfl
    popal
    iret

.globl nmi_interrupt
nmi_interrupt:
    pushal
    pushfl
    call nmi_interrupt_handler
    popfl
    popal
    iret

.globl breakpoint_exception
breakpoint_exception:
    pushal
    pushfl
    call breakpoint_exception_handler
    popfl
    popal
    iret

.globl overflow_exception
overflow_exception:
    pushal
    pushfl
    call overflow_exception_handler
    popfl
    popal
    iret

.globl bound_range_exceeded_exception
bound_range_exceeded_exception:
    pushal
    pushfl
    call bound_range_exceeded_exception_handler
    popfl
    popal
    iret

.globl invalid_opcode_exception
invalid_opcode_exception:
    pushal
    pushfl
    call invalid_opcode_exception_handler
    popfl
    popal
    iret

.globl device_not_available_exception
device_not_available_exception:
    pushal
    pushfl
    call device_not_available_exception_handler
    popfl
    popal
    iret

.globl double_fault_exception
double_fault_exception:
    addl $4, %esp
    pushal
    pushfl
    call double_fault_exception_handler
    popfl
    popal
    iret

.globl coprocessor_segment_overrun
coprocessor_segment_overrun:
    pushal
    pushfl
    call coprocessor_segment_overrun_handler
    popfl
    popal
    iret

.globl invalid_tss_exception
invalid_tss_exception:
    addl $4, %esp
    pushal
    pushfl
    call invalid_tss_exception_handler
    popfl
    popal
    iret

.globl segment_not_present
segment_not_present:
    addl $4, %esp
    pushal
    pushfl
    call segment_not_present_handler
    popfl
    popal
    iret

.globl stack_fault_exception
stack_fault_exception:
    addl $4, %esp
    pushal
    pushfl
    call stack_fault_exception_handler
    popfl
    popal
    iret

.globl general_protection_exception
general_protection_exception:
    addl $4, %esp
    pushal
    pushfl
    call general_protection_exception_handler
    popfl
    popal
    iret

.globl page_fault_exception
page_fault_exception:
    addl $4, %esp
    pushal
    pushfl
    call page_fault_exception_handler
    popfl
    popal
    iret

.globl x87_FPU_floating_point_error
x87_FPU_floating_point_error:
    pushal
    pushfl
    call x87_FPU_floating_point_error_handler
    popfl
    popal
    iret

.globl alignment_check_exception
alignment_check_exception:
    addl $4, %esp
    pushal
    pushfl
    call alignment_check_exception_handler
    popfl
    popal
    iret

.globl machine_check_exception
machine_check_exception:
    pushal
    pushfl
    call machine_check_exception_handler
    popfl
    popal
    iret

.globl SIMD_floating_point_exception
SIMD_floating_point_exception:
    pushal
    pushfl
    call SIMD_floating_point_exception_handler
    popfl
    popal
    iret

.globl reserved_exception
reserved_exception:
    pushal
    pushfl
    call reserved_exception_handler
    popfl
    popal
    iret
.end  
