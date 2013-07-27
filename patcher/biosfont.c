/* KallistiOS ##version##

   biosfont.c

   Copyright (C) 2000-2002 Dan Potter
   Japanese code Copyright (C) Kazuaki Matsumoto
 */

#include "biosfont.h"

/*

This module handles interfacing to the bios font. It supports the standard
European encodings via ISO8859-1, and Japanese in both Shift-JIS and EUC
modes. For Windows/Cygwin users, you'll probably want to call
bfont_set_encoding(BFONT_CODE_SJIS) so that your messages are displayed
properly; otherwise it will default to EUC (for *nix).

Thanks to Marcus Comstedt for the bios font information.

All the Japanese code is by Kazuaki Matsumoto.

Foreground/background color switching based on code by Chilly Willy.

*/

/* Our current conversion mode */
static int bfont_code_mode = BFONT_CODE_ISO8859_1;

/* Current colors/pixel format. Default to white foreground, black background
   and 16-bit drawing, so the default behavior doesn't change from what it has
   been forever. */
static uint32 bfont_fgcolor = 0xFFFFFFFF;
static uint32 bfont_bgcolor = 0x00000000;
static int bfont_32bit = 0;

/* Select an encoding for Japanese (or disable) */
void bfont_set_encoding(int enc) {
    if(enc < BFONT_CODE_ISO8859_1 || enc > BFONT_CODE_SJIS) {
        return;
    }

    bfont_code_mode = enc;
}

/* Set the foreground color and return the old color */
uint32 bfont_set_foreground_color(uint32 c) {
    uint32 rv = bfont_fgcolor;
    bfont_fgcolor = c;
    return rv;
}

/* Set the background color and return the old color */
uint32 bfont_set_background_color(uint32 c) {
    uint32 rv = bfont_bgcolor;
    bfont_bgcolor = c;
    return rv;
}

/* Set the font to draw in 32 or 16 bit mode */
int bfont_set_32bit_mode(int on) {
    int rv = bfont_32bit;
    bfont_32bit = !!on;
    return rv;
}

/* A little assembly that grabs the font address */
extern uint8* get_font_address();
__asm__("	.text\n"
        "	.align 2\n"
        "_get_font_address:\n"
        "	mov.l	syscall_b4,r0\n"
        "	mov.l	@r0,r0\n"
        "	jmp	@r0\n"
        "	mov	#0,r1\n"
        "\n"
        "	.align 4\n"
        "syscall_b4:\n"
        "	.long	0x8c0000b4\n");


/* Shift-JIS -> JIS conversion */
unsigned int sjis2jis(unsigned int sjis) {
    unsigned int hib, lob;

    hib = (sjis >> 8) & 0xff;
    lob = sjis & 0xff;
    hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
    hib = (hib << 1) + 1;

    if(lob > 0x7f) lob--;

    if(lob >= 0x9e) {
        lob -= 0x7d;
        hib++;
    }
    else
        lob -= 0x1f;

    return (hib << 8) | lob;
}


/* EUC -> JIS conversion */
unsigned int euc2jis(unsigned int euc) {
    return euc & ~0x8080;
}

/* Given an ASCII character, find it in the BIOS font if possible */
uint8 *bfont_find_char(int ch) {
    int index = -1;
    uint8   *fa = get_font_address();

    /* 33-126 in ASCII are 1-94 in the font */
    if(ch >= 33 && ch <= 126)
        index = ch - 32;

    /* 160-255 in ASCII are 96-161 in the font */
    if(ch >= 160 && ch <= 255)
        index = ch - (160 - 96);

    /* Map anything else to a space */
    if(index == -1)
        index = 72 << 2;

    return fa + index * 36;
}

/* JIS -> (kuten) -> address conversion */
uint8 *bfont_find_char_jp(int ch) {
    uint8   *fa = get_font_address();
    int ku, ten, kuten = 0;

    /* Do the requested code conversion */
    switch(bfont_code_mode) {
        case BFONT_CODE_ISO8859_1:
            return NULL;
        case BFONT_CODE_EUC:
            ch = euc2jis(ch);
            break;
        case BFONT_CODE_SJIS:
            ch = sjis2jis(ch);
            break;
        default:
            return NULL;
    }

    if(ch > 0) {
        ku = ((ch >> 8) & 0x7F);
        ten = (ch & 0x7F);

        if(ku >= 0x30)
            ku -= 0x30 - 0x28;

        kuten = (ku - 0x21) * 94 + ten - 0x21;
    }

    return fa + (kuten + 144) * 72;
}


