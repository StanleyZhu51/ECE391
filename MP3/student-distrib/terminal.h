#ifndef _TERMINAL_H
#define _TERMINAL_H



#include "types.h"
#include "lib.h"
#include "syscall.h"

#define TERM0 0
#define TERM1 1
#define TERM2 2
#define TERM_COUNT 3
#define MAX_CHAR 128
#define SCREEN_BUFF_SIZE 4000
#define FOUR_KB 0x1000

char terminal_read_buff[MAX_CHAR];

volatile uint8_t read_flag;

uint32_t terminal;

uint32_t writer;

typedef struct terminal_t {
    int32_t on;
    int32_t active;
    int32_t screen_x;
    int32_t screen_y;
    int32_t active_pid;
    int32_t base_pid;

    int32_t curridx;

    int32_t num_hits;

    int32_t saved_read_flag;
    
    volatile int32_t rtc_read_flag;
    volatile int32_t num_int_gen;
    volatile int32_t read_flag;

    char terminal_read_buff[MAX_CHAR];
    char keyboard_buff[MAX_CHAR];
    char screen_buff[SCREEN_BUFF_SIZE];

} terminal_t;

terminal_t terminals[TERM_COUNT];

void terminal_init();

void terminal_switch(int32_t term_num);

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_open(const uint8_t* filename);

int32_t terminal_close(int32_t fd);

#endif /* TERMINAL.H */
