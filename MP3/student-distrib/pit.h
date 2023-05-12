#ifndef _PIT_H
#define _PIT_H

#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

#define VIRT_VID 0xB8
#define VID_TERM_OFFSET 2
#define DEF_DIVIDEND 1193180
#define PIT_HZ 50
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT 0x40
#define PIT_COMMAND 0x36
#define PIT_MASK 0xFF
#define RIGHT_SHIFT_8 8

uint32_t timer_ticks;

volatile uint32_t scheduled_term;

uint32_t seconds;

void pit_init(void);

void pit_handler(void);

void schedule(void);

#endif /* PIT.H */