/* Half-width kana -> address conversion */
uint8 *bfont_find_char_jp_half(int ch) {
    uint8 *fa = get_font_address();
    return fa + (32 + ch) * 36;
}

/* Draw half-width kana */
void bfont_draw_thin(void *b, int bufwidth, int opaque, int c, int iskana) {
    uint8   *ch;
    uint16  word;
    int x, y;

    if(iskana)
        ch = bfont_find_char_jp_half(c);
    else
        ch = bfont_find_char(c);

    if(!bfont_32bit) {
        uint16 *buffer = (uint16 *)b;

        for(y = 0; y < 24;) {
            /* Do the first row */
            word = (((uint16)ch[0]) << 4) | ((ch[1] >> 4) & 0x0f);

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 12;
            y++;

            /* Do the second row */
            word = ((((uint16)ch[1]) << 8) & 0xf00) | ch[2];

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 12;
            y++;

            ch += 3;
        }
    }
    else {
        uint32 *buffer = (uint32 *)b;

        for(y = 0; y < 24;) {
            /* Do the first row */
            word = (((uint16)ch[0]) << 4) | ((ch[1] >> 4) & 0x0f);

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 12;
            y++;

            /* Do the second row */
            word = ((((uint16)ch[1]) << 8) & 0xf00) | ch[2];

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 12;
            y++;

            ch += 3;
        }
    }
}

/* Compat function */
void bfont_draw(void *buffer, int bufwidth, int opaque, int c) {
    bfont_draw_thin(buffer, bufwidth, opaque, c, 0);
}

/* Draw wide character */
void bfont_draw_wide(void *b, int bufwidth, int opaque, int c) {
    uint8   *ch = bfont_find_char_jp(c);
    uint16  word;
    int x, y;

    if(!bfont_32bit) {
        uint16 *buffer = (uint16 *)b;

        for(y = 0; y < 24;) {
            /* Do the first row */
            word = (((uint16)ch[0]) << 4) | ((ch[1] >> 4) & 0x0f);

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            word = ((((uint16)ch[1]) << 8) & 0xf00) | ch[2];

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 24;
            y++;

            ch += 3;
        }
    }
    else {
        uint32 *buffer = (uint32 *)b;

        for(y = 0; y < 24;) {
            /* Do the first row */
            word = (((uint16)ch[0]) << 4) | ((ch[1] >> 4) & 0x0f);

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            word = ((((uint16)ch[1]) << 8) & 0xf00) | ch[2];

            for(x = 0; x < 12; x++) {
                if(word & (0x0800 >> x))
                    *buffer = bfont_fgcolor;
                else {
                    if(opaque)
                        *buffer = bfont_bgcolor;
                }

                buffer++;
            }

            buffer += bufwidth - 24;
            y++;

            ch += 3;
        }
    }
}


/* Draw string of full-width (wide) and half-width (thin) characters
   Note that this handles the case of mixed encodings unless Japanese
   support is disabled (BFONT_CODE_ISO8859_1). */
void bfont_draw_str(void *b, int width, int opaque, char *str) {
    uint16 nChr, nMask, nFlag;
    int adv = bfont_32bit ? 48 : 24;    /* Amount to advance, in bytes */
    uint8 *buffer = (uint8 *)b;

    while(*str) {
        nFlag = 0;
        nChr = *str & 0xff;

        if(bfont_code_mode != BFONT_CODE_ISO8859_1 && (nChr & 0x80)) {
            switch(bfont_code_mode) {
                case BFONT_CODE_EUC:

                    if(nChr == 0x8e) {
                        str++;
                        nChr = *str & 0xff;

                        if((nChr < 0xa1) || (nChr > 0xdf))
                            nChr = 0xa0;    /* Blank Space */
                    }
                    else
                        nFlag = 1;

                    break;
                case BFONT_CODE_SJIS:
                    nMask = nChr & 0xf0;

                    if((nMask == 0x80) || (nMask == 0x90) || (nMask == 0xe0))
                        nFlag = 1;

                    break;
                default:
                    return;
            }

            if(nFlag == 1) {
                str++;
                nChr = (nChr << 8) | (*str & 0xff);
                bfont_draw_wide(buffer, width, opaque, nChr);
                buffer += adv + adv;
            }
            else {
                bfont_draw_thin(buffer, width, opaque, nChr, 1);
                buffer += adv;
            }
        }
        else {
            bfont_draw_thin(buffer, width, opaque, nChr, 0);
            buffer += adv;
        }

        str++;
    }
}

