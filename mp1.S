
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

#extern __mp1_copy_from_user
#extern mp1_malloc
#extern mp1_free

mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

mp1_rtc_tasklet:
        push %ebp            # save and update ebp
        movl %esp, %ebp

        push %ebx            # callee save
        push %esi
        push %edi
Loop:
        #CMPL $0, 
	ret

mp1_ioctl:
        push %ebp               # save and update ebp
        movl %esp, %ebp

        push %ebx               # callee save
        push %esi
        push %edi

        movl 8(%ebp), %ebx      # ebx <- arg
        movl 12(%ebp), %esi     # esi <- cmd

        cmpl $3, %esi           # check cmd is between 0-3
        ja error
        cmpl $0, %esi
        jb error
        jmp *jumptable(,%esi,4) # jump
error_free:
        push %ecx               # push address of malloc
        call mp1_free
error:
        movl $-1, %eax          # return value = -1
        pop %edi                # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl_add:

        xorl %edi, %edi         # edi = 16
        addl $16, %edi
        
        # ??CALLER SAVE??
        push %edi               # push 16
        call mp1_malloc         # allocate new memory
        testl %eax, %eax        # if eax == null (??is this 0??), return error
        jz error

        movl %eax, %ecx         # copy address of malloc to ecx
        push %eax               # push address of new malloc
        push %ebx               # push address of source
        call mp1_copy_from_user  # fill in the new memory
        testl %eax, %eax        # if result != 0, return error
        jnz error_free

        movw 4(%ebx), %ax       # ax = on length
        movw %ax, 8(%ebx)       # countdown = ax
        movw $1, 10(%ebx)       # status = 1
        movl %ecx, mp1_list_head # make struct the head

        cmpw $0, %bx              # check if location is between 0-1999
        jb error_free
        cmpw $1999, %bx
        ja error_free

        movb 2(%ebx), %cl       # cl = on_char
        movw (%ebx), %ax          # eax = location
        call mp1_poke
        leave
	ret
        
mp1_ioctl_remove:
	ret
        
mp1_ioctl_find:
	ret

mp1_ioctl_sync:
	ret


jumptable:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end
