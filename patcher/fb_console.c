/* KallistiOS ##version##

   util/fb_console.c
   Copyright (C) 2009, 2013 Lawrence Sebald

*/

#include <stdarg.h>
#include "biosfont.h"
#include "video.h"

/* This is a very simple dbgio interface for doing debug to the framebuffer with
   the biosfont functionality. Basically, this was written to aid in debugging
   the network stack, and I figured other people would probably get some use out
   of it as well. */

static uint16 *fb;
static int fb_w, fb_h;
static int cur_x, cur_y;
static int min_x, min_y, max_x, max_y;

/* I don't think this is in a header anywhere any more. */
extern void *memcpy2(void *dest, const void *src, uint32 count);
extern void *memset2(void *s, unsigned short c, uint32 count);

/* Bleh. */
int vsprintf(char *s, const char *format, va_list ap);

#define FONT_CHAR_WIDTH 12
#define FONT_CHAR_HEIGHT 24

int fb_init() {
    bfont_set_encoding(BFONT_CODE_ISO8859_1);

    /* Assume we're using 640x480x16bpp */
    fb = NULL;
    fb_w = 640;
    fb_h = 480;
    min_x = 32;
    min_y = 32;
    max_x = 608;
    max_y = 448;
    cur_x = 32;
    cur_y = 32;

    return 0;
}

static int fb_write(int c) {
    uint16 *t = fb;

    if(!t)
        t = vram_s;

    if(c != '\n') {
        bfont_draw(t + cur_y * fb_w + cur_x, fb_w, 0, c);
        cur_x += FONT_CHAR_WIDTH;
    }

    /* If we have a newline or we've gone past the end of the line, advance down
       one line. */
    if(c == '\n' || cur_x + FONT_CHAR_WIDTH > max_x) {
        cur_y += FONT_CHAR_HEIGHT;
        cur_x = min_x;

        /* If going down a line put us over the edge of the screen, move
           everything up a line, fixing the problem. */
        if(cur_y + FONT_CHAR_HEIGHT > max_y) {
            memcpy2(t + min_y * fb_w, t + (min_y + FONT_CHAR_HEIGHT) * fb_w,
                    (cur_y - min_y - FONT_CHAR_HEIGHT) * fb_w * 2);
            cur_y -= FONT_CHAR_HEIGHT;
            memset2(t + cur_y * fb_w, 0, FONT_CHAR_HEIGHT * fb_w * 2);
        }
    }

    return 1;
}

int fb_write_string(const char *data) {
    int rv = 0;

    while(*data) {
        fb_write((int)(*data++));
        ++rv;
    }

    return rv;
}

int fb_write_hex(uint32 val) {
    int shift = 28;
    const char hexes[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    fb_write('0');
    fb_write('x');
    while(shift >= 0) {
        fb_write(hexes[(val >> shift) & 0x0F]);
        shift -= 4;
    }

    return 10;
}
