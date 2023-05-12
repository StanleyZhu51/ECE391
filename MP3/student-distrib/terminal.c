#include "terminal.h"

char* vid_mem = (char *)TV;

/*
void terminal_init(void)
Input: void
Output: void
Side Effects: initializes terminals for term switching
*/
void terminal_init(){
    // init terminal variables
    int32_t i;
    terminal = 0;
    terminals[TERM0].active = 1;
    terminals[TERM0].curridx = 0;
    terminals[TERM0].read_flag = 0;
    terminals[TERM0].rtc_read_flag = 0;
    terminals[TERM0].num_int_gen = 0;
    terminals[TERM0].num_hits = 1;
    terminals[TERM0].on = 1;
    terminals[TERM0].screen_x = 0;
    terminals[TERM0].screen_y = 0;
    terminals[TERM0].active_pid = 0;
    terminals[TERM0].base_pid = 0;
    
    // fill buffers with default values
    for(i = 0; i < MAX_CHAR; i++){
        terminals[TERM0].terminal_read_buff[i] = '\0';
    }

    for(i = 0; i < SCREEN_BUFF_SIZE; i++){
        if(i % 2 == 0){
            terminals[TERM0].screen_buff[i] = ' ';
        }else{
            terminals[TERM0].screen_buff[i] = ATTRIB;
        }
    }

    // repeat for index 1 (the second terminal) in terminal array
    terminals[TERM1].active = 0;
    terminals[TERM1].curridx = 0;
    terminals[TERM1].on = 0;
    terminals[TERM1].read_flag = 0;
    terminals[TERM1].rtc_read_flag = 0;
    terminals[TERM1].num_int_gen = 0;
    terminals[TERM1].num_hits = 1; 
    terminals[TERM1].screen_x = 0;
    terminals[TERM1].screen_x = 0;
    terminals[TERM1].active_pid = -1;
    terminals[TERM1].base_pid = -1;
    for(i = 0; i < MAX_CHAR; i++){
        terminals[TERM1].terminal_read_buff[i] = '\0';
    }

    for(i = 0; i < SCREEN_BUFF_SIZE; i++){
        if(i % 2 == 0){
            terminals[TERM1].screen_buff[i] = ' ';
        }else{
            terminals[TERM1].screen_buff[i] = ATTRIB;
        }
    }

    // repeat for index 2 (the second terminal) in terminal array
    terminals[TERM2].active = 0;
    terminals[TERM2].read_flag = 0;
    terminals[TERM2].curridx = 0;
    terminals[TERM2].rtc_read_flag = 0;
    terminals[TERM2].num_int_gen = 0;
    terminals[TERM2].num_hits = 1;
    terminals[TERM2].on = 0;
    terminals[TERM2].screen_x = 0;
    terminals[TERM2].screen_x = 0;
    terminals[TERM2].active_pid = -1;
    terminals[TERM2].base_pid = -1;
    for(i = 0; i < MAX_CHAR; i++){
        terminals[TERM2].terminal_read_buff[i] = '\0';
    }

    for(i = 0; i < SCREEN_BUFF_SIZE; i++){
        if(i % 2 == 0){
            terminals[TERM2].screen_buff[i] = ' ';
        }else{
            terminals[TERM2].screen_buff[i] = ATTRIB;
        }
    }
    return;
}

/*
void terminal_switch(int32_t term_num)
Input: term_num
Output: void
Side Effects: switches visible terminal to given input terminal number
*/
void terminal_switch(int32_t term_num){
    // switch active term
    cli();
    terminals[terminal].read_flag = 0;

    terminals[terminal].active = 0;
    terminals[term_num].active = 1;

    // save terminal x and y
    int32_t t_screen_x = terminals[terminal].screen_x;
    int32_t t_screen_y = terminals[terminal].screen_y;

    // check valid terminal
    if(term_num == terminal || term_num == -1){
        return;
    }

    // save current screen
    char* new_video_mem = (char *)(VIDEO + FOUR_KB*(terminal+2));
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        terminals[terminal].screen_buff[(i << 1)] = *(uint8_t *)(vid_mem + (i << 1));
        terminals[terminal].screen_buff[(i << 1) + 1] = *(uint8_t *)(vid_mem + (i << 1) + 1);
    }

    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(new_video_mem + (i << 1)) = terminals[terminal].screen_buff[(i << 1)];
        *(uint8_t *)(new_video_mem + (i << 1) + 1) = terminals[terminal].screen_buff[(i << 1) + 1];
    }

    // restore next screen properties
    terminals[terminal].screen_x = t_screen_x;
    terminals[terminal].screen_y = t_screen_y;

    char* n_video_mem = (char *)(VIDEO + FOUR_KB*(term_num+2));

    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        terminals[term_num].screen_buff[(i << 1)] = *(uint8_t *)(n_video_mem + (i << 1));
        terminals[term_num].screen_buff[(i << 1) + 1] = *(uint8_t *)(n_video_mem + (i << 1) + 1);
    }

    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(vid_mem + (i << 1)) = terminals[term_num].screen_buff[(i << 1)];
        *(uint8_t *)(vid_mem + (i << 1) + 1) = terminals[term_num].screen_buff[(i << 1) + 1];
    }
    
    terminal = term_num;
    sti();
}




/*
terminal_read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: copies from terminal buf into the input buffer
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    // check flag
    while(terminals[scheduled_term].read_flag != 1);
    terminals[scheduled_term].read_flag = 0;
    // check valid input
    if(nbytes <= 0){
        return -1;
    }else if(nbytes > MAX_CHAR){
        nbytes = MAX_CHAR;
    }
    // copies into buf
    memcpy(buf, &terminals[scheduled_term].terminal_read_buff, nbytes);
    int i = 0;
    while(1)
    {
        if(terminals[scheduled_term].terminal_read_buff[i] == '\0')
        {
            break;
        }
        i++;
    }
    return i;
}

/*
terminal_write(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Writes to screen with contents in buf
*/
int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes){
    int32_t i;
    char* bf = buf;
    // check valid input
    if(nbytes <= 0){
        return -1;
    }
    // prints contents of buf to screen
    for(i = 0; i < nbytes; i++){
        cli();
        putc(bf[i]);
        sti();
    }
    return 0;
}

/*
terminal_open(const uint8_t* filename)
Input: filename
Output: int32_t
Side Effects: None
*/
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/*
terminal_close(int32_t fd)
Input: fd
Output: int32_t
Side Effects: None
*/
int32_t terminal_close(int32_t fd){
    return -1;
}
