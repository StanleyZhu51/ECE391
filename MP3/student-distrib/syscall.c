#include "syscall.h"

// defines all fops
file_operations_t dir_fop    = {dir_open, dir_close, dir_read, dir_write};
file_operations_t file_fop   = {file_open, file_close, file_read, file_write};
file_operations_t rtc_fop    = {rtc_open, rtc_close, rtc_read,rtc_write};
file_operations_t stdin_fop  = {terminal_open, terminal_close, terminal_read, null_write};
file_operations_t stdout_fop = {terminal_open, terminal_close, null_read, terminal_write};
file_operations_t null_fop   = {null_open, null_close, null_read, null_write};

/*
int32_t halt(uint8_t status)
Input: status
Output: int32_t
Side Effects: halts current process, restores parent process
*/
int32_t halt(uint8_t status){
    cli();
    // grab current pcb close its parameters
    
    pcb_t* current_pcb = (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);
    current_pcb->active = 0;

    int i;
    for(i = 0; i < FD_NUM; i++){
        (void) close(i);
    }

    // check if base shell
    if(terminals[scheduled_term].active_pid == terminals[scheduled_term].base_pid){
        pid_array[terminals[scheduled_term].active_pid] = 0;
        curr_pid = terminals[scheduled_term].base_pid;
        glob_parent_pid = terminals[scheduled_term].base_pid;
        sti();
        execute((uint8_t*)"shell");
    }
    
    pid_array[terminals[scheduled_term].active_pid] = 0;

    // grab parent pcb and restore it as current
    pcb_t* parent = (pcb_t*) get_curr_pcb(current_pcb->parent_id);
    parent->active = 1;

    terminals[scheduled_term].active_pid = parent->process_id;
    glob_parent_pid = parent->parent_id;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = parent->tss_esp0;
    
    // switch paging to current process
    page_directory[USER_MEM_INDEX].p4MB.page_base_address = ((2 + parent->process_id) * FOUR_MB) >> PAGING_4MB_SHIFT; 

    // Flushing tlb
    asm volatile(
        "movl %%cr3, %%eax ;"
        "movl %%eax, %%cr3 ;"
        :
        :
        :"eax", "cc"
    );
    terminals[scheduled_term].active_pid = parent->process_id;

    // returns from halt
    sti();
    halt_ret(parent->saved_esp, parent->saved_ebp, status);
    
    return 0;
}

