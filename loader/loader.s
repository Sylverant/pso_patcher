!   This file is part of Sylverant PSO Patcher
!   Copyright (C) 2011, 2013 Lawrence Sebald
!
!   This program is free software: you can redistribute it and/or modify
!   it under the terms of the GNU General Public License version 3 as
!   published by the Free Software Foundation.
!
!   This program is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU General Public License for more details.
!
!   You should have received a copy of the GNU General Public License
!   along with this program.  If not, see <http://www.gnu.org/licenses/>.

!   This code is simply responsible for loading the next phase of the program
!   into RAM at the appropriate address (0xACE00000), rather than the normal
!   1ST_READ.BIN location of 0xAC010000 (or 0x8C010000, if you like locations in
!   P1 instead of P2).
    .text
    .balign     2
    .globl      start
start:
    ! Disable interrupts
    mov.l       init_sr, r0
    ldc         r0, sr
    ! Copy the binary over to the appropriate location
    mov.l       bin_base, r2
    mov.l       bin_size, r1
    mov.l       bin_ptr, r0
    mov         r2, r4
loop:
    mov.l       @r0+, r3
    dt          r1
    mov.l       r3, @r2
    bf/s        loop
    add         #4, r2
    jmp         @r4
    nop

    .balign     4
init_sr:
    .long       0x500000F0
bin_size:
    .long       (bin_end - bin) >> 2
bin_ptr:
    .long       bin
bin_base:
    .long       0xACE00000

    .section    .rodata
    .balign     4
bin:
    .incbin     "patcher.bin"
    .balign     4
bin_end:
