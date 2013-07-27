/*
    This file is part of Sylverant PSO Patcher
    Copyright (C) 2011, 2012, 2013 Lawrence Sebald

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by  the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cdfs.h"
#include "cdrom.h"
#include "gd.h"
#include "patches.h"
#include "video.h"
#include "maple.h"
#include "utils.h"

#ifndef PLANET_RING
#include "psobg.h"
#else
#include "prbg.h"
#endif

#define BIN_BASE    0xac010000
#define IP_BASE     0xac008000
#define SYS_BASE    0x8c008000
#define IP_LEN      32768
#define MAP_TABLE   0xac0081fc
#define MAP_NAMES   0xac0082bc

extern unsigned long end;
static uint8 *ip_bin = (uint8 *)IP_BASE;
static uint8 *bin = (uint8 *)BIN_BASE;

extern int fs_iso9660_gd_init();

/* GD-ROM syscall replacement. All of these extern symbols are defined in
   patch.s */
extern uint32 gd_syscall(uint32, uint32, uint32, uint32);
extern uint32 patch_trigger_addr;
extern uint32 patch_trigger_pattern;
extern uint32 old_gd_vector;
extern uint32 patches_count;
extern uint32 patches[18];
extern uint32 server_addr;
extern uint32 gd_syscall_len;
extern uint8 patches_enabled;
extern void boot_stub(void *, uint32) __attribute__((noreturn));
extern uint32 boot_stub_len;

uint32 *gd_vector_addr = (uint32 *)0xac0000bc;

/* Dummy stub to make libgcc happy... */
void atexit() { }

/* Calculate a CRC-32 checksum over a given block of data. Somewhat inspired by
   the CRC32 function in Figure 14-6 of http://www.hackersdelight.org/crc.pdf */
uint32 crc32(const uint8 *data, int size) {
    int i;
    uint32 rv = 0xFFFFFFFF;

    for(i = 0; i < size; ++i) {
        rv ^= data[i];
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
        rv = (0xEDB88320 & (-(rv & 1))) ^(rv >> 1);
    }

    return ~rv;
}

static void draw_bg(void) {
    int i;
    uint16 *o = vram_s, *input = (uint16 *)bg_data;
    uint16 *end = (uint16 *)(bg_data + bg_size);

    /* Decode the RLE'd data into the framebuffer. */
    while(input < end) {
        for(i = 0; i < *input; ++i) {
            *o++ = *(input + 1);
        }

        input += 2;
    }
}

/* Find the right version of the game to patch. */
static pso_disc_t *find_disc(uint32 ipcrc, uint32 bincrc) {
    int i;
    pso_disc_t *rv = (pso_disc_t *)0;

    for(i = 0; i < NUM_PSO_DISCS; ++i) {
        if(discs[i].crc32_ip == ipcrc && discs[i].crc32_bin == bincrc) {
            rv = &discs[i];
        }
    }

    return rv;
}

static void wait_for_disc() {
    int status, type = 0;

    while(type != CD_GDROM) {
        cdrom_get_status(&status, &type);
    }

    /* Mark that we found a disc (preliminarily... we'll clear this later if the
       disc isn't what we're looking for). */
    patches_enabled = 1;
}

uint32 gd_locate_data_track(CDROM_TOC *toc) {
    int i, first, last;

    first = TOC_TRACK(toc->first);
    last = TOC_TRACK(toc->last);

    if(first < 1 || last > 99 || first > last) {
        /* Guess that its the first High Density area track... */
        return 45150;
    }

    for(i = first; i <= last; ++i) {
        if(TOC_CTRL(toc->entry[i - 1]) == 4) {
            return TOC_LBA(toc->entry[i - 1]);
        }
    }

    /* Punt. */
    return 45150;
}

/* The next 3 functions are borrowed (with minor changes) from Marcus' old maple
   demo program. The last one wasn't a function in Marcus' old code, but rather
   sat right near the end of main(). */
