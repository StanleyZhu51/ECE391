#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "syscall_linkage.h"
#include "filesys.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "pit.h"


#define M0 0
#define M1 1
#define M2 2
#define M3 3

#define MAGIC0 0x7F
#define MAGIC1 0x45
#define MAGIC2 0x4C
#define MAGIC3 0x46
#define ELFS 4

#define FOUR_MB 0x400000
#define EIGHT_MB 0x800000
#define EIGHT_KB 0x2000
#define VM 0x08000000
#define PAGING_4MB_SHIFT 22
#define FD_NUM 8
#define PROG_ADDR 0x08048000
#define PROG_LOAD_OFFSET 2
#define PROG_IP_OFFSET 24
#define VIDMAP_ARG 0x08400000
#define VIDMAP_ADD 0xB8
#define PROCESS_CAP 6

uint32_t curr_pid;
uint32_t glob_parent_pid;

typedef struct file_operations_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, void* buf, int32_t nbytes);
} file_operations_t;

typedef struct file_descriptor_t {
    file_operations_t* file_operations_table_pointer;
    uint32_t inode_number;
    uint32_t file_position;
    uint32_t flags;
} file_descriptor_t;

typedef struct pcb_t {
    uint32_t process_id;
    uint32_t parent_id;

    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t ts_esp;
    uint32_t ts_ebp;
    int32_t active;

    uint32_t tss_esp0;
    uint32_t user_esp;
    uint32_t user_eip;

    file_descriptor_t fd_array[8];
    uint8_t arguments[32];
} pcb_t;

uint8_t pid_array[6];

int32_t halt(uint8_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, const void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

int32_t getargs(uint8_t* buf, int32_t nbytes);

int32_t vidmap(uint8_t** screen_start);

int32_t set_handler(int32_t signum, void* handler_address);

int32_t sigreturn(void);

pcb_t* get_curr_pcb(uint32_t pcb_pid);

uint8_t* parse_command(const uint8_t* command);

int32_t null_read(int32_t fd, void* buf, int32_t nbytes);

int32_t null_write(int32_t fd, void* buf, int32_t nbytes);

int32_t null_open(const uint8_t* filename);

int32_t null_close(int32_t fd);

#endif /* SYSCALL.H*/
