#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"
#include "terminal.h"

#define KEYBOARD_PORT 0x60 // data port

#define ESCAPE 0x01
#define LEFT_CTRL 0x1D
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define LEFT_ALT 0x38
#define CAPSLOCK 0x3A

#define ESCAPE_REL 0x81
#define LEFT_CTRL_REL 0x9D
#define LEFT_SHIFT_REL 0xAA
#define RIGHT_SHIFT_REL 0xB6
#define LEFT_ALT_REL 0xB8
#define CAPSLOCK_REL 0xBA

#define NUM_KEYS 0x58
#define NUM_MODES 4

#define CUR_DATA1 0x0A
#define CUR_DATA2 0x20
#define CUR_CPORT 0x3D4
#define CUR_DPORT 0x3D5

#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define KB_UPBOUND 0x3A
#define ENTER 0x1B
#define TAB_SPACES 4

#define KEYBOARD_IRQ_NUM 0x1

    void disable_cursor();

    void keyboard_init();

    void keyboard_handler();

#endif /* KEYBOARD.H */
