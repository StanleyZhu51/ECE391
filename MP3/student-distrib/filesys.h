#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "lib.h"

// numbers
#define DENTRY_OFF 64
#define KB1 4096

#define FILENAME_SIZE 32
#define RESERVE0_SIZE 52
#define RESERVE1_SIZE 24
#define DENTRY_NUM 63
#define DATA_BLOCK_NUM 1023
/*
structs needed:
boot block
inode
data block

file type 0 for user-level
1 for directory
2 for regular file

index node number only meaningful for regular files ignore for rtc and directory types

open and close always successful
write should always fail
*/

// cock sycjer
typedef struct dentry_t {
    uint8_t filename[FILENAME_SIZE];
    uint32_t file_type;
    uint32_t inode_number;
    uint8_t reserved1[RESERVE1_SIZE];
} dentry_t;

typedef struct boot_block_t {
    uint32_t dentry_num;
    uint32_t inode_num;
    uint32_t data_block_num;
    uint8_t reserved0[RESERVE0_SIZE];
    dentry_t dentries[DENTRY_NUM];
} boot_block_t;

typedef struct inode_t {
    uint32_t length;
    uint32_t data_block_number[DATA_BLOCK_NUM];
} inode_t;

typedef struct data_block_t {
    uint8_t data[KB1]; 
} data_block_t;

boot_block_t* Boot_block;

inode_t* Inode;

data_block_t* Data_block;

dentry_t* Dentry;

void filesystem_init(uint32_t fs_start);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_write(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t* filename);

int32_t dir_close(int32_t fd);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t file_write(int32_t fd, void* buf, int32_t nbytes);

int32_t file_open(const uint8_t* filename);

int32_t file_close(int32_t fd);

#endif /* FILESYS.H */