/*
int32_t execute(const uint8_t* command)
Input: command
Output: int32_t
Side Effects: launches process given by command arg
*/
int32_t execute(const uint8_t* command){
    cli();
    // parse command
    uint32_t cmd_index;
    uint32_t currbuff_idx = 0;

    // clear cmd array
    uint8_t cmd[FILENAME_SIZE];
    uint32_t cmd_length = strlen((int8_t*)command);
    
    uint8_t args[FILENAME_SIZE];


    int i;
    for(i = 0; i < FILENAME_SIZE; i++){
        cmd[i] = '\0';
        args[i] = '\0';
    }
    
    // loop through command to parse, adds to cmd
    for(cmd_index = 0; cmd_index < cmd_length; cmd_index++){
        if(command[cmd_index] != ' ' && command[cmd_index] != '\n'){
            cmd[currbuff_idx] = command[cmd_index];
            currbuff_idx++; 
        }else{
            while(command[cmd_index] == ' ' || command[cmd_index] == '\n'){
                cmd_index++;
            }
            break;
            if(command[cmd_index] == ' '){
                for(i = currbuff_idx; i < FILENAME_SIZE; i++){
                    cmd[i] = '\0';
                }
                break;
            }
        }
    }

    // loop through command to parse, adds to args
    uint32_t arg_index;

    currbuff_idx = 0;
    for(arg_index = cmd_index; arg_index < cmd_length; arg_index++){
        if(command[arg_index] != ' ' && command[arg_index] != '\n'){
            args[currbuff_idx] = command[arg_index];
            currbuff_idx++; 
        }else{
            break;
            int zba;
            for(zba = cmd_index; zba > cmd_length; zba++){
                cmd[zba] = '\0';
            }
            break;
        }
    }

    // check if file exists:
    dentry_t exec_file;
    if(read_dentry_by_name(cmd, &exec_file) == -1){
        sti();
        return -1;
    }

    //check if first four bytes are the magic numbers
    uint8_t magic_nums[ELFS];
    if(read_data(exec_file.inode_number, (uint32_t)0 , magic_nums, ELFS) == -1){
        sti();
        return -1;
    }

    if(magic_nums[M0] != MAGIC0 || magic_nums[M1] != MAGIC1 || magic_nums[M2] != MAGIC2 || magic_nums[M3] != MAGIC3){
        sti();
        return -1;
    }

    // grabs the prog eip
    uint32_t prog_eip;
    if(read_data(exec_file.inode_number, (uint32_t)PROG_IP_OFFSET , (uint8_t*)&prog_eip, ELFS) == -1){
        sti();
        return -1;
    }

    //pcb initialization
    int exec_pid;
    for(i = 0; i < PROCESS_CAP; i++){
        if(pid_array[i] != 1){
            exec_pid = i;
            pid_array[i] = 1;
            break;
        }
    }

    // checks if process cap reached
    if(i == PROCESS_CAP){
        printf("PROCESSES FULL \n");
        sti();
        return -1;
    }

    // sets up paging for process
    page_directory[USER_MEM_INDEX].p4MB.page_base_address = ((PROG_LOAD_OFFSET + exec_pid) * FOUR_MB) >> PAGING_4MB_SHIFT; 

    // Flushing tlb
    asm volatile(
        "movl %%cr3, %%eax ;"
        "movl %%eax, %%cr3 ;"
        :
        :
        :"eax", "cc"
    );

    // load file
    inode_t* load_inode = (inode_t *)(Inode+exec_file.inode_number);
    read_data(exec_file.inode_number, 0, (uint8_t*)PROG_ADDR, load_inode->length);

    // assign pcb to right spot and then initialize
    pcb_t* pcb = (pcb_t*) get_curr_pcb(exec_pid);
    pcb->process_id = exec_pid;

    // loops through and sets up fd array
    for(i = 0; i < FD_NUM; i++){
        pcb->fd_array[i].file_operations_table_pointer = &null_fop;
        pcb->fd_array[i].inode_number = 0;
        pcb->fd_array[i].file_position = 0;
        pcb->fd_array[i].flags = 0;
    }

    // sets up stdin
    pcb->fd_array[0].file_operations_table_pointer = &stdin_fop;
    pcb->fd_array[0].inode_number = 0;
    pcb->fd_array[0].file_position = 0;
    pcb->fd_array[0].flags = 1;

    // sets up stdout
    pcb->fd_array[1].file_operations_table_pointer = &stdout_fop;
    pcb->fd_array[1].inode_number = 0;
    pcb->fd_array[1].file_position = 0;
    pcb->fd_array[1].flags = 1;

    // edge case for base shell
    pcb->process_id = exec_pid;
    if(exec_pid == terminals[scheduled_term].base_pid){
        pcb->parent_id = exec_pid;
    }else{
        pcb->parent_id = terminals[scheduled_term].active_pid;
    }

    // sets current process to active
    pcb->active = 1;

    // saves user eip and esp
    pcb->user_eip = prog_eip;
    pcb->user_esp = VM + FOUR_MB - sizeof(uint32_t);

    // argument copy for get args sys call
    memcpy((int8_t*)(pcb->arguments), (int8_t*)(args), FILENAME_SIZE);

    // update global pids
    glob_parent_pid = curr_pid;
    curr_pid = exec_pid;

    // saves esp and ebp into parent process
    uint32_t temp_esp;
    uint32_t temp_ebp;

    terminals[scheduled_term].active_pid = exec_pid;
    
    pcb_t * par = get_curr_pcb(pcb->parent_id);

    asm volatile (
        "movl %%esp, %0"
        : "=r" (temp_esp)
        :
        :"memory", "cc"
    );

    asm volatile (
        "movl %%ebp, %0"
        : "=r" (temp_ebp)
        :
        :"memory", "cc"
    );

    par->saved_esp = temp_esp; 
    par->saved_ebp = temp_ebp; 

    par->ts_esp = temp_esp; 
    par->ts_ebp = temp_ebp; 

    // tss switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - (terminals[scheduled_term].active_pid)*EIGHT_KB - sizeof(uint32_t);
    pcb->tss_esp0 = EIGHT_MB - (terminals[scheduled_term].active_pid)*EIGHT_KB - sizeof(uint32_t);

    // sets ds and cs
    uint16_t ds = USER_DS;
    uint16_t cs = USER_CS;
    sti();
    // go to user mode
    asm volatile(
        "movw %%ax, %%ds ;"
        "pushl %%eax ;"
        "movl %%ebx, %%eax ;"
        "pushl %%eax ;"
        "pushfl ;"
        "popl %%ebx ;"
        "orl $0x200, %%ebx ;"
        "pushl %%ebx ;"
        "pushl %%ecx ;"
        "pushl %%edx ;"
        "iret ;"
        :
        : "a" (ds),  "c" (cs), "d"(prog_eip),"b" (pcb->user_esp)
        : "memory"
    );
    return 0;
}

