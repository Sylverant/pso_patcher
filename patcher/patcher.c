/*
    This file is part of Sylverant PSO Patcher
    Copyright (C) 2011 Lawrence Sebald

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

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <kos/net.h>
#include <kos/dbgio.h>

#include <dc/cdrom.h>
#include <dc/maple.h>
#include <dc/sq.h>
#include <dc/maple/controller.h>

#include <arch/types.h>
#include <arch/exec.h>
#include <arch/cache.h>

#include <png/png.h>

#include "patches.h"
#include "gd.h"

#define BIN_BASE    0x8C010000
#define IP_BASE     0x8C008000
#define IP_LEN      32768
#define MAP_TABLE   0x8C0081FC
#define MAP_NAMES   0x8C0082BC

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

uint32 *gd_vector_addr = (uint32 *)0x8C0000BC;

/* Stuff for starting the game binary... */
typedef uint32 u32;
typedef void (*runfunc)(u32, u32, u32, u32) __attribute__((noreturn));
extern void clear_and_load(u32, u32, u32, u32);
extern uint32 _arch_old_sr, _arch_old_vbr, _arch_old_stack, _arch_old_fpscr;

/* Framebuffer printf with transparent backgrounds... */
int fb_init();
int fb_printf(const char *fmt, ...);

/* Find the right version of the game to patch. */
static pso_disc_t *find_disc(uint32 ipcrc, uint32 bincrc) {
    int i;
    pso_disc_t *rv = NULL;

    for(i = 0; i < NUM_PSO_DISCS; ++i) {
        if(discs[i].crc32_ip == ipcrc && discs[i].crc32_bin == bincrc) {
            rv = &discs[i];
        }
    }

    return rv;
}

static void wait_for_start() {
    maple_device_t *dev;
    cont_state_t *state;
    int i;

    for(;;) {
        i = 0;

        while((dev = maple_enum_type(i++, MAPLE_FUNC_CONTROLLER))) {
            state = (cont_state_t *)maple_dev_status(dev);

            if(state && (state->buttons & CONT_START)) {
                return;
            }
        }
    }
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

static void load_and_draw_bg() {
    kos_img_t img;

    if(png_to_img("/rd/bg.png", PNG_NO_ALPHA, &img)) {
        return;
    }

    /* Make sure the image is sane... */
    if(img.w != 640 || img.h != 480 || img.fmt != KOS_IMG_FMT_RGB565) {
        return;
    }

    /* Copy it over to the framebuffer */
    sq_cpy(vram_s, img.data, img.byte_count);

    kos_img_free(&img, 0);
}

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

int main(int argc, char *argv[]) {
    FILE *fp;
    uint32 len;
    uint32 crc, ipcrc;
    CDROM_TOC toc;
    int rv = -1, i;
    uint32 data_fad = 0, sz;
    pso_disc_t *disc;
    runfunc f;

    fb_init();
    load_and_draw_bg();

    fb_printf("Sylverant PSO Patcher v1.1\n"
              "Copyright (C) 2011 Lawrence Sebald\n\n");

    /* Wait for the user to insert a GD-ROM */
    fb_printf("Please insert a GD-ROM...\n");
    wait_for_disc();

    fb_printf("Please wait while the disc is read...\n");

    /* Reinitialize the drive */
    while(rv != ERR_OK) {
        rv = cdrom_reinit();
    }

    /* Read the TOC of the inserted disc */
    sz = 408;
    gd_read_toc((uint16 *)&toc, &sz);

    /* Figure out where IP.BIN should be... */
    data_fad = cdrom_locate_data_track(&toc);

    /* We'll need to read from the disc to grab 1ST_READ.BIN... */
    fs_iso9660_gd_init();

    fb_printf("Reading IP.BIN...\n");

    /* Attempt to read in IP.BIN */
    for(i = 0; i < 16; ++i) {
        sz = 2048;
        gd_read_sector(data_fad + i, (uint16 *)(ip_bin + (i * 2048)), &sz);
    }

    /* Grab the CRC of IP.BIN */
    ipcrc = net_crc32le(ip_bin, IP_LEN);
    fb_printf("Reading 1ST_READ.BIN...\n");

    fp = fopen("/gd/1ST_READ.BIN", "rb");
    if(!fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    len = (uint32)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    fread(bin, 1, (size_t)len, fp);
    fclose(fp);

    /* Flush the cache over the binary's space. */
    dcache_flush_range(0x8C010000, len);
    icache_flush_range(0x8C010000, len);

    /* Grab the CRC of the binary */
    crc = net_crc32le(bin, len);

    /* Find the appropriate patch set... */
    disc = find_disc(ipcrc, crc);

    if(!disc) {
        fb_printf("Inserted disc is unknown...\n"
               "If it is PSO, please report the CRCs shown below\n"
               "along with the version of PSO in use.\n"
               "IP.BIN CRC: %08x\n"
               "1ST_READ.BIN CRC = %08x\n\n"
               "Press START to load anyway.\n", (unsigned int)ipcrc,
               (unsigned int)crc);
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
        memcpy(&patches_count, patch_tables[disc->index], sizeof(uint32) * 19);

        /* Copy the syscall on to where it goes, and patch the address in the
           syscalls table. */
        memcpy((void *)IP_BASE, gd_syscall, gd_syscall_len);
        *gd_vector_addr = IP_BASE;

        /* Copy the map table stuff, as appropriate. */
        memcpy((void *)MAP_TABLE, map_ptrs[disc->index], sizeof(uint32) * 48);
        memcpy((void *)MAP_NAMES, map_names, 68);

        dcache_flush_range(IP_BASE, 768);
        icache_flush_range(IP_BASE, 768);
        dcache_flush_range(0x8C0000BC, 4);

        /* Wait for the user to hit start. */
        fb_printf("Detected game:\n%s\n"
               "Press START to load and patch!\n", disc->name);

        wait_for_start();
    }

    /* We're done, pass off control. */
    f = (runfunc)(0x8C008000 +
                  (((uint8 *)clear_and_load) - ((uint8 *)gd_syscall)));
    f(_arch_old_sr, _arch_old_vbr, _arch_old_fpscr, _arch_old_stack);
}
