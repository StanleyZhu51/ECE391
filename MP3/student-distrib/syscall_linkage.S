.global syscall_linker
.global syscall_table
.global halt_ret
.global exec_ret
.global term_switch

syscall_linker:
    #pushfl

    #pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    cmpl $1, %eax
    jl bad_syscall
    cmpl $10, %eax
    jg bad_syscall

    decl %eax

    call *syscall_table (, %eax, 4)

    jmp done

bad_syscall:
    xorl %eax, %eax
    movl $-1, %eax
    jmp done
    
done:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    #popl %esp

    #popfl

    iret

halt_ret:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %esp
    movl 16(%ebp), %eax
    movl 12(%ebp), %ebp
    sti
    leave
    ret

term_switch:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %esp
    movl 16(%ebp), %eax
    movl 12(%ebp), %edx
    sti
    leave
    
    movl %edx, %ebp
    
    ret

    
    
syscall_table:
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn


