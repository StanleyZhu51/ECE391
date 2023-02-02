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

# void mp1_rtc_tasklet()
# Description: Loops through the linked list to check and update (poke) each item inside based on status and countdown
# Interface: C-style
#    Inputs: Uses mp1_list_head (global var)
#   Outputs: Void
# Registers: 
# esi = cur struct
# di = countdown / on_length
# ax = location
# bx = status
mp1_rtc_tasklet:
        push %ebp                               # save and update ebp
        movl %esp, %ebp
        push %ebx                               # callee save
        push %esi
        push %edi
        xorl %ebx, %ebx                         # clear regs
        xorl %esi, %esi
        xorl %edi, %edi
                                        
        movl mp1_list_head, %esi                # esi = list head
        testl %esi, %esi
        jz tasklet_end                          # check if head == null
loop:      
        decw COUNTDOWN(%esi)                    # decrement countdown
        movw COUNTDOWN(%esi), %di              
        testw %di, %di                          # if countdown != 0, jump next_struct
        jnz next_struct                         # else, continue to check status
check_status:
        xorl %ebx, %ebx                         # clear regs
        movw STATUS(%esi), %bx         
        testw %bx, %bx                          # if status == 0, jump to_on
        jz to_on                                # else, jump to_off
        jmp to_off
to_on:                                          # change pixel on
        xorl %eax, %eax                         # clear regs
        xorb %cl, %cl
        xorl %edi, %edi
                                        
        movb ON_CHAR(%esi), %cl                 # setup args for poke
        movw LOCATION(%esi), %ax       

        cmpw $0, %ax                            # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct
        
        addw %ax, %ax                           # multiply location by 2
        call mp1_poke

        movw ON_LENGTH(%esi), %di              
        movw %di, COUNTDOWN(%esi)               # countdown = on_length
        movw $1, STATUS(%esi)                   # status = 1
        jmp next_struct                         # go to next struct
to_off:                                         # change pixel off
        xorl %eax, %eax                         # clear regs
        xorb %cl, %cl
        xorl %edi, %edi
                                        
        movb OFF_CHAR(%esi), %cl                # setup args for poke
        movw LOCATION(%esi), %ax        

        cmpw $0, %ax                            # check if location is between 0-1999
        jb next_struct
        cmpw $1999, %ax
        ja next_struct

        addw %ax, %ax                           # multiply location by 2
        call mp1_poke

        movw OFF_LENGTH(%esi), %di              
        movw %di, COUNTDOWN(%esi)               # countdown = off_length
        movw $0, STATUS(%esi)                   # status = 0
        jmp next_struct                         # go to next struct
next_struct:
        movl NEXT(%esi), %esi                   # cur struct = next struct
        testl %esi, %esi                        # if esi != 0, jump loop
        jnz loop                                # else, end function
tasklet_end:        
        pop %edi                                # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl:
        xorl %ebx, %ebx                         # clear regs
        xorl %esi, %esi
        xorl %edi, %edi

        movl 4(%esp), %ebx                      # ebx <- arg
        movl 8(%esp), %esi                      # esi <- cmd

        cmpl $3, %esi                           # check cmd is between 0-3
        ja error
        cmpl $0, %esi
        jb error
        jmp *jumptable(,%esi,4)                 # jump
error:
        movl $-1, %eax                          # return value = -1
	ret