/*
int32_t read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: general read sys call, copies nbytes amount of bytes read into buf
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    // checks valid fd
    if(fd < 0 || fd > FD_NUM){
        return -1;
    }

    // checks valid number of bytes
    if(nbytes < 0){
        return -1;
    }
    
    // checks valid buf
    if(buf == NULL){
        return -1;
    }

    // grabs current pcb, checks if fd is active
    pcb_t* curr_pcb =  (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);
    if(curr_pcb->fd_array[fd].flags == 0){
        return -1;
    }

    // calls correct read function
    int32_t ret_val = curr_pcb->fd_array[fd].file_operations_table_pointer->read(fd, buf, nbytes);
    return ret_val;
}

/*
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: general write sys call, writes nbytes amount of bytes to respective location
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    if(fd < 0 || fd > FD_NUM){
        return -1;
    }

    // checks valid nbytes
    if(nbytes < 0){
        return -1;
    }

    // checks valid buf
    if(buf == NULL){
        return -1;
    }

    // grabs current pcb, checks if fd is active
    pcb_t* curr_pcb =  (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);
    if(curr_pcb->fd_array[fd].flags == 0){
        return -1;
    }
    
    // calls correct write function
    int32_t ret_val = curr_pcb->fd_array[fd].file_operations_table_pointer->write(fd, (void*)buf, nbytes);
    return ret_val;
}

/*
int32_t open(const uint8_t* filename)
Input: filename
Output: int32_t
Side Effects: general open sys call, opens correct file
*/
int32_t open(const uint8_t* filename){
    // checks valid filename
    if(filename == NULL || strlen((int8_t*)filename) == 0){
        return -1;
    }

    // checks if file exists and writes dentry
    dentry_t file_exits;
    if(read_dentry_by_name(filename, &file_exits) == -1){
        return -1;
    }

    // grabs current pcb
    pcb_t* curr_pcb =  (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);

    // intializes fd array and goes to correct open function
    int fd;
    for(fd = 0; fd < 8; fd++){
        if(curr_pcb->fd_array[fd].flags == 0){
            curr_pcb->fd_array[fd].flags = 1;
            curr_pcb->fd_array[fd].file_position = 0;
            curr_pcb->fd_array[fd].inode_number = file_exits.inode_number;
            switch(file_exits.file_type){
                case 0:
                    curr_pcb->fd_array[fd].file_operations_table_pointer = &rtc_fop;
                    break;
                case 1:
                    curr_pcb->fd_array[fd].file_operations_table_pointer = &dir_fop;
                    break;
                case 2:
                    curr_pcb->fd_array[fd].file_operations_table_pointer = &file_fop;
                    break;
                default:
                    break;
            }
            curr_pcb->fd_array[fd].file_operations_table_pointer->open(filename);
            return fd;
        }
    }
    return -1;
}

