/* KallistiOS ##version##

   cdrom.c

   Copyright (C) 2000 Dan Potter

 */

/* Imported from KOS with slight modifications so that it compiles without the
   rest of KOS. Also, some parts that aren't needed for the patcher have been
   stripped out.

   Basically, everything related to threads and synchronization is gone, as is
   anything related to CDDA. */

#ifndef NULL
#define NULL ((void*)0)
#endif

#include "cdrom.h"

/*

This module contains low-level primitives for accessing the CD-Rom (I
refer to it as a CD-Rom and not a GD-Rom, because this code will not
access the GD area, by design). Whenever a file is accessed and a new
disc is inserted, it reads the TOC for the disc in the drive and
gets everything situated. After that it will read raw sectors from
the data track on a standard DC bootable CDR (one audio track plus
one data track in xa1 format).

Most of the information/algorithms in this file are thanks to
Marcus Comstedt. Thanks to Maiwe for the verbose command names and
also for the CDDA playback routines.

Note that these functions may be affected by changing compiler options...
they require their parameters to be in certain registers, which is
normally the case with the default options. If in doubt, disassemble the
output and look to make sure.

*/


/* GD-Rom BIOS calls... named mostly after Marcus' code. None have more
   than two parameters; R7 (fourth parameter) needs to describe
   which syscall we want. */

#define MAKE_SYSCALL(rs, p1, p2, idx) \
    unsigned int *syscall_bc = (unsigned int*)0x8c0000bc; \
    int (*syscall)() = (int (*)())(*syscall_bc); \
    rs syscall((p1), (p2), 0, (idx));

/* Reset system functions */
static void gdc_init_system() {
    MAKE_SYSCALL(/**/, 0, 0, 3);
}

/* Submit a command to the system */
static int gdc_req_cmd(int cmd, void *param) {
    MAKE_SYSCALL(return, cmd, param, 0);
}

/* Check status on an executed command */
static int gdc_get_cmd_stat(int f, void *status) {
    MAKE_SYSCALL(return, f, status, 1);
}

/* Execute submitted commands */
static void gdc_exec_server() {
    MAKE_SYSCALL(/**/, 0, 0, 2);
}

/* Check drive status and get disc type */
static int gdc_get_drv_stat(void *param) {
    MAKE_SYSCALL(return, param, 0, 4);
}

/* Set disc access mode */
static int gdc_change_data_type(void *param) {
    MAKE_SYSCALL(return, param, 0, 10);
}

/* These two functions are never actually used in here. They're only really here
   for reference at this point. */
#if 0
/* Reset the GD-ROM */
static void gdc_reset() {
    MAKE_SYSCALL(/**/, 0, 0, 9);
}

/* Abort the current command */
static void gdc_abort_cmd(int cmd) {
    MAKE_SYSCALL(/**/, cmd, 0, 8);
}
#endif

/* The CD access mutex */
static int initted = 0;
static int sector_size = 2048;   /* default 2048, 2352 for raw data reading */

/* Command execution sequence */
int cdrom_exec_cmd(int cmd, void *param) {
    int status[4] = {0};
    int f, n;

    /* Submit the command and wait for it to finish */
    f = gdc_req_cmd(cmd, param);

    do {
        gdc_exec_server();
        n = gdc_get_cmd_stat(f, status);
    }
    while(n == PROCESSING);

    if(n == COMPLETED)
        return ERR_OK;
    else if(n == ABORTED)
        return ERR_ABORTED;
    else if(n == NO_ACTIVE)
        return ERR_NO_ACTIVE;
    else {
        switch(status[0]) {
            case 2:
                return ERR_NO_DISC;
            case 6:
                return ERR_DISC_CHG;
            default:
                return ERR_SYS;
        }
    }
}

/* Return the status of the drive as two integers (see constants) */
int cdrom_get_status(int *status, int *disc_type) {
    int     rv = ERR_OK;
    unsigned int  params[2];

    rv = gdc_get_drv_stat(params);

    if(rv >= 0) {
        if(status != NULL)
            *status = params[0];

        if(disc_type != NULL)
            *disc_type = params[1];
    }
    else {
        if(status != NULL)
            *status = -1;

        if(disc_type != NULL)
            *disc_type = -1;
    }

    return rv;
}

/* Re-init the drive, e.g., after a disc change, etc */
int cdrom_reinit() {
    int rv = ERR_OK;
    int r = -1, cdxa;
    unsigned int  params[4];
    int timeout;

    /* Try a few times; it might be busy. If it's still busy
       after this loop then it's probably really dead. */
    timeout = 10 * 1000;

    while(timeout > 0) {
        r = cdrom_exec_cmd(CMD_INIT, NULL);

        if(r == 0) break;

        if(r == ERR_NO_DISC) {
            rv = r;
            goto exit;
        }
        else if(r == ERR_SYS) {
            rv = r;
            goto exit;
        }

        timeout--;
    }

    if(timeout <= 0) {
        rv = r;
        goto exit;
    }

    /* Check disc type and set parameters */
    gdc_get_drv_stat(params);
    cdxa = params[1] == 32;
    params[0] = 0;                      /* 0 = set, 1 = get */
    params[1] = 8192;                   /* ? */
    params[2] = cdxa ? 2048 : 1024;     /* CD-XA mode 1/2 */
    params[3] = sector_size;            /* sector size */

    if(gdc_change_data_type(params) < 0) {
        rv = ERR_SYS;
        goto exit;
    }

exit:
    return rv;
}

/* Read the table of contents */
int cdrom_read_toc(CDROM_TOC *toc_buffer, int session) {
    struct {
        int session;
        void    *buffer;
    } params;
    int rv;

    params.session = session;
    params.buffer = toc_buffer;
    rv = cdrom_exec_cmd(CMD_GETTOC2, &params);

    return rv;
}

/* Initialize: assume no threading issues */
int cdrom_init() {
    unsigned int p;
    volatile unsigned int *react = (unsigned int*)0xa05f74e4,
                     *bios = (unsigned int*)0xa0000000;

    if(initted)
        return -1;

    /* Reactivate drive: send the BIOS size and then read each
       word across the bus so the controller can verify it. */
    *react = 0x1fffff;

    for(p = 0; p < 0x200000 / 4; p++) {
        (void)bios[p];
    }

    /* Reset system functions */
    gdc_init_system();

    /* Do an initial initialization */
    cdrom_reinit();

    return 0;
}

void cdrom_shutdown() {
    if(!initted)
        return;

    initted = 0;
}