# int mp1_ioctl_add(struct * arg)
# Description: adds a user level ptr to a struct to the linked list
# Interface: C-style
#    Inputs: arg (ptr to struct that should be added)
#   Outputs: 0 if success, -1 if fail
# Registers: 
# ebx = arg
# edi = address of new struct
# ecx = temp reg
# eax = various return values
mp1_ioctl_add:
        push %ebp                               # save and update ebp
        movl %esp, %ebp
        push %ebx                               # callee save
        push %esi
        push %edi

        testl %ebx, %ebx                        # if ebx (arg) == null, return error
        jz error_add
        
        push $STRUCT_SIZE                       # push 16
        call mp1_malloc                         # allocate new memory
        testl %eax, %eax                        # if eax == null, return error
        jz error_add

        movl %eax, %edi                         # copy address of malloc to edi

        push %ebx                               # push args
        push %eax                      
        call mp1_copy_from_user                 # copy from user
        add $12, %esp                           # remove args
        testl %eax, %eax                        # if result != 0, return error
        jnz error_free

        movw ON_LENGTH(%edi), %ax     
        movw %ax, COUNTDOWN(%edi)               # countdown = on_length
        movw $1, STATUS(%edi)                   # status = 1

        movl mp1_list_head, %ecx                # insert struct at head
        movl %edi, mp1_list_head  
        movl %ecx, NEXT(%edi)           
              
        cmpw $0, LOCATION(%ebx)                 # check if location is between 0-1999
        jb error_free
        cmpw $1999, LOCATION(%ebx)
        ja error_free

        xorl %eax, %eax                         # clear regs
        xorb %cl, %cl

        movb ON_CHAR(%ebx), %cl                 # set up args for poke
        movw (%ebx), %ax                        
        addw %ax, %ax                           # multiply location by 2
        call mp1_poke

        xorl %eax, %eax                         # return = 0
        pop %edi                                # callee restore
        pop %esi
        pop %ebx
        leave
	ret
error_add:
        pop %edi                                # callee restore
        pop %esi
        pop %ebx
        movl $-1, %eax                          # return value = -1
        leave
	ret
error_free:
        pop %edi                                # callee restore
        pop %esi
        pop %ebx
        push %ecx                               # push address of malloc
        call mp1_free
        addl $4, %esp                      
        leave
        ret


# int mp1_ioctl_remove(struct * arg)
# Description: removes a struct from the linked list
# Interface: C-style
#    Inputs: arg (ptr to struct that should be added)
#   Outputs: 0 if success, -1 if fail
# Registers: 
# ebx = arg
# edi = address of new struct
# ecx = temp reg
# eax = various return values
mp1_ioctl_remove:
        ret
        push %ebp                               # save and update ebp
        movl %esp, %ebp
        push %ebx                               # callee save
        push %esi
        push %edi

        push %ebx                       # push args, search, pop args
        call search_list
        addl $4, %esp
        testl %eax, %eax                # if search returns null, error
        jz remove_err
        testl %ecx, %ecx                # if search returns null, error
        jz remove_err

        cmpl %eax, mp1_list_head        # check if need to remove head
        je remove_head

        movl NEXT(%eax), %edi                 # edi = next struct
        movl %edi, NEXT(%ecx)

        push %eax                       # push address struct to remove
        call mp1_free
        addl $4, %esp 

        xorl %eax, %eax                 # return = 0
        jmp remove_done

remove_head:
        movl NEXT(%eax), %edi                 # edi = next struct
        movl %edi, mp1_list_head                      # push address struct to remove
        push %eax                       # push address struct to remove
        call mp1_free
        addl $4, %esp 

        xorl %eax, %eax                 # return = 0
        jmp remove_done

remove_err:
        movl $-1, %eax
remove_done:        
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret

# ebx = arg
# eax = various ret values        
mp1_ioctl_find:
        ret
        push %ebp                       # save and update ebp
        movl %esp, %ebp
        push %ebx                       # callee save
        push %esi
        push %edi
        
        testl %ebx, %ebx                # if ebx (arg) == null, return error
        jz find_err
        
        push $STRUCT_SIZE
        call mp1_malloc
        addl $4, %esp
        cmpl $0, %eax
        je find_err

        movl %eax, %edi
        
        push $STRUCT_SIZE
        push %ebx
        push %eax
        call mp1_copy_from_user
        addl $12, %esp
        cmpl $0, %eax
        jne find_err_malloc

        cmpw $0, LOCATION(%edi)                 # check if location is between 0-1999
        jb find_err_malloc
        cmpw $1999, LOCATION(%edi)
        ja find_err_malloc

        movw LOCATION(%edi), %si
        push %esi
        call search_list
        addl $4, %esp
        cmpl $0, %eax
        je find_err_malloc

        push $STRUCT_SIZE
        push %eax
        push %ebx
        call mp1_copy_to_user
        add $12, %esp
        cmpl $0, %eax
        jne find_err_malloc
        xorl %eax, %eax
        jmp find_deall

