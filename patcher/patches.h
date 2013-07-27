/*
    This file is part of Sylverant PSO Patcher
    Copyright (C) 2011, 2013 Lawrence Sebald

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

#ifndef PATCHES_H
#define PATCHES_H

#ifndef PLANET_RING
#define NUM_PSO_DISCS   7
#else
#define NUM_PSO_DISCS   1
#endif

/* Structure used to store the information about each game disc. */
typedef struct pso_disc {
    const char *name;
    unsigned int crc32_ip;
    unsigned int crc32_bin;
    unsigned int server_addr;
    unsigned int patch_trigger_addr;
    unsigned int patch_trigger_pattern;
    int index;
} pso_disc_t;

#ifndef PLANET_RING
/* List of known PSO discs */
static pso_disc_t discs[NUM_PSO_DISCS] = {
    {
        "Phantasy Star Online Ver.1 (Japanese)",
        0xECF7CEE8,
        0xE69859E1,
        0x8C28CC1A,
        0x8C28CC1C,
        0x2E31306F,
        0
    },
    {
        "Phantasy Star Online Ver.1 (US)",
        0xC2BB8853,
        0xFB669263,
        0x8C2866FA,
        0x8C2866FC,
        0x3130656D,
        1
    },
    {
        "Phantasy Star Online Ver.1 (European)",
        0x17FEFBBC,
        0x0F10B726,
        0x8C2861CA,
        0x8C2861CC,
        0x72657473,
        2
    },
    {
        "Phantasy Star Online Ver.2 (Japanese)",
        0x741FB98C,
        0x64360E99,
        0x8C2EC50B,
        0x8C2EC50C,
        0x31306F73,
        3
    },
    {
        "Phantasy Star Online Ver.2 (US)",
        0xD744EFC0,
        0xE962AC47,
        0x8C2EEA73,
        0x8C2EEA74,
        0x30656D61,
        4
    },
    {
        "Phantasy Star Online Ver.2 (European)",
        0xDA49302B,
        0xE962AC47,
        0x8C2E301B,
        0x8C2E301C,
        0x65747361,
        5
    },
    {
        "Phantasy Star Online Network Trial Edition",
        0x27EA2AB8,
        0xE5A7F3A1,
        0x8C22DF8D,
        0x8C22DF90,
        0x642E3230,
        6
    }
};

/* Various 32-bit sized patches that we perform on the main game binary. These
   include the Hunter's License check disabling, as well as fixing up Sega's
   bug in the map table (not including all the ultimate mode maps). The first
   number in the list is the number of patches performed for that version. The
   rest of the numbers are in <address, value> pairs. */
