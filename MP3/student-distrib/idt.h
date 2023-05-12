/*
    idt.h
    Contains all the idtentries that have been implemeted [0, 255]
*/
#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "exception_linkage.h"
#include "interrupt.h"
#include "syscall_linkage.h"
#include "exceptions.h"


//syscalls soon

#ifndef ASM

    void idt_init();

#endif /* ASM */

#define KEYBOARD_INTERRUPT_NUM 0x21
#define RTC_INTERRUPT_NUM 0x28
#define PIT_INTERRUPT_NUM 0x20

#define INT_VECTOR 0x80

#define INTERRUPT_START 0x20
#define INTERRUPT_END 0x2F

#endif /* IDT.H */
