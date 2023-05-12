/*
    interrupt.h
    Contains all the interrupts that need to be implemeted
*/
#ifndef _INTERRUPT_H
#define _INTERRUPT_H


#ifndef ASM

    #include "rtc.h"
    #include "keyboard.h"
    #include "pit.h"
    void rtc_handler_linkage();
    void keyboard_handler_linkage();
    void pit_handler_linkage();
    
#endif /* ASM */

//INTR_LINK(rtc_handler_linkage, rtc_handler);
//INTR_LINK(keyboard_handler_linkage, keyboard_handler);

#endif /* INTERRUPT.H */