find_deall:
        push %edi
        call mp1_free
        addl $4, %esp
        xorl %eax, %eax
        jmp find_done

find_err_malloc:
        push %edi
        call mp1_free
        addl $4, %esp
        movl $-1, %eax
        jmp find_done
find_err:
        movl $-1, %eax    
find_done: 
        pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
	ret

mp1_ioctl_sync:
        ret
        push %ebp                       # save and update ebp
        movl %esp, %ebp
        push %ebx                       # callee save
        push %esi
        push %edi

        movw %bx, %si                   # si = 2nd integer (low 16bits)
        shrl $16, %ebx
        movw %bx, %di                   # di = 1st integer (high 16bits)

        push %ecx                       # caller save ecx
        push %edi                        # push args for search list
        call search_list
        addl $4, %esp                    # pop args
        pop %ecx                        # caller restore ecx
        testl %eax, %eax                # if search returns null, error
        jz sync_err
        movl %eax, %edi                 # edi = address of struct matching integer 1

        push %ecx                       # caller save ecx
        push %esi                        # push args for search list
        call search_list
        addl $4, %esp                    # pop args
        pop %ecx                        # caller restore ecx
        testl %eax, %eax                # if search returns null, error
        jz sync_err
        movl %eax, %esi                 # edi = address of struct matching integer 2

        movw ON_LENGTH(%edi), %bx         # assign 1's on_length to 2
        movw %bx, ON_LENGTH(%esi)
        movw OFF_LENGTH(%edi), %bx         # assign 1's off_length to 2
        movw %bx, OFF_LENGTH(%esi)
        movw COUNTDOWN(%edi), %bx         # assign 1's countdown to 2
        movw %bx, COUNTDOWN(%esi)
        movw STATUS(%edi), %bx         # assign 1's status to 2
        movw %bx, STATUS(%esi)
        testw %bx, %bx                  # if status == 0, 
        jz stat_zero
stat_zero:
        xorl %eax, %eax                 # clear regs
        xorb %cl, %cl

        movb OFF_CHAR(%esi), %cl                # push args for poke
        movw LOCATION(%esi), %ax

        cmpw $0, LOCATION(%esi)                 # check if location is between 0-1999
        jb sync_err
        cmpw $1999, LOCATION(%esi)
        ja sync_err

        addw %ax, %ax                           
        call mp1_poke
        movw $0, STATUS(%esi)
        xorl %eax, %eax                         # return = 0
        jmp sync_done
stat_one:
        xorl %eax, %eax                 # clear regs
        xorb %cl, %cl

        movb ON_CHAR(%esi), %cl
        movw LOCATION(%esi), %ax

        cmpw $0, LOCATION(%esi)                 # check if location is between 0-1999
        jb sync_err
        cmpw $1999, LOCATION(%esi)
        ja sync_err

        addw %ax, %ax
        call mp1_poke
        movw $1, STATUS(%esi)
        xorl %eax, %eax                         # return = 0
        jmp sync_done
sync_err:                               # return = -1
        movl $-1, %eax
sync_done:
	pop %edi                        # callee restore
        pop %esi
        pop %ebx
        leave
        ret

# ret_ptr search_list(arg) in c-style convention
# return: eax = target ptr
#        ecx = prev ptr
#       returns null if failed
# clobbers ecx
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
        movl mp1_list_head, %edi             # edi = list ptr
        cmpl $0, mp1_list_head          # check if head is null
        je err_search

loop_search:
        movw LOCATION(%edi), %si         # 
        cmpw %si, %bx                  # check if arg found
        je found

        movl %edi, %ecx                 # hold prev pointer in ecx
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
