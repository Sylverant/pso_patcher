/* GDEMU SDK

   Copyright (C) 2019 megavolt85

   KallistiOS ##version##
   hardware/g1ata.c

   Copyright (C) 2013, 2014, 2015 Lawrence Sebald
*/

#include "gdemu.h"
#include "utils.h"

#define ATAPI_CMD_PACKET        0xA0

/* ATA-related registers. Some of these serve very different purposes when read
   than they do when written (hence why some addresses are duplicated). */
#define G1_ATA_ALTSTATUS        0xA05F7018      /* Read */
#define G1_ATA_DATA             0xA05F7080      /* Read/Write */
#define G1_ATA_STATUS_REG       0xA05F709C      /* Read */
#define G1_ATA_COMMAND_REG      0xA05F709C      /* Write */

/* status of the external interrupts
 * bit 3 = External Device interrupt
 * bit 2 = Modem interrupt
 * bit 1 = AICA interrupt
 * bit 0 = GD-ROM interrupt */
#define EXT_INT_STAT            0xA05F6904      /* Read */

/* Bitmasks for the STATUS_REG/ALT_STATUS registers. */
#define G1_ATA_SR_ERR   0x01
#define G1_ATA_SR_DRQ   0x08
#define G1_ATA_SR_BSY   0x80

/* Macros to access the ATA registers */
#define OUT16(addr, data) *((volatile uint16 *)addr) = data
#define OUT8(addr, data)  *((volatile uint8  *)addr) = data
#define IN32(addr)        *((volatile uint32 *)addr)
#define IN8(addr)         *((volatile uint8  *)addr)

#define g1_ata_wait_interrupt() \
    do {} while(!(IN32(EXT_INT_STAT) & 1));

#define g1_ata_wait_status(n) \
    do {} while((IN8(G1_ATA_ALTSTATUS) & (n)))

#define g1_ata_wait_nstatus(n) \
    do {} while(!(IN8(G1_ATA_ALTSTATUS) & (n)))

#define g1_ata_wait_drq() g1_ata_wait_nstatus(G1_ATA_SR_DRQ)

#define g1_ata_wait_bsydrq() g1_ata_wait_status(G1_ATA_SR_DRQ | G1_ATA_SR_BSY)

static int send_packet_command(uint16 *cmd_buff)
{
    g1_ata_wait_bsydrq();
    OUT8(G1_ATA_COMMAND_REG, ATAPI_CMD_PACKET);
    g1_ata_wait_drq();

    (void) IN32(G1_ATA_STATUS_REG);
    int i;
    for (i = 0; i < 6; i++) {
        OUT16(G1_ATA_DATA, cmd_buff[i]);
    }

    g1_ata_wait_interrupt();

    (void) IN32(G1_ATA_STATUS_REG);

    return (IN8(G1_ATA_ALTSTATUS) & G1_ATA_SR_ERR);
}

/* param = 0x55 next img */
/* param = 0x44 prev img */
int gdemu_img_cmd(uint8 cmd)
{
    uint8 cmd_buff[12];
    memset(cmd_buff, 0, 12);

    cmd_buff[0] = 0x52;
    cmd_buff[1] = 0x81;
    cmd_buff[2] = cmd;

    return send_packet_command((uint16 *) cmd_buff);
}
