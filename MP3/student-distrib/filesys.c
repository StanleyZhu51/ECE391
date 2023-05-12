#include "filesys.h"
#include "syscall.h"

uint32_t file_counter = 0;
uint32_t bytes_readed = 0;

boot_block_t* Boot_block;

inode_t* Inode;

data_block_t* Data_block;

dentry_t* Dentry;

/*
filesystem_init(uint32_t fs_start)
Input: fs_start
Output: none
Side Effects: initializes global pointers
*/
void filesystem_init(uint32_t fs_start){
    // intialize global pointers
    Boot_block = (boot_block_t*) fs_start;
    Dentry = (dentry_t*) (fs_start + DENTRY_OFF);
    Inode = (inode_t*) (fs_start + KB1);
    Data_block = (data_block_t*) (fs_start + (Boot_block->inode_num + 1)*KB1);  
}

/*
read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
Input: fname, dentry
Output: int32_t
Side Effects: Fill the given dentry with contents matching input filename
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int index;
    // loop through dentries indices
    for(index = 0; index < DENTRY_NUM; index++){       
        // check if names match
        if(strncmp((int8_t*)(Boot_block->dentries[index].filename), (int8_t*)(fname), FILENAME_SIZE) == 0){
            // copy info into new dentry
            memcpy(&(dentry->filename), &(Boot_block->dentries[index].filename), FILENAME_SIZE);
            dentry->file_type = Boot_block->dentries[index].file_type;
            dentry->inode_number = Boot_block->dentries[index].inode_number;
            return strlen((int8_t*)fname);
        }
    }
    return -1;
}

/*
read_dentry_by_index(uint32_t index, dentry_t* dentry)
Input: index, dentry
Output: int32_t
Side Effects: Fill the given dentry with contents matching input index
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    // check if valid index
    if(index < 0 || index >= DENTRY_NUM){
        return -1;
    }
    if(index >= Boot_block->dentry_num){
        return -1;
    }
    // copy info into new dentry
    memcpy(&(dentry->filename), &(Boot_block->dentries[index].filename), FILENAME_SIZE);
    dentry->file_type = Boot_block->dentries[index].file_type;
    dentry->inode_number = Boot_block->dentries[index].inode_number;
    return FILENAME_SIZE;
}

/*
read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
Input: inode, offset, buf, length
Output: int32_t
Side Effects: Fill buf "length" number of bytes with the data from given inode + offset
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // check valid inode
     if(inode >= Boot_block->inode_num){
        return -1;
    }

    inode_t* readinode = &(Inode[inode]);

    if(offset == readinode->length){
        return 0;
    }else if(offset > readinode->length){
        return -1;
    }

    // set indexes
    uint32_t inode_index = offset / KB1;
    uint32_t data_block_index = offset % KB1;
    uint32_t readdatablock = readinode->data_block_number[inode_index];
    // check valid block
    if(readdatablock > Boot_block->data_block_num){
        return -1;
    }

    // loop through indexes
    uint32_t index;
    for(index = 0; index < length; index++, data_block_index++){

        // check if need to reset data block index
        if(data_block_index >= KB1){
            data_block_index = 0;
            inode_index++;

            if(inode_index >= Boot_block->inode_num){
                return index;
            }else{
                readdatablock = readinode->data_block_number[inode_index];
                if(readdatablock > Boot_block->data_block_num){
                    return index;
                }
            }
        }

        // check if in bounds
        if(offset + index >= readinode->length){
            break;
        }

        // copy data into buf
        memcpy(&buf[index], &(Data_block[readdatablock].data[data_block_index]), 1);
        //strncpy(&buf[index], &(Data_block[readdatablock].data[data_block_index]), 1);
    }
    return index;
}

/*
dir_read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Writes directory name into buf
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
    // check if reached number of dentries
    if(fd >= Boot_block->dentry_num){
        return -1;
    }
    pcb_t * pcb = get_curr_pcb(terminals[terminal].active_pid);
    // read dentry
    dentry_t extract;
    uint32_t fp = pcb->fd_array[fd].file_position;
    if(fp >= 17){
        return 0;
    }
    uint32_t bytes_read = read_dentry_by_index(fp, &extract);
    // copy file name into buf
    strncpy(buf, (int8_t*) &(extract.filename), FILENAME_SIZE);
    pcb->fd_array[fd].file_position++;
    return bytes_read;
}

/*
dir_write(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Does nothing - WRITE ONLY Filesystem
*/
int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/*
dir_open(const uint8_t* filename)
Input: filename
Output: int32_t
Side Effects: None
*/
int32_t dir_open(const uint8_t* filename){
    return 0;
}

/*
dir_close(int32_t fd)
Input: fd
Output: int32_t
Side Effects: None
*/
int32_t dir_close(int32_t fd){
    return 0;
}

/*
file_read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Reads files contents and copies into buf
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    // check if passed dentry name
    if(file_counter >= Boot_block->dentry_num){
        return -1;
    }
    pcb_t* pcb = get_curr_pcb(terminals[terminal].active_pid);

    // read data
    uint32_t bytes_return = read_data(pcb->fd_array[fd].inode_number, pcb->fd_array[fd].file_position, buf, nbytes);
    if(bytes_return < 0){
        return -1;
    }
    pcb->fd_array[fd].file_position += bytes_return;
    return bytes_return;
}

/*
file_write(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: None
*/
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

/*
file_open(const uint8_t* filename)
Input: filename
Output: int32_t
Side Effects: None
*/
int32_t file_open(const uint8_t* filename){
    return 0;
}

/*
file_close(int32_t fd)
Input: fd
Output: int32_t
Side Effects: None
*/
int32_t file_close(int32_t fd){
    return 0;
}
