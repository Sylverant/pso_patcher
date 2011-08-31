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

!   GD-ROM syscall replacement function.
!   This is our vector for patching PSO after decryption. On the first GD-ROM
!   access after the main game binary is in RAM, this will trigger patching.
!   This gets copied to 0x8C008000. We have exactly 768 bytes there to do our
!   thing, which is plenty of space.
!
!   Before this gets copied, the various variables must be filled in.
    .globl      _gd_syscall

! List of variables that should be filled in...
    .globl      _patch_trigger_addr
    .globl      _patch_trigger_pattern
    .globl      _old_gd_vector
    .globl      _patches_count
    .globl      _patches
    .globl      _server_addr
    .globl      _patches_enabled
    .globl      _clear_and_load
    .globl      _gd_syscall_len

    .balign     4
_gd_syscall:
    ! Make sure we found something earlier
    mov.w       _patches_enabled, r0
    sts.l       pr, @-r15
    tst         #1, r0
    bt          .do_syscall
    ! Check the space that we need to look for the signature of the game binary.
    mov.l       _patch_trigger_addr, r1
    mov.l       _patch_trigger_pattern, r2
    mov.l       @r1, r1
    cmp/eq      r1, r2
    bt          .patch_pso
.do_syscall:
    ! Call on the original GD-ROM syscall to do the heavy lifting.
    mov.l       _old_gd_vector, r0
    jsr         @r0
    nop
    lds.l       @r15+, pr
    rts
    nop

    .balign     4
_patch_trigger_addr:
    .long       0
_patch_trigger_pattern:
    .long       0
_old_gd_vector:
    .long       0

.patch_pso:
    ! Copy any 32-bit patches we have...
    mov.l       _patches_count, r1
    mova        _patches, r0
    mov         #0, r2
    cmp/eq      r1, r2
    bt          .patch_server_name
.patches_loop:
    mov.l       @r0+, r3
    mov.l       @r0+, r2
    dt          r1
    mov.l       r2, @r3
    bf/s        .patches_loop
    ocbp        @r3
    ! Patch the server address
.patch_server_name:
    mova        .server_host, r0
    mov.l       _server_addr, r2
.server_copy_loop:
    mov.b       @r0+, r3
    cmp/eq      r1, r3
    mov.b       r3, @r2
    bf/s        .server_copy_loop
    add         #1, r2
    bra         .do_syscall
    nop

    .balign     4
_patches_count:
    .long       (.patches_end - _patches) >> 3
_patches:
    ! The maximum number of patches here is 9. The maximal list is shown in the
    ! comments. Not all versions will have all patches (and since EUv2 needs the
    ! map fix patch, but not the HL Check, they may not all match up with the
    ! comment either).
    .long       0, 0                    ! HL Check patch (rts; mov #0, r0)
    .long       0, 0                    ! Cave 1 map pointers (Ultimate)
    .long       0, 0                    ! Cave 1 map count (Ultimate)
    .long       0, 0                    ! Cave 3 map pointers (Ultimate)
    .long       0, 0                    ! Cave 3 map count (Ultimate)
    .long       0, 0                    ! Mine 1 map pointers (Ultimate)
    .long       0, 0                    ! Mine 1 map count (Ultimate)
    .long       0, 0                    ! Mine 2 map pointers (Ultimate)
    .long       0, 0                    ! Mine 2 map count (Ultimate)
.patches_end:
_server_addr:
    .long       0
.server_host:
    .string     "sylverant.net"
    .balign     2
_patches_enabled:
    .word       1                       ! We will reset this otherwise...
_clear_and_load:
    ! Disable interrupts
    mov.l       .no_ints_sr, r0
    ldc         r0, sr
    ! Clear out the loader
    mov.l       .loader_base, r0
    mov.l       .loader_end, r1
    mov         #0, r2
.clear_loop:
    mov.l       r2, @r0
    add         #4, r0
    cmp/eq      r0, r1
    bf          .clear_loop
    ! Clean up the various registers...
    ldc         r4, sr
    ldc         r5, vbr
    lds         r6, fpscr
    mov         r7, r15
    ! Jump to IP.BIN 
    mov.l       .bin_base, r0
    jmp         @r0
    nop
    .balign     4
.no_ints_sr:
    .long       0x500000F0
.loader_base:
    .long       0xACD00000
.loader_end:
    .long       0xAD000000
.bin_base:
    ! This is actually the first stage of the IP.BIN file.
    .long       0xAC00B800
    
_gd_syscall_len:
    .long       _gd_syscall_len - _gd_syscall
    .balign     4
