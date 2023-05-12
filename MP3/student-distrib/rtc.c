#include "rtc.h"


/*
rtc_init
Input: none
Output: none
Side Effects: enables rtc interrupts
*/

void rtc_init(){
    num_int_gen = 0;
    virtualization_rtc = 0;
    //num_hits = 1;
    //rtc_read_flag = 0;

    outb(RTC_REG_A, RTC_PORT);
    char prev = inb(RTC_DATA);
    outb(RTC_REG_B, RTC_PORT);
    outb(prev | 0x40, RTC_DATA);

    enable_irq(RTC_IRQ_NUM);
    // copied from os dev, this should initialized rtc at 1024 HZ by default
}

/*
rtc_handler
Input: none
Output: none
Side Effects: runs test interrupts to test rtc
*/
void rtc_handler(){
    //cli();
    //printf("RTC INT GENERATED %x !", 1);
    if(terminals[scheduled_term].num_int_gen < terminals[scheduled_term].num_hits){
        terminals[0].num_int_gen++;
        terminals[1].num_int_gen++;
        terminals[2].num_int_gen++;

    }
    else{
        terminals[scheduled_term].num_int_gen = 0;
        virtualization_rtc++;
        terminals[scheduled_term].rtc_read_flag = 1;
    }
    //test_interrupts();
    outb(RTC_REG_C, RTC_PORT);
    inb(RTC_DATA);
    /*It is important to know that upon a IRQ 8, Status Register C will contain a bitmask telling 
    which interrupt happened. The RTC is capable of producing a periodic interrupt (what this article 
    describes), an update ended interrupt, and an alarm interrupt.
    If you are only using the RTC as a simple timer this is not important. What is important is that 
    if register C is not read after an IRQ 8, then the interrupt will not happen again. 
    So, even if you don't care about what type of interrupt it is, just attach this code to the bottom
    of your IRQ handler to be sure you get another interrupt. */
    send_eoi(RTC_IRQ_NUM);
    //sti();
}

/*
rtc_read(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Sets flag to 0 and waits for interrupt to flip flag
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    // set flag 0

    // wait for interrupt
    while(terminals[scheduled_term].rtc_read_flag != 1);
    terminals[scheduled_term].rtc_read_flag = 0;
    return 0;
}

/*
rtc_write(int32_t fd, void* buf, int32_t nbytes)
Input: fd, buf, nbytes
Output: int32_t
Side Effects: Changes rtc frequency
*/
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes){
    // error check
    
    if(buf == NULL){
        return -1;
    }
    if(sizeof(buf) != sizeof(uint32_t)){
        return -1;
    }
    // update frequency
    uint32_t freq = *(uint32_t*)buf;
    // check if freq is power of 2
    if((freq != 0) && ((freq & (freq - 1)) == 0)){
        terminals[scheduled_term].num_hits = DEF_FREQ / (2*freq);
        return 0;
    } 
    return -1;
}

/*
rtc_open(const uint8_t* filename)
Input: filename
Output: int32_t
Side Effects: Sets default frequency
*/
int32_t rtc_open(const uint8_t* filename){
    terminals[scheduled_term].num_hits = DEF_FREQ / 2;
    return 0;
}

/*
rtc_close(int32_t fd)
Input: fd
Output: int32_t
Side Effects: None
*/
int32_t rtc_close(int32_t fd){
    return 0;
}