unsigned int read_belong(unsigned int *l) {
    unsigned char *b = (unsigned char *)l;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

void write_belong(unsigned int *l, unsigned int v) {
    unsigned char *b = (unsigned char *)l;
    b[0] = v >> 24;
    b[1] = v >> 16;
    b[2] = v >> 8;
    b[3] = v;
}

/* Borrowed from KOS... */
#define CONT_C              (1<<0)
#define CONT_B              (1<<1)
#define CONT_A              (1<<2)
#define CONT_START          (1<<3)
#define CONT_DPAD_UP        (1<<4)
#define CONT_DPAD_DOWN      (1<<5)
#define CONT_DPAD_LEFT      (1<<6)
#define CONT_DPAD_RIGHT     (1<<7)
#define CONT_Z              (1<<8)
#define CONT_Y              (1<<9)
#define CONT_X              (1<<10)
#define CONT_D              (1<<11)
#define CONT_DPAD2_UP       (1<<12)
#define CONT_DPAD2_DOWN     (1<<13)
#define CONT_DPAD2_LEFT     (1<<14)
#define CONT_DPAD2_RIGHT    (1<<15)

static uint16 wait_for_buttons(uint16 mask) {
    int port;
    char *res;
    unsigned int params[1];
    uint16 buttons;

    for(;;) {
        for(port = 0; port < 4; port++) {
            /* Query controller condition (non-controller will return error) */
            write_belong(&params[0], MAPLE_FUNC_CONTROLLER);

            do {
                res = maple_docmd(port, 0, MAPLE_COMMAND_GETCOND, 1, params);
            } while(*res == MAPLE_RESPONSE_AGAIN);

            if(*res == MAPLE_RESPONSE_DATATRF && res[3] >= 2 &&
               read_belong((unsigned int *)(res + 4)) == MAPLE_FUNC_CONTROLLER) {
                buttons = ~(*(uint16 *)(res + 8));

                if(buttons & mask)
                    return buttons & mask;
            }
        }
    }
}

#define wait_for_start() wait_for_buttons(CONT_START)

/* Framebuffer printf with transparent backgrounds... */
int fb_init();
int fb_write_string(const char *data);
int fb_write_hex(uint32 val);

void dcache_flush_range(uint32 base, uint32 len);

int main(int argc, char *argv[]) {
    CDROM_TOC toc;
    uint32 sz = 408, data_fad;
    int i, fd, cur = 0, rsz;
    char filename[17];
    uint32 ipcrc, crc;
    pso_disc_t *disc;
    int rv = -1;

#ifndef PLANET_RING
    const uint32 *ptbl;
    uint16 buttons;
#endif

    (void)argc;
    (void)argv;

    vid_init(DM_640x480, PM_RGB565);
    cdrom_init();
    maple_init();

restart:
    fb_init();
    draw_bg();

#ifndef PLANET_RING
    fb_write_string("Sylverant PSO Patcher v2.0\n"
                    "Copyright (C) 2011-2013 Lawrence Sebald\n\n");
#else
    fb_write_string("Sylverant Planet Ring Patcher v2.0\n"
                    "Copyright (C) 2011-2013 Lawrence Sebald\n\n");
#endif

    /* Wait for the user to insert a GD-ROM */
    fb_write_string("Please insert a GD-ROM...\n");
    wait_for_disc();

    fb_write_string("Please wait while the disc is read...\n");

    /* Reinitialize the drive */
    do {
        rv = cdrom_reinit();
    } while(rv != ERR_OK);

    gd_read_toc((uint16 *)&toc, &sz);

    /* Figure out where IP.BIN should be... */
    data_fad = gd_locate_data_track(&toc);

    fb_write_string("Reading IP.BIN...\n");

    /* Attempt to read in IP.BIN */
    for(i = 0; i < 16; ++i) {
        sz = 2048;
        gd_read_sector(data_fad + i, (uint16 *)(ip_bin + i * 2048), &sz);
    }

    /* Figure out what the boot file is called */
    memcpy(filename, (char *)ip_bin + 0x60, 16);
    filename[16] = 0;

    /* Remove any spaces at the end of the filename */
    for(i = 0; i < 16; ++i) {
        if(filename[i] == ' ') {
            filename[i] = 0;
            break;
        }
    }

    /* Grab the CRC of IP.BIN */
    ipcrc = crc32(ip_bin, IP_LEN);

    /* See if the binary is a WinCE binary or not... Unfortunately, I still
       can't seem to get WinCE games to boot... */
    if(hex_to_uint32((char *)ip_bin + 0x38, 7) & 1) {
        fb_write_string("Windows CE based games are not supported!\n"
                        "Please remove the GD-ROM, insert a supported\n"
                        "game and press START.\n");
        wait_for_start();
        patches_enabled = 0;
        goto restart;
    }

    fb_write_string("Reading ");
    fb_write_string(filename);
    fb_write_string("...\n");

    /* Read the binary in. This reads directly into the correct address. */
    if((fd = open(filename, O_RDONLY)) < 0)
        return -1;

    while((rsz = read(fd, bin + cur, 2048)) > 0) {
        cur += rsz;
    }

    close(fd);

    /* Grab the CRC of the binary */
    crc = crc32(bin, cur);

    /* Find the appropriate patch set... */
    disc = find_disc(ipcrc, crc);

    if(!disc) {
#ifndef PLANET_RING
        fb_write_string("Inserted disc is unknown...\n"
                        "If it is PSO, please report the CRCs below\n"
                        "along with the version of PSO in use.\n"
                        "IP.BIN CRC: ");
        fb_write_hex(ipcrc);
        fb_write_string("\n");
        fb_write_string(filename);
        fb_write_string(" CRC = ");
        fb_write_hex(crc);
        fb_write_string("\n\nPress START to load anyway.\n");
#else
        fb_write_string("Inserted disc is unknown...\n"
                        "If it is Planet Ring, please report the\n"
                        "CRCs below.\n"
                        "IP.BIN CRC: ");
        fb_write_hex(ipcrc);
        fb_write_string("\n");
        fb_write_string(filename);
        fb_write_string(" CRC = ");
        fb_write_hex(crc);
        fb_write_string("\n\nPress START to load anyway.\n");
#endif

        wait_for_start();
        patches_enabled = 0;
    }
    else {
        /* Copy the GD-ROM syscall replacement... We can use from 0x8C008000 - 
           0x8C0082FF (768 bytes) without making PSO angry.
           Not a lot of space, but it should be enough. */
        /* First fill in all the variables that it needs... */
        patch_trigger_addr = disc->patch_trigger_addr;
        patch_trigger_pattern = disc->patch_trigger_pattern;
        old_gd_vector = *gd_vector_addr;
        server_addr = disc->server_addr;

        /* Wait for the user to hit start. */
        fb_write_string("Detected game:\n");
        fb_write_string(disc->name);

#ifdef PLANET_RING
        fb_write_string("\nPress START to load and patch!\n");
        wait_for_start();
#else
        /* If we're looking at PSOv2, there's optional patches. */
        if(disc->index >= 3 && disc->index <= 5) {
            fb_write_string("\nPress START to load with all patches or\n"
                            "press A to load without the battle stage\n"
                            "music patch.\n");
            buttons = wait_for_buttons(CONT_START | CONT_A);

            if(buttons & CONT_START)
                ptbl = patch_tables2[disc->index];
            else
                ptbl = patch_tables[disc->index];
        }
        else {
            fb_write_string("\nPress START to load and patch!\n");
            wait_for_start();
            ptbl = patch_tables[disc->index];
        }
#endif

#ifndef PLANET_RING
        /* Copy the patches over, now that we know what we're copying... */
        memcpy(&patches_count, ptbl, sizeof(uint32) * 23);
#endif

        /* Copy the syscall on to where it goes, and patch the address in the
           syscalls table. */
        memcpy((void *)SYS_BASE, gd_syscall, gd_syscall_len);
        *gd_vector_addr = SYS_BASE;

#ifndef PLANET_RING
        /* Copy the map table stuff, as appropriate. */
        memcpy((void *)MAP_TABLE, map_ptrs[disc->index], sizeof(uint32) * 48);
        memcpy((void *)MAP_NAMES, map_names, 68);
#endif        

        dcache_flush_range(SYS_BASE, 768);
    }

    /* The binary is in place, so let's try to boot it, shall we? */
    void (*f)(void) __attribute__((noreturn));
    f = (void *)((uint32)(&boot_stub) | 0xa0000000);
    f();
}
