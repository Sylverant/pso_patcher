!   This file is part of Sylverant PSO Patcher
!   Copyright (C) 2011 Lawrence Sebald
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

!   This is wonderful, magical code to access the GD-ROM drive. It will only do
!   two things, namely read a TOC and read a single sector. Was there a
!   particular reason to do this in assembly? Not really. I do have a C version
!   of this same code that I wrote a while back but never bothered to release.
!   Perhaps I will at some point release that code too...
!
!   Anyway, calling this a piece of code that reads the GD-ROM would be a bit of
!   a misnomer. It will not read GD-ROM discs as it stands, by design. You may
!   ask, "If that is the case, then why is it even here, and how are you reading
!   the disc?" Well, that involves a bit of a trick. ;-)
!
!   To anyone that understands and can figure this piece of code out, more power
!   to you. It was a pain to write, but it gave me a bit more practice at
!   writing SH4 assembly. As I said, there is not really a particular reason to
!   write this in assembly other than that.
    .globl      _gd_read_toc
    .globl      _gd_read_sector

    .balign     4
.do_pio_cmd:
    mov         #0x88, r2
    mov.l       .reg1, r0
    extu.b      r2, r2
    mov.l       @r0, r1
.pio_cmd_wait_ready_loop1:
    tst         r2, r1
    bf/s        .pio_cmd_wait_ready_loop1
    mov.l       @r0, r1
    mov.l       .reg2, r2
    mov         #0, r1
    mov.b       r1, @r2
    mov.l       @r6, r1
    mov.l       .reg7, r3
    mov         r1, r2
    shlr8       r1
    mov.b       r1, @r3
    mov.l       .reg6, r3
    mov.b       r1, @r3
    mov.l       .reg3, r2
    mov         #0xA0, r1
    mov.b       r1, @r2
    mov         #0x08, r2
    mov.l       @r0, r1
.pio_cmd_wait_ready_loop2:
    tst         r2, r1
    bt/s        .pio_cmd_wait_ready_loop2
    mov.l       @r0, r1
    mov.l       .reg4, r0
    mov.l       .reg5, r2
    mov.w       @r4+, r3
    mov.l       @r0, r1
    mov.w       r3, @r2
    mov.w       @r4+, r3
    mov.w       r3, @r2
    mov.w       @r4+, r3
    mov.w       r3, @r2
    mov.w       @r4+, r3
    mov.w       r3, @r2
    mov.w       @r4+, r3
    mov.w       r3, @r2
    mov.w       @r4, r3
    mov.w       r3, @r2
    mov.l       .asic_reg_b, r1
    mov         #0x01, r2
    mov.l       @r1, r3
.asic_wait_loop:
    tst         r2, r3
    bt/s        .asic_wait_loop
    mov.l       @r1, r3
    mov.l       @r0, r1
    mov.l       .reg7, r2
    mov.l       .reg6, r1
    mov.b       @r2, r2
    mov.b       @r1, r1
    extu.b      r2, r2
    extu.b      r1, r1
    shll8       r2
    or          r2, r1
    mov.l       @r0, r2
    mov.l       r1, @r6
    shlr        r1
    mov.l       .reg5, r3
.pio_cmd_read_loop:
    mov.w       @r3, r2
    dt          r1
    mov.w       r2, @r5
    bf/s        .pio_cmd_read_loop
    add         #2, r5
    mov.l       .asic_reg_b, r1
    mov         #0x01, r2
    mov.l       @r1, r3
.asic_wait_loop2:
    tst         r2, r3
    bt/s        .asic_wait_loop2
    mov.l       @r1, r3
    mov.l       @r0, r2
    rts
    mov.l       @r6, r0
    .balign     4
.reg1:
    .long       0xA05F7018
.reg2:
    .long       0xA05F7084
.reg3:
.reg4:
    .long       0xA05F709C
.reg5:
    .long       0xA05F7080
.asic_reg_b:
    .long       0xA05F6904
.reg6:
    .long       0xA05F7090
.reg7:
    .long       0xA05F7094

_gd_read_toc:
    sts.l       pr, @-r15
    mov.l       r4, @-r15
    mova        .toc_cmd_buf, r0
    mov         r5, r6
    mov         r4, r5
    bsr         .do_pio_cmd
    mov         r0, r4
    mov.l       @r15+, r4
    mov         #102, r1
.swap_toc:
    mov.l       @r4, r2
    dt          r1
    swap.b      r2, r2
    swap.w      r2, r2
    swap.b      r2, r2
    mov.l       r2, @r4
    bf/s        .swap_toc
    add         #4, r4
    lds.l       @r15+, pr
    rts
    nop
    .balign     4
.toc_cmd_buf:
    .byte       0x14, 0, 0, (408 >> 8)
    .byte       (408 & 0xFF), 0, 0, 0
    .byte       0, 0, 0, 0

_gd_read_sector:
    sts.l       pr, @-r15
    mova        .read_cmd_buf, r0
    mov         r4, r1
    mov         r4, r2
    shlr16      r1
    shlr8       r2
    mov         #2, r3
    mov.b       r1, @(r0, r3)
    add         #1, r3
    mov.b       r2, @(r0, r3)
    add         #1, r3
    mov.b       r4, @(r0, r3)
    bsr         .do_pio_cmd
    mov         r0, r4
    lds.l       @r15+, pr
    rts
    nop
    .balign     4
.read_cmd_buf:
    .byte       0x30, 0x24, 0, 0
    .byte       0, 0, 0, 0
    .byte       0, 0, 1, 0
