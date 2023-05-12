/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)
// globals
unsigned char led_flag = 0x00; // 0x01 - initializing, 0x02 - ready to set led, 0x03 - writing to led
unsigned char flags = 0x00; //bit 0 = led ready
unsigned char buttons_status;
int led_rdy = 0;
unsigned char convert[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 
						     0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};
unsigned char buf[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* 
	0 = 11100111 - 0xE7
	1 = 00000110 - 0x06
	2 = 11001011 - 0xCB
	3 = 10001111 - 0x8F
	4 = 00101110 - 0x2E
	5 = 10101101 - 0xAD
	6 = 11101101 - 0xED
	7 = 10000110 - 0x86
	8 = 11101111 - 0xEF
	9 = 10101111 - 0xAF 
	A = 11101110 - 0xEE
	B = 01101101 - 0x6D (lower case b)
	C = 11100001 - 0xE1
	D = 01001111 - 0x4F (lower case d)
	E = 11101001 - 0xE9
	F = 11101000 - 0xE8
	*/
// local functions
int tux_initializtion(struct tty_struct* tty);
int tux_led(struct tty_struct* tty, unsigned long arg);
int tux_but(struct tty_struct* tty, unsigned long arg);
int tux_reset(struct tty_struct* tty);
/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned char left, down;
    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    //printk("packet : %x %x %x\n", a, b, c);
	switch(a){
		case MTCP_ACK:
			if((led_flag == 0x01) || (led_flag == 0x03)) // flag check if led rdy
			{
				led_flag = 0x02;
			}
			break;
		case MTCP_RESET:
			tux_reset(tty);
			//tux_led(tty, leds_status);
			break;
		case MTCP_BIOC_EVENT:
			left = (c & 0x02) << 1; // isolate left and shift left
			down = (c & 0x04) >> 1; // isolate down and shift down

			c = (c & 0x09);	// clear left and down
			c = c | left;   // swap locations of left and down
			c = c | down;
			c = c << 4;  //slide CBAS a byte
			c = c | (b & 0x0F);  //copy in RDLU
			//printk("kern: %c", c);
			buttons_status = c;
			break;
		default:
			return;
	}
	return;
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tux_initializtion(tty);
	case TUX_BUTTONS:
		return tux_but(tty, arg);
	case TUX_SET_LED:
		return tux_led(tty, arg);
	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

int tux_initializtion(struct tty_struct* tty)
{
	char buf1[1];
	buf1[0] = MTCP_RESET_DEV;				// send reset cmd, will go to reset helper once packet received
	if(tuxctl_ldisc_put(tty, buf1, 1) > 0)
		return -1;
	buttons_status = 0xFF;				// set globals		
	led_flag = 0x01;
	return 0;
}

int tux_reset(struct tty_struct* tty)
{
	char buf1[2];
	buf1[0] = MTCP_BIOC_ON;
	buf1[1] = MTCP_LED_USR;
	tuxctl_ldisc_put(tty, buf1, 2);

	//tuxctl_ldisc_put(tty, buf, 6);
	buttons_status = 0xFF;						

	return 0;
}

int tux_led(struct tty_struct* tty, unsigned long arg)
{	
	// separate parts of arg
	unsigned int num = (arg & 0x0000FFFF);
	unsigned int leds = (arg & 0x000F0000) >> 16;
	unsigned int decimal = (arg & 0x0F000000) >> 24;
	int place = 0;
	unsigned int led_mask = 0x00000001;
	unsigned int num_mask = 0x0000000F;
	int i;
	unsigned char send;
	int index = 0;
	
	if(led_flag != 0x02)				// flag check
		return -1;
	// set first 2 bytes
	buf[0] = MTCP_LED_SET;
	buf[1] = leds;
	//printk("leds: %x \n", buf[1]);
	// fill the rest of buffer arguments
	
	for(i = 0; i < 4; i++)
	{
		/* printk("leds %x\n", leds);
		printk("led_mask %x\n", led_mask); */
		if(led_mask & leds) // check if led is active
		{
			index = num_mask & num;
			if(decimal & led_mask) // check if decimal is active
			{
				
				send = (convert[index] | 0x10); // activate decimal point bit
				//printk("adding decimals!! \n");
			}
			else
			{
				//printk("index: %x \n", index);
				send = convert[index];
				//printk("nope decimals \n");
			}
			index = num_mask & num;
			buf[2+place] = send;		// add to buf
			
			place++;
		}
		led_mask = led_mask << 1;									// shift masks
		num = num >> 4;
	}
	led_flag = 0x03;
	tuxctl_ldisc_put(tty, buf, 6);					// send to tux
	//printk("Sent LED stuff to Tux...\n");
	return 0;
}

int tux_but(struct tty_struct* tty, unsigned long arg)
{
	int * ptr = (int*)arg;
	//printk("%x \n", buttons_status);
	if(ptr==NULL)
		return -EINVAL;
	if(copy_to_user(ptr, &buttons_status, 1) != 0) 			// send button info to user space
		return -EINVAL;
	return 0;
	
}
