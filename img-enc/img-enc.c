/*
    This file is part of Sylverant PSO Patcher
    Copyright (C) 2013 Lawrence Sebald

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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static uint32_t input_buf[640 * 480];
static uint16_t xform_buf[640 * 480];

#define XRGB8888_TO_RGB565(x) \
    (((x & 0xF8000000) >> 16) | \
     ((x & 0x00FC0000) >> 13) | \
     ((x & 0x0000F800) >> 11))

#ifdef __BIG_ENDIAN__
#define LE16(x) ((x >> 8) & 0xFF) | ((x & 0xFF) << 8)
#else
#define LE16(x) x
#endif

int main(int argc, char *argv[]) {
    FILE *fp;
    int i, runlen = 0;
    uint16_t tmp;

    if(argc != 3) {
        printf("Usage: %s input output\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!(fp = fopen(argv[1], "rb"))) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    if(fread(input_buf, 1, 640 * 480 * 4, fp) != 640 * 480 * 4) {
        printf("Input data malformed!\n");
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    for(i = 0; i < 640 * 480; ++i) {
        xform_buf[i] = LE16((uint16_t)XRGB8888_TO_RGB565(ntohl(input_buf[i])));
    }

    if(!(fp = fopen(argv[2], "wb"))) {
        perror("Cannot open output file");
        exit(EXIT_FAILURE);
    }

    i = 1;
    runlen = 0;
    tmp = xform_buf[0];

    while(i < 640 * 480) {
        /* Do we still have a run of whatever we're looking at? */
        if(runlen < 65535 && tmp == xform_buf[i]) {
            ++runlen;
            ++i;
            continue;
        }

        /* Looks like the current run is done, write it out. */
        runlen = LE16(runlen);
        fwrite(&runlen, 1, 2, fp);
        fwrite(&tmp, 1, 2, fp);

        tmp = xform_buf[i++];
        runlen = 1;
    }

    /* Write out the last run. */
    runlen = LE16(runlen);
    fwrite(&runlen, 1, 2, fp);
    fwrite(&tmp, 1, 2, fp);

    fclose(fp);

    return 0;
}
