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

#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void memset(uint8 *p1, uint32 val, uint32 count);
void memset2(uint16 *p1, uint32 val, uint32 count);
void memset4(uint32 *p1, uint32 val, uint32 count);

void memcpy(void *p1, const void *p2, uint32 count);
void memcpy2(uint16 *p1, const uint16 *p2, uint32 count);
void memcpy4(uint32 *p1, const uint32 *p2, uint32 count);

uint32 hex_to_uint32(const char *hex, int len);

#endif /* !UTILS_H */
