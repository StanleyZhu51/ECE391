
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


mp1_poke:
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret


# esi = cur struct
# di = countdown
mp1_rtc_tasklet:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        
                                        
        movl mp1_list_head, %esi        # esi = list head
        testl %esi, %esi
        jz tasklet_end                  # check if head == null??
loop:      
        decw COUNTDOWN(%esi)            # decrement countdown
        movw COUNTDOWN(%esi), %di       # di = countdown
        testw %di, %di                  # if di == 0, jump loop
        jnz loop                        # else, continue to check status

# bx = status
check_status:
        movw STATUS(%esi), %bx          # bx = status
        testw %bx, %bx                  # if status == 0, jump to_on
        jz to_on                        # else, jump to_off
        jmp to_off

# cl = on char
# ax = location
# di = on length
to_on:                                  # change pixel on
                                        # setup args for poke
        movb ON_CHAR(%esi), %cl         # cl = on char 
        movw LOCATION(%esi), %ax        # ax = loaction

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct
        
        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw ON_LENGTH(%esi), %di       # di = on length
        movw %di, COUNTDOWN(%esi)       # countdown = on length
        jmp next_struct                 # go to next struct

# cl = off char
# ax = location
# di = off length
to_off:                                 # change pixel on
                                        # setup args for poke
        movb OFF_CHAR(%esi), %cl        # cl = off char 
        movw LOCATION(%esi), %ax        # ax = loaction

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct

        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw OFF_LENGTH(%esi), %di      # di = off length
        movw %di, COUNTDOWN(%esi)       # countdown = off length
        jmp next_struct                # go to next struct

next_struct:
        movl NEXT(%esi), %esi           # esi (cur struct) = next struct
        testl %esi, %esi                # if esi != 0, jump loop
        jnz loop                        # else, end function

tasklet_end:        
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi

        movl 8(%ebp), %ebx              # ebx <- arg
        movl 12(%ebp), %esi             # esi <- cmd

        cmpl $3, %esi                   # check cmd is between 0-3
        ja error
        cmpl $0, %esi
        jb error
        jmp *jumptable(,%esi,4)         # jump
error_free:
        push %ecx                       # push address of malloc
        call mp1_free
error:
        movl $-1, %eax                  # return value = -1
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
	ret


# Register table
# ebx = arg
# edi = address of new struct
# ecx = temp reg
# eax = return values
mp1_ioctl_add:
        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz error
        
        push $STRUCT_SIZE               # push 16
        call mp1_malloc                 # allocate new memory
        testl %eax, %eax                # if eax == null, return error
        jz error

        movl %eax, %edi                 # copy address of malloc to ecx

        push %ebx                       # push address of source
        push %eax                       # push address of new malloc
        call mp1_copy_from_user         # fill in the new memory

        testl %eax, %eax                # if result != 0, return error
        jnz error_free

        movw ON_LENGTH(%ebx), %ax       # ax = on length
        movw %ax, COUNTDOWN(%ebx)       # countdown = ax
        movw $1, STATUS(%ebx)           # status = 1

        movl mp1_list_head, %ecx
        movl %ecx, NEXT(%edi)           # struct->next = current head
        movl %edi, mp1_list_head        # head = struct

        cmpw $0, (%ebx)                 # check if location is between 0-1999
        jb error_free
        cmpw $1999, (%ebx)
        ja error_free

        movb ON_CHAR(%ebx), %cl         # cl = on_char
        movw (%ebx), %ax                # ax = location
        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        xorl %eax, %eax                 # return = 0
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
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
