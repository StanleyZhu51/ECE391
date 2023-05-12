/* 
 * tab:4
 *
 * modex.h - header file for mode X 320x200 graphics
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 23:08:21 2004
 * Filename:      modex.h
 * History:
 *    SL    1    Thu Sep  9 23:08:21 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:35:41 2009
 *        Integrated original release back into main code base.
 */

#ifndef MODEX_H
#define MODEX_H

#include "text.h"

/* 
 * IMAGE  is the whole screen in mode X: 320x200 pixels in our flavor.
 * SCROLL is the scrolling region of the screen.
 *
 * X_DIM   is a horizontal screen dimension in pixels.
 * X_WIDTH is a horizontal screen dimension in 'natural' units
 *         (addresses, characters of text, etc.)
 * Y_DIM   is a vertical screen dimension in pixels.
 */
#define IMAGE_X_DIM     320   /* pixels; must be divisible by 4             */
#define IMAGE_Y_DIM     200-18   /* pixels                                     */
#define IMAGE_X_WIDTH   (IMAGE_X_DIM / 4)          /* addresses (bytes)     */
#define SCROLL_X_DIM    IMAGE_X_DIM                /* full image width      */
#define SCROLL_Y_DIM    IMAGE_Y_DIM                /* full image width      */
#define SCROLL_X_WIDTH  (IMAGE_X_DIM / 4)          /* addresses (bytes)     */

/*
 * NOTES
 *
 * Mode X is an (originally) undocumented variant of mode 13h, the first
 * IBM 256-color graphics mode.  Each pixel uses a single byte to specify
 * one of the 256 possible colors, and a palette is used to map each color
 * into an 18-bit space (6-bit red, green, and blue intensities).
 *
 * The map from memory as seen by the host processor is non-linear, and was
 * originally designed to allow high-performance hardware designs.  Video
 * memory is in words of 32 bits, and is divided into four planes.  In mode
 * X, groups of four pixels are mapped into a single host address and written
 * individually or together by setting a bit mask of planes to be written
 * (a VGA register).
 *
 * each four pixels counts as one (one-byte) address ->
 * 0123012301230123012301230123012301230123012301230123012301230123
 *
 * The mapping is more contorted than with mode 13h, but allows use of
 * graphics tricks such as double-buffering.
 *
 * The design here is strongly influenced by the fact that we are running
 * in a virtual machine in which writes to video memory are exorbitantly
 * expensive.  In particular, writing a chunk of 16kB with a single x86
 * instruction (REP MOVSB) is much faster than writing two hundred bytes
 * with many instructions.
 *
 * That said, the design is not unreasonable, and is only slightly different
 * than was (and is) often used today.
 *
 * Double-buffering uses two sections of memory to allow a program to
 * draw the next screen to be displayed without having the partially drawn
 * screen visible on the monitor (which causes flicker).  When the drawing
 * is complete, the video adapter is told to display the new screen instead
 * of the old one, and the memory used for the old screen is then used to
 * draw a third screen, the video adapter is switched back, and the process
 * starts again.
 *
 * In our variant of double-buffering, we use non-video memory as the
 * scratch pad, copy the drawn screen as a whole into one of two buffers
 * in video memory, and switch the picture between the two buffers.  The
 * cost of the copy is negligible; the cost of writing to video memory
 * instead is quite high (under VirtualPC).
 *
 * In order to reduce drawing time, we reuse most of the screen data between
 * video frames.  New data are drawn only when the viewing window moves
 * within a logical space defined by the program.  For example, if this
 * window shifts one pixel to the left, only the left border of the screen
 * is drawn.  Other data are left untouched in most cases.
 */

/* configure VGA for mode X; initializes logical view to (0,0) */
#define FRUIT_SIZE 7
#define STRING_SIZE 10

//unsigned char fruit_strings[FRUIT_SIZE][STRING_SIZE] = {{"Wow Apple!"}, {"Wow Grape!"}, {"Wow Peach!"}, {"Strawberry"}, {"Wow Banana"}, {"Watermelon"}, {"Great Dew!"}};
//extern unsigned char string_mask[FRUIT_SIZE*STRING_SIZE];


extern int set_mode_X(
        void (*horiz_fill_fn)(int, int, unsigned char[SCROLL_X_DIM]),
        void (*vert_fill_fn)(int, int, unsigned char[SCROLL_Y_DIM]));

/* return to text mode */
extern void clear_mode_X();

/* set logical view window coordinates */
extern void set_view_window(int scr_x, int scr_y);

extern void status_bar(unsigned int level, unsigned int fruit, unsigned int min, unsigned int sec);
/* show the logical view window on the monitor */
extern void show_screen();

/* clear the video memory in mode X */
extern void clear_screens();

/* changes a specific palette */
extern void set_pal(unsigned char pal, int num);

/* draws the transparent text */
extern void draw_trans_block(int pos_x, int pos_y, unsigned char* blk, int color, unsigned char * mask);

/* copyies background of the transparent text */
extern void copy_trans(int pos_x, int pos_y, unsigned char * blk); 

/* copies background underneath player*/
extern void copy_background(int pos_x, int pos_y, unsigned char * buf);

/* draws block using player mask */
extern void draw_player_mask(int pos_x, int pos_y, unsigned char * blk, unsigned char * pmask); 


/*
 * draw a 12x12 block with upper left corner at logical position
 * (pos_x,pos_y); any part of the block outside of the logical view window
 * is clipped (cut off and not drawn)
 */
extern void draw_full_block(int pos_x, int pos_y, unsigned char* blk);

/* draw a horizontal line at vertical pixel y within the logical view window */
extern int draw_horiz_line(int y);

/* draw a vertical line at horizontal pixel x within the logical view window */
extern int draw_vert_line(int x);

#endif /* MODEX_H */
