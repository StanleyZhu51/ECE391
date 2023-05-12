#ifndef _RTC_H
#define _RTC_H

#include "i8259.h"
#include "lib.h"
#include "terminal.h"

uint32_t num_hits;

volatile int rtc_interrupt;

volatile int rtc_read_flag;

uint8_t virtualization_rtc;

uint32_t num_int_gen;

#define RTC_PORT 0x70
#define RTC_DATA 0x71

#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C

#define DEF_FREQ 1024 

#define RTC_IRQ_NUM 0x8

    void rtc_init();

    void rtc_handler();

    int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

    int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);

    int32_t rtc_open(const uint8_t* filename);

    int32_t rtc_close(int32_t fd);

#endif /* RTC.H */
