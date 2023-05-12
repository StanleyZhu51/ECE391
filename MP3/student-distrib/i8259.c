/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    uint8_t master_mask;
    uint8_t slave_mask;

    master_mask = inb(MASTER_DATA);
    slave_mask = inb(SLAVE_DATA); // saving masks will put back in at the end
    
    outb(0xff, MASTER_DATA);
    outb(0xff, SLAVE_DATA); // mask everthing on secondary and primary pics

    outb(ICW1, MASTER_8259_PORT); // selected the primary pic
    outb(ICW2_MASTER, MASTER_DATA); // IR 0 - 7 mapped to 0x20 - 0x27
    outb(ICW3_MASTER, MASTER_DATA); // The secondary pic is IRQ 02
    outb(ICW4, MASTER_DATA); // ISA = x86 / normal EOI

    outb(ICW1, SLAVE_8259_PORT); // selected the secondary pic
    outb(ICW2_SLAVE, SLAVE_DATA); // IR 0 - 7 mapped for 0x28 - 0x2F
    outb(ICW3_SLAVE, SLAVE_DATA); // Recognizes that its IRQ 2 on the primary pic
    outb(ICW4, SLAVE_DATA); // ISA = x86 / Normal EOI

    outb(master_mask, MASTER_DATA);
    outb(slave_mask, SLAVE_DATA); // write the masks back to the pic
    
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

    uint32_t port;
    uint8_t value;

    if(irq_num < 8){ // decide whether secondary or primary pic
        port = MASTER_DATA;
    }else{
        port = SLAVE_DATA;
        irq_num = irq_num - 8; // need to set secondary pic
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port); // write to pic 

}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint32_t port;
    uint8_t value;

    if(irq_num < 8){ // decide whether secondary or primary pic
        port = MASTER_DATA;
    }else{
        port = SLAVE_DATA;
        irq_num = irq_num - 8; // need to set secondary pic
    }
    value = inb(port) | ~(1 << irq_num);
    outb(value, port); // writing to pic
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8){ // if slave pic send EOI to secondary pic too
        outb((EOI), SLAVE_8259_PORT);
    }
    outb((EOI), MASTER_8259_PORT); // sends EOI to primary pic
}
