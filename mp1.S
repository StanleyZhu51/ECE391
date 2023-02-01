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

        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi
                                        
        movl mp1_list_head, %esi        # esi = list head
        testl %esi, %esi
        jz tasklet_end                  # check if head == null
loop:      
        decw COUNTDOWN(%esi)            # decrement countdown
        movw COUNTDOWN(%esi), %di       # di = countdown
        testw %di, %di                  # if di != 0, jump loop
        jnz next_struct                        # else, continue to check status

# bx = status
check_status:
        xorl %ebx, %ebx                 # clear regs

        movw STATUS(%esi), %bx          # bx = status
        testw %bx, %bx                  # if status == 0, jump to_on
        jz to_on                        # else, jump to_off
        jmp to_off

# cl = on char
# ax = location
# di = on length
to_on:                                  # change pixel on
        xorl %eax, %eax                 # clear regs
        xor %cl, %cl
        xorl %edi, %edi
                                        # setup args for poke
        movb ON_CHAR(%esi), %cl         # cl = on char 
        movw LOCATION(%esi), %ax        # ax = location

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct
        
        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw ON_LENGTH(%esi), %di       # di = on length
        movw %di, COUNTDOWN(%esi)       # countdown = on length
        movw $1, STATUS(%esi)           # status = 1
        jmp next_struct                 # go to next struct

# cl = off char
# ax = location
# di = off length
to_off:                                 # change pixel on
        xorl %eax, %eax                 # clear regs
        xor %cl, %cl
        xorl %edi, %edi
                                        # setup args for poke
        movb OFF_CHAR(%esi), %cl        # cl = off char 
        movw LOCATION(%esi), %ax        # ax = location

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct

        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw OFF_LENGTH(%esi), %di      # di = off length
        movw %di, COUNTDOWN(%esi)       # countdown = off length
        movw $0, STATUS(%esi)           # status = 0
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
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi

        movl 4(%esp), %ebx              # ebx <- arg
        movl 8(%esp), %esi             # esi <- cmd

        cmpl $3, %esi                   # check cmd is between 0-3
        ja error
        cmpl $0, %esi
        jb error
        jmp *jumptable(,%esi,4)         # jump
error:
        movl $-1, %eax                  # return value = -1
	ret


# Register table
# ebx = arg
# edi = address of new struct
# ecx = temp reg
# eax = return values
mp1_ioctl_add:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi

        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz error_add
        
        push $STRUCT_SIZE               # push 16
        call mp1_malloc                 # allocate new memory
        testl %eax, %eax                # if eax == null, return error
        jz error_add

        movl %eax, %edi                 # copy address of malloc to edi

        push %ebx                       # push address of source
        push %eax                       # push address of new malloc
        call mp1_copy_from_user         # fill in the new memory
        subl $12, %esp                  # remove arguments
        testl %eax, %eax                # if result != 0, return error
        jnz error_free

        movw ON_LENGTH(%ebx), %ax       # ax = on length
        movw %ax, COUNTDOWN(%ebx)       # countdown = ax
        movw $1, STATUS(%ebx)           # status = 1

        movl mp1_list_head, %ecx        # insert struct at head
        movl %ecx, NEXT(%edi)           # struct->next = current head
        movl %edi, mp1_list_head        # head = struct

        cmpw $0, (%ebx)                 # check if location is between 0-1999
        jb error_free
        cmpw $1999, (%ebx)
        ja error_free

        xorl %eax, %eax                 # clear regs
        xor %cl, %cl

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
error_add:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        movl $-1, %eax                  # return value = -1
        leave
	ret
error_free:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        push %ecx                       # push address of malloc
        call mp1_free                      
        leave
        ret
        
mp1_ioctl_remove:
        ret


# ebx = arg
# eax = various ret values        
mp1_ioctl_find:
        ret # TEMPPP

        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi 

        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz err_find

        push LOCATION(%ebx)             # push args for search list
        push mp1_list_head
        call search_list
        subl $8, %esp                    # pop args

        testl %eax, %eax
        jz err_find

        push $STRUCT_SIZE               # push args for copy
        push %eax
        push %ebx
        call mp1_copy_to_user
        subl $12, %esp                   # pop args
        xorl %eax, %eax                 # return = 0
        jmp find_done
err_find:
        movl $-1, %eax
find_done: 
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl_sync:
        ret #TTEMPP
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi 

        movw %bx, %si                   # si = 2nd integer (low 16bits)
        shr $16, %ebx
        movl %bx, %di                   # di = 1st integer (high 16bits)


	pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret

# eax search_list(list ptr, arg) in c-style convention
# return matching ptr 
# returns null if failed
# no clobbers
search_list:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi  

        movl 8(%ebp), %ebx              # ebx = arg
        movl 16(%ebp), %edi             # edi = list ptr

loop_search:
        movw LOCATION(%edi), %esi         # ecx = cur struct arg
        cmpl %esi, %ebx                  # check if arg found
        je found

        movl NEXT(%edi), %edi           # edi = next->struct
        testl %edi, %edi                # if null ptr,    
        jz err_search
        jmp loop_search
found:                                  # set return value to matching ptr
        movl %edi, %eax
        jmp end_search
err_search:                             # set return value to null
        xorl %eax, %eax
end_search:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret



jumptable:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end
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

        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi
                                        
        movl mp1_list_head, %esi        # esi = list head
        testl %esi, %esi
        jz tasklet_end                  # check if head == null