/*
int32_t close(int32_t fd)
Input: fd
Output: int32_t
Side Effects: general close sys call, closes corrects file
*/
int32_t close(int32_t fd){
    // checks valid fd
    if(fd < 0 || fd > FD_NUM){
        return -1;
    }

    // grabs current pcb, and checks if file exists
    pcb_t* curr_pcb = (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);
    if(curr_pcb->fd_array[fd].flags == 0){
        return -1;
    }
    
    // calls correct close function
    int32_t ret_val = curr_pcb->fd_array[fd].file_operations_table_pointer->close(fd);
    if(ret_val == -1){
        return -1;
    }
    // closes current pcb
    curr_pcb->fd_array[fd].flags = 0;
    curr_pcb->fd_array[fd].file_position = 0;
    curr_pcb->fd_array[fd].inode_number = 0;
    curr_pcb->fd_array[fd].file_operations_table_pointer = &null_fop;
    return ret_val;
}

/*
int32_t getargs(uint8_t* buf, int32_t nbytes)
Input: buf , nbytes
Output: int32_t
Side Effects: will grab the argument from exec and put into the buf with specified bytes
*/
int32_t getargs(uint8_t* buf, int32_t nbytes){
    // err check
    pcb_t* curr_pcb = (pcb_t*) get_curr_pcb(terminals[scheduled_term].active_pid);

    if(curr_pcb->arguments[0] == '\0' || curr_pcb->arguments == NULL || curr_pcb->arguments[0] == NULL){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    if(nbytes == 0){
        return -1;
    }
    memcpy((buf), (curr_pcb->arguments), FILENAME_SIZE);
    return 0;
}

/*
int32_t vidmap(uint8_t** screen_start)
Input: 
Output: int32_t
Side Effects: sets up the given double pointer to point to the Virtual address 0xb9000
*/
int32_t vidmap(uint8_t** screen_start){
    // null check
    if(screen_start == NULL){
        return -1;
    }
    // check in user space
    if((uint32_t)screen_start < VM || (uint32_t)screen_start >= VM+FOUR_MB){
        return -1;
    } 
    // paging setup
    vid_page_table[0].present = 1;
    vid_page_table[0].user_supervisor = 1;
    vid_page_table[1].present = 1;
    vid_page_table[1].user_supervisor = 1;
    vid_page_table[2].present = 1;
    vid_page_table[2].user_supervisor = 1;
    vid_page_table[0].address = VIRT_VID;
    vid_page_table[1].address = VIRT_VID;
    vid_page_table[2].address = VIRT_VID;

    *screen_start = (uint8_t*)(VIDMAP_ARG + FOUR_KB*(scheduled_term));
    
    // Flushing tlb
    asm volatile(
        "movl %%cr3, %%eax ;"
        "movl %%eax, %%cr3 ;"
        :
        :
        :"eax", "cc"
    );
    return 0;
}

/*
int32_t set_handler(int32_t signum, void* handler_address)
Input: 
Output: int32_t
Side Effects: 
*/
int32_t set_handler(int32_t signum, void* handler_address){
    return -1;
}

/*
int32_t sigreturn(void)
Input: 
Output: int32_t
Side Effects: 
*/
int32_t sigreturn(void){
    return -1;
}

/*
pcb_t* get_curr_pcb(uint32_t this_pid)
Input: this_pid
Output: int32_t
Side Effects: returns matching pcb
*/
pcb_t* get_curr_pcb(uint32_t this_pid){
    return (pcb_t*) (EIGHT_MB - (this_pid + 1)*EIGHT_KB);
}

/*
int32_t null_read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: 
*/
int32_t null_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/*
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: 
*/
int32_t null_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/*
int32_t null_open(const uint8_t* filename)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: 
*/
int32_t null_open(const uint8_t* filename){
    return -1;
}

/*
int32_t null_close(int32_t fd)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: 
*/
int32_t null_close(int32_t fd){
    return -1;
}
