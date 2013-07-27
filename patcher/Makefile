#	Sylverant PSO Patcher Makefile
#	Copyright (C) 2013 Lawrence Sebald
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

AS = sh-elf-as -little
CC = sh-elf-gcc -ml -m4-nofpu -ffunction-sections -fdata-sections \
	 -ffreestanding -Wall -std=c99 -Wextra -Werror
OBJCOPY = sh-elf-objcopy -O binary

# startup.o must show up first here...
OBJS = startup.o cdrom.o gd.o patch.o cdfs.o patcher.o video.o biosfont.o \
	   fb_console.o maple.o utils.o cache.o

all: rm-elf patcher.bin

prpatcher: CC += -DPLANET_RING
prpatcher: rm-elf prpatcher.bin

patcher.elf: $(OBJS)
	$(CC) -o $@ -Wl,--gc-sections -Tshlelf.xc -nostartfiles -nostdlib $^ -lgcc

patcher.bin: patcher.elf
	$(OBJCOPY) $^ $@

prpatcher.elf: $(OBJS)
	$(CC) -o $@ -Wl,--gc-sections -Tshlelf.xc -nostartfiles -nostdlib $^ -lgcc

prpatcher.bin: prpatcher.elf
	kos-objcopy -O binary $^ $@
	cp prpatcher.bin patcher.bin

.PHONY : rm-elf
rm-elf:
	-rm -f loader.elf loader.bin

.PHONY : clean
clean:
	-rm -f $(OBJS)
	-rm -f *.elf
	-rm -f *.bin
