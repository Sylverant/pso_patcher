#	Sylverant PSO Patcher Makefile
#	Copyright (C) 2011, 2013 Lawrence Sebald
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License version 3 as
#   published by the Free Software Foundation.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.

TARGET = patcher.bin

OBJS = patcher.o fs_iso9660.o patch.o gd.o fb_console.o

KOS_LDFLAGS = -Tshlelf.xc -nodefaultlibs $(KOS_LIB_PATHS)

all: rm-elf $(TARGET)

prpatcher: KOS_CFLAGS += -DPLANET_RING
prpatcher: rm-elf prpatcher.bin

clean:
	-rm -f $(TARGET) patcher.bin patcher.elf romdisk.* $(OBJS)
	-rm -f prpatcher.elf prpatcher.bin pr-romdisk.*

rm-elf:
	-rm -f $(TARGET) patcher.elf romdisk.*
	-rm -f prpatcher.bin prpatcher.elf pr-romdisk.*

patcher.elf: $(OBJS) romdisk.o
	kos-cc -o $@ $^ -lpng -lz -lm -lkosutils
	
romdisk.img:
	$(KOS_GENROMFS) -f $@ -d romdisk -v -x .svn

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk $@

patcher.bin: patcher.elf
	kos-objcopy -O binary $^ $@

prpatcher.elf: $(OBJS) pr-romdisk.o
	kos-cc -o $@ $^ -lpng -lz -lm -lkosutils

pr-romdisk.img:
	$(KOS_GENROMFS) -f $@ -d romdisk -v -x .svn

pr-romdisk.o: pr-romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk $@

prpatcher.bin: prpatcher.elf
	kos-objcopy -O binary $^ $@
	cp prpatcher.bin patcher.bin

# We need to preprocess the patch.s file...
patch.o: patch.s
	kos-cc -c $< -o $@

include $(KOS_BASE)/Makefile.rules
