#include "keyboard.h"

char shift_pressed = 0;
char capslock_pressed = 0;
char alt_pressed = 0;
char ctrl_pressed = 0;

void disable_cursor(){
    // turn off cursor
	outb(CUR_DATA1, CUR_CPORT);
	outb(CUR_DATA2, CUR_DPORT);
}

// LOOK INTO E0 stuff
// normal, capslock, shift
char scan_code_to_char[NUM_KEYS][NUM_MODES] = {
    {' ', ' ', ' ', ' '}, // escape
    {'1', '1', '!', '!'},
    {'2', '2', '@', '@'},
    {'3', '3', '#', '#'},
    {'4', '4', '$', '$'},
    {'5', '5', '%', '%'},
    {'6', '6', '^', '^'},
    {'7', '7', '&', '&'},
    {'8', '8', '*', '*'},
    {'9', '9', '(', '('},
    {'0', '0', ')', ')'},
    {'-', '-', '_', '_'},
    {'=', '=', '+', '+'},
    {'\b', '\b', '\b', '\b'}, // backspace
    {'\t', '\t', '\t', '\b'}, // tab
    {'q', 'Q', 'Q', 'q'},
    {'w', 'W', 'W', 'w'},
    {'e', 'E', 'E', 'e'},
    {'r', 'R', 'R', 'r'},
    {'t', 'T', 'T', 't'},
    {'y', 'Y', 'Y', 'y'},
    {'u', 'U', 'U', 'u'},
    {'i', 'I', 'I', 'i'},
    {'o', 'O', 'O', 'o'},
    {'p', 'P', 'P', 'p'},
    {'[', '[', '{', '{'},
    {']', ']', '}', '}'},
    {'\n', '\n', '\n', '\n'}, // enter
    {' ', ' ', ' ', ' '}, // left ctrl
    {'a', 'A', 'A', 'a'},
    {'s', 'S', 'S', 's'},
    {'d', 'D', 'D', 'd'},
    {'f', 'F', 'F', 'f'},
    {'g', 'G', 'G', 'g'},
    {'h', 'H', 'H', 'h'},
    {'j', 'J', 'J', 'j'},
    {'k', 'K', 'K', 'k'},
    {'l', 'L', 'L', 'l'},
    {';', ';', ':', ':'},
    {'\'', '\'', '\"', '\"'},
    {'`', '`', '~','~'},
    {' ', ' ', ' ', ' '}, // left shift
    {'\\', '\\', '\\', '\\'},
    {'z', 'Z', 'Z', 'z'},
    {'x', 'X', 'X', 'x'},
    {'c', 'C', 'C', 'c'},
    {'v', 'V', 'V', 'v'},
    {'b', 'B', 'B', 'b'},
    {'n', 'N', 'N', 'n'},
    {'m', 'M', 'M', 'm'},
    {',', ',', '<', '<'},
    {'.', '.', '>', '>'},
    {'/', '/', '?', '?'},
    {' ', ' ', ' ', ' '}, // right shift
    {'*', '*', '*', '*'},
    {' ', ' ', ' ', ' '}, // LEFT ALT
    {' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' '}, // capslock
    {' ', ' ', ' ', ' '}, //f1
    {' ', ' ', ' ', ' '}, //f2
    {' ', ' ', ' ', ' '}, //f3
    {' ', ' ', ' ', ' '}, //f4
    {' ', ' ', ' ', ' '}, //f5
    {' ', ' ', ' ', ' '}, //f6
    {' ', ' ', ' ', ' '}, //f7
    {' ', ' ', ' ', ' '}, //f8
    {' ', ' ', ' ', ' '}, //f9
    {' ', ' ', ' ', ' '} //f10
};

/* void keyboard_init()
 * Inputs: void
 * Return Value: none
 * Function: initializes keybord */
void keyboard_init(){
    enable_irq(KEYBOARD_IRQ_NUM);
    // enable irq and potential things you may need
}

/* keyboard_handler()
 * Inputs: void
 * Return Value: none
 * Function: handles keyboard */
void keyboard_handler(){
    // grab key from port
    unsigned char key = inb(KEYBOARD_PORT);
    cli();
    char entered = 0;
    switch(key){
        case LEFT_CTRL:
            entered = 1;
            ctrl_pressed = 1;
            break;

        case LEFT_SHIFT:
            entered = 1;
            shift_pressed = 1;
            break;

        case RIGHT_SHIFT:
            entered = 1;
            shift_pressed = 1;
            break;

        case LEFT_ALT:
            entered = 1;
            alt_pressed = 1;
            break;

        case CAPSLOCK:
            entered = 1;
            if(capslock_pressed == 0){
                capslock_pressed = 1;
            }else{
                capslock_pressed = 0;
            }
            break;

        case LEFT_CTRL_REL:
            entered = 1;
            ctrl_pressed = 0;
            break;

        case LEFT_SHIFT_REL:
            entered = 1;
            shift_pressed = 0;
            break;

        case RIGHT_SHIFT_REL:
            entered = 1;
            shift_pressed = 0;
            break;

        case LEFT_ALT_REL:
            entered = 1;
            alt_pressed = 0;
            break;

        default:
            break;
    }
    if(alt_pressed == 1 && (key >= F1 && key <= F3)){
        int32_t term_num;
        switch (key)
        {
        case F1:
            term_num = 0;
            break;
        case F2:
            term_num = 1;
            break;
        case F3:
            term_num = 2;
            break;
        default:
            term_num = -1;
            break;
        }

        send_eoi(KEYBOARD_IRQ_NUM);
        terminal_switch(term_num);
        return; 
    }
    if(entered == 1 || (key <= 0 || key >= KB_UPBOUND)){
        sti();
        send_eoi(KEYBOARD_IRQ_NUM);
        return;
    }
    key--;
    unsigned char whichthird = 0;
    if(capslock_pressed == 1){
        whichthird = 1;
    }
    if(shift_pressed == 1){
        whichthird = 2;
    }
    if(shift_pressed == 1 && capslock_pressed == 1){
        whichthird = 3;
    }
    if(terminals[terminal].curridx >= MAX_CHAR){
        terminals[terminal].keyboard_buff[MAX_CHAR-1] = '\n';
        terminals[terminal].curridx = MAX_CHAR-1;
        key = ENTER;
    }
    if(scan_code_to_char[key][whichthird] == '\b'){
        if(terminals[terminal].curridx > 0){
            backspace();
            terminals[terminal].keyboard_buff[terminals[terminal].curridx] = '\0';
            terminals[terminal].curridx--;
        }
        // run vga backspace routine
    }else if(ctrl_pressed == 1 && (scan_code_to_char[key][whichthird] == 'L' || scan_code_to_char[key][whichthird] == 'l')){
        clear_tv();
        int transfer;
        for(transfer = 0; transfer < MAX_CHAR; transfer++){
            terminals[terminal].terminal_read_buff[transfer] = terminals[terminal].keyboard_buff[transfer];
        }
        terminals[terminal].curridx = 0;
        for(transfer = 0; transfer < MAX_CHAR; transfer++){
            terminals[terminal].keyboard_buff[transfer] = '\0';
        }
    }else if(scan_code_to_char[key][whichthird] == '\n'){
        terminals[terminal].keyboard_buff[terminals[terminal].curridx] = scan_code_to_char[key][whichthird];
        int transfer;
        for(transfer = 0; transfer < MAX_CHAR; transfer++){
            terminals[terminal].terminal_read_buff[transfer] = terminals[terminal].keyboard_buff[transfer];
        }
        terminals[terminal].read_flag = 1;
        putk(scan_code_to_char[key][whichthird]);
        terminals[terminal].curridx = 0;
        scroll_up_tv();
        for(transfer = 0; transfer < MAX_CHAR; transfer++){
            terminals[terminal].keyboard_buff[transfer] = '\0';
        }
        // run vga scroll up routine
    }else if(scan_code_to_char[key][whichthird] == '\t'){
        int tabidx = TAB_SPACES;
        while(tabidx != 0 && terminals[terminal].curridx < MAX_CHAR){
            putk(scan_code_to_char[key][whichthird]);
            tabidx--;
            terminals[terminal].curridx++;
        }
    }else{
        terminals[terminal].keyboard_buff[terminals[terminal].curridx] = scan_code_to_char[key][whichthird];
        terminals[terminal].curridx++;
        putk(scan_code_to_char[key][whichthird]);
    }
    sti();
    send_eoi(KEYBOARD_IRQ_NUM);
}

