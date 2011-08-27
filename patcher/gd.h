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

#ifndef GD_H
#define GD_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* Read a single sector off the disc */
int gd_read_sector(uint32 fad, uint16 *out, uint32 *sz);

/* Read the TOC off the disc */
int gd_read_toc(uint16 *out, uint32 *sz);

__END_DECLS

#endif /* !GD_H */