loop:      
        decw COUNTDOWN(%esi)            # decrement countdown
        movw COUNTDOWN(%esi), %di       # di = countdown
        testw %di, %di                  # if di != 0, jump loop
        jnz next_struct                        # else, continue to check status

# bx = status
check_status:
        xorl %ebx, %ebx                 # clear regs

        movw STATUS(%esi), %bx          # bx = status
        testw %bx, %bx                  # if status == 0, jump to_on
        jz to_on                        # else, jump to_off
        jmp to_off

# cl = on char
# ax = location
# di = on length
to_on:                                  # change pixel on
        xorl %eax, %eax                 # clear regs
        xor %cl, %cl
        xorl %edi, %edi
                                        # setup args for poke
        movb ON_CHAR(%esi), %cl         # cl = on char 
        movw LOCATION(%esi), %ax        # ax = location

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct
        
        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw ON_LENGTH(%esi), %di       # di = on length
        movw %di, COUNTDOWN(%esi)       # countdown = on length
        movw $1, STATUS(%esi)           # status = 1
        jmp next_struct                 # go to next struct

# cl = off char
# ax = location
# di = off length
to_off:                                 # change pixel on
        xorl %eax, %eax                 # clear regs
        xor %cl, %cl
        xorl %edi, %edi
                                        # setup args for poke
        movb OFF_CHAR(%esi), %cl        # cl = off char 
        movw LOCATION(%esi), %ax        # ax = location

        cmpw $0, %ax                    # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct

        addw %ax, %ax                   # multiply location by 2
        call mp1_poke

        movw OFF_LENGTH(%esi), %di      # di = off length
        movw %di, COUNTDOWN(%esi)       # countdown = off length
        movw $0, STATUS(%esi)           # status = 0
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
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi

        movl 4(%esp), %ebx              # ebx <- arg
        movl 8(%esp), %esi             # esi <- cmd

        cmpl $3, %esi                   # check cmd is between 0-3
        ja error
        cmpl $0, %esi
        jb error
        jmp *jumptable(,%esi,4)         # jump
error:
        movl $-1, %eax                  # return value = -1
	ret


# Register table
# ebx = arg
# edi = address of new struct
# ecx = temp reg
# eax = return values
mp1_ioctl_add:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi

        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz error_add
        
        push $STRUCT_SIZE               # push 16
        call mp1_malloc                 # allocate new memory
        testl %eax, %eax                # if eax == null, return error
        jz error_add

        movl %eax, %edi                 # copy address of malloc to edi

        push %ebx                       # push address of source
        push %eax                       # push address of new malloc
        call mp1_copy_from_user         # fill in the new memory
        subl $12, %esp                  # remove arguments
        testl %eax, %eax                # if result != 0, return error
        jnz error_free

        movw ON_LENGTH(%ebx), %ax       # ax = on length
        movw %ax, COUNTDOWN(%ebx)       # countdown = ax
        movw $1, STATUS(%ebx)           # status = 1

        movl mp1_list_head, %ecx        # insert struct at head
        movl %ecx, NEXT(%edi)           # struct->next = current head
        movl %edi, mp1_list_head        # head = struct

        cmpw $0, (%ebx)                 # check if location is between 0-1999
        jb error_free
        cmpw $1999, (%ebx)
        ja error_free

        xorl %eax, %eax                 # clear regs
        xor %cl, %cl

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
error_add:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        movl $-1, %eax                  # return value = -1
        leave
	ret
error_free:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        push %ecx                       # push address of malloc
        call mp1_free                      
        leave
        ret
        
mp1_ioctl_remove:
        ret


# ebx = arg
# eax = various ret values        
mp1_ioctl_find:
        ret # TEMPPP

        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi 

        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz err_find

        push LOCATION(%ebx)             # push args for search list
        push mp1_list_head
        call search_list
        subl $8, %esp                    # pop args

        testl %eax, %eax
        jz err_find

        push $STRUCT_SIZE               # push args for copy
        push %eax
        push %ebx
        call mp1_copy_to_user
        subl $12, %esp                   # pop args
        xorl %eax, %eax                 # return = 0
        jmp find_done
err_find:
        movl $-1, %eax
find_done: 
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl_sync:
        ret #TTEMPP
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi 

        movw %bx, %si                   # si = 2nd integer (low 16bits)
        shr $16, %ebx
        movl %bx, %di                   # di = 1st integer (high 16bits)


	pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret

# eax search_list(list ptr, arg) in c-style convention
# return matching ptr 
# returns null if failed
# no clobbers
search_list:
        push %ebp                       # save and update ebp
        movl %esp, %ebp

        push %ebx                       # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                 # clear regs
        xorl %esi, %esi
        xorl %edi, %edi  

        movl 8(%ebp), %ebx              # ebx = arg
        movl 16(%ebp), %edi             # edi = list ptr

loop_search:
        movw LOCATION(%edi), %esi         # ecx = cur struct arg
        cmpl %esi, %ebx                  # check if arg found
        je found

        movl NEXT(%edi), %edi           # edi = next->struct
        testl %edi, %edi                # if null ptr,    
        jz err_search
        jmp loop_search
found:                                  # set return value to matching ptr
        movl %edi, %eax
        jmp end_search
err_search:                             # set return value to null
        xorl %eax, %eax
end_search:
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret



jumptable:
        .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


.end