static const uint32 patch_tables[NUM_PSO_DISCS][23] = {
    /* PSOv1 Japanese -- HL Check only */
    {
        1,
        0x8C20C474, 0xE000000B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv1 US -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv1 European -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv2 Japanese -- HL Check + Maps */
    {
        9,
        0x8C237404, 0xE000000B, 0x8C32D638, 0x8C0081FC, 0x8C32D63C, 0x00000006,
        0x8C32D648, 0x8C00822C, 0x8C32D64C, 0x00000006, 0x8C32D650, 0x8C00825C,
        0x8C32D654, 0x00000006, 0x8C32D658, 0x8C00828C, 0x8C32D65C, 0x00000006,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv2 US -- HL Check + Maps */
    {
        9,
        0x8C23859C, 0xE000000B, 0x8C32FED0, 0x8C0081FC, 0x8C32FED4, 0x00000006,
        0x8C32FEE0, 0x8C00822C, 0x8C32FEE4, 0x00000006, 0x8C32FEE8, 0x8C00825C,
        0x8C32FEEC, 0x00000006, 0x8C32FEF0, 0x8C00828C, 0x8C32FEF4, 0x00000006,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv2 European -- Maps only */
    {
        8,
        0x8C3244A8, 0x8C0081FC, 0x8C3244AC, 0x00000006, 0x8C3244B8, 0x8C00822C,
        0x8C3244BC, 0x00000006, 0x8C3244C0, 0x8C00825C, 0x8C3244C4, 0x00000006,
        0x8C3244C8, 0x8C00828C, 0x8C3244CC, 0x00000006, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSO NTE -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
};

static const uint32 patch_tables2[NUM_PSO_DISCS][23] = {
    /* PSOv1 Japanese -- HL Check only */
    {
        1,
        0x8C20C474, 0xE000000B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv1 US -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv1 European -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    /* PSOv2 Japanese -- HL Check + Maps */
    {
        11,
        0x8C237404, 0xE000000B, 0x8C32D638, 0x8C0081FC, 0x8C32D63C, 0x00000006,
        0x8C32D648, 0x8C00822C, 0x8C32D64C, 0x00000006, 0x8C32D650, 0x8C00825C,
        0x8C32D654, 0x00000006, 0x8C32D658, 0x8C00828C, 0x8C32D65C, 0x00000006,
        0x8C333FC0, 0x8C345430, 0x8C333FD4, 0x8C34543A
    },
    /* PSOv2 US -- HL Check + Maps */
    {
        11,
        0x8C23859C, 0xE000000B, 0x8C32FED0, 0x8C0081FC, 0x8C32FED4, 0x00000006,
        0x8C32FEE0, 0x8C00822C, 0x8C32FEE4, 0x00000006, 0x8C32FEE8, 0x8C00825C,
        0x8C32FEEC, 0x00000006, 0x8C32FEF0, 0x8C00828C, 0x8C32FEF4, 0x00000006,
        0x8C336860, 0x8C347D48, 0x8C336874, 0x8C347D52
    },
    /* PSOv2 European -- Maps only */
    {
        10,
        0x8C3244A8, 0x8C0081FC, 0x8C3244AC, 0x00000006, 0x8C3244B8, 0x8C00822C,
        0x8C3244BC, 0x00000006, 0x8C3244C0, 0x8C00825C, 0x8C3244C4, 0x00000006,
        0x8C3244C8, 0x8C00828C, 0x8C3244CC, 0x00000006, 0x8C32AE38, 0x8C33AE08,
        0x8C32AE4C, 0x8C33AE12, 0x00000000, 0x00000000
    },
    /* PSO NTE -- No Patches */
    {
        0,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
};

/* Map pointer tables... These are effectively pointers to filenames. This
   replaces the data for Caves and Mines in the PSO binaries (for v2). */
static const uint32 map_ptrs[NUM_PSO_DISCS][48] = {
    /* PSOv1 Japanese, US and European -- Nothing of interest */
    { 0 }, { 0 }, { 0 },
    /* PSOv2 Japanese */
    {
        0x8C32DBBF, 0x8C32DBCB, 0x8C32DBBF, 0x8C32DBDA, 0x8C32DBBF, 0x8C32DBE9,
        0x8C32DBBF, 0x8C32DBF8, 0x8C32DBBF, 0x8C32DC07, 0x8C32DBBF, 0x8C0082BC,
        0x8C32DC6D, 0x8C32DC79, 0x8C32DC6D, 0x8C32DC88, 0x8C32DC6D, 0x8C32DC97,
        0x8C32DC6D, 0x8C32DCA6, 0x8C32DC6D, 0x8C32DCB5, 0x8C32DC6D, 0x8C0082CB,
        0x8C32DCC4, 0x8C32DCD3, 0x8C32DCC4, 0x8C32DCE5, 0x8C32DCC4, 0x8C32DCF7,
        0x8C32DCC4, 0x8C32DD09, 0x8C32DCC4, 0x8C32DD1B, 0x8C32DCC4, 0x8C0082DA,
        0x8C32DD2D, 0x8C32DD3C, 0x8C32DD2D, 0x8C32DD4E, 0x8C32DD2D, 0x8C32DD60,
        0x8C32DD2D, 0x8C32DD72, 0x8C32DD2D, 0x8C32DD84, 0x8C32DD2D, 0x8C0082EC
    },
    /* PSOv2 US */
    {
        0x8C330457, 0x8C330463, 0x8C330457, 0x8C330472, 0x8C330457, 0x8C330481,
        0x8C330457, 0x8C330490, 0x8C330457, 0x8C33049F, 0x8C330457, 0x8C0082BC,
        0x8C330505, 0x8C330511, 0x8C330505, 0x8C330520, 0x8C330505, 0x8C33052F,
        0x8C330505, 0x8C33053E, 0x8C330505, 0x8C33054D, 0x8C330505, 0x8C0082CB,
        0x8C33055C, 0x8C33056B, 0x8C33055C, 0x8C33057D, 0x8C33055C, 0x8C33058F,
        0x8C33055C, 0x8C3305A1, 0x8C33055C, 0x8C3305B3, 0x8C33055C, 0x8C0082DA,
        0x8C3305C5, 0x8C3305D4, 0x8C3305C5, 0x8C3305E6, 0x8C3305C5, 0x8C3305F8,
        0x8C3305C5, 0x8C33060A, 0x8C3305C5, 0x8C33061C, 0x8C3305C5, 0x8C0082EC
    },
    /* PSOv2 European */
    {
        0x8C324A2F, 0x8C324A3B, 0x8C324A2F, 0x8C324A4A, 0x8C324A2F, 0x8C324A59,
        0x8C324A2F, 0x8C324A68, 0x8C324A2F, 0x8C324A77, 0x8C324A2F, 0x8C0082BC,
        0x8C324ADD, 0x8C324AE9, 0x8C324ADD, 0x8C324AF8, 0x8C324ADD, 0x8C324B07,
        0x8C324ADD, 0x8C324B16, 0x8C324ADD, 0x8C324B25, 0x8C324ADD, 0x8C0082CB,
        0x8C324B34, 0x8C324B43, 0x8C324B34, 0x8C324B55, 0x8C324B34, 0x8C324B67,
        0x8C324B34, 0x8C324B79, 0x8C324B34, 0x8C324B8B, 0x8C324B34, 0x8C0082DA,
        0x8C324B9D, 0x8C324BAC, 0x8C324B9D, 0x8C324BBE, 0x8C324B9D, 0x8C324BD0,
        0x8C324B9D, 0x8C324BE2, 0x8C324B9D, 0x8C324BF4, 0x8C324B9D, 0x8C0082EC
    },
    /* PSO NTE -- Nothing of interest */
    { 0 }
};

/* Added map names */
static const char map_names[68] =
    "map_acave01_05\0map_acave03_05\0"
    "map_amachine01_05\0map_amachine02_05\0\0\0";

#else

/* List of known Planet Ring discs */
static pso_disc_t discs[NUM_PSO_DISCS] = {
    {
        "Planet Ring (European)",
        0xA9E0B039,
        0xC4229B0E,
        0x8C0721EC,
        0x8C0721EC,
        0x7473616D,
        0
    }
};

#endif

#endif /* !PATCHES_H */
