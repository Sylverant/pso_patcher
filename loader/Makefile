#	Sylverant PSO Patcher Makefile
#	Copyright (C) 2011 Lawrence Sebald
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

TARGET = loader.bin

OBJS = loader.o

all: rm-elf $(TARGET)

clean:
	-rm -f $(TARGET) loader.bin loader.elf $(OBJS)

rm-elf:
	-rm -f $(TARGET) loader.elf

loader.elf: loader.s ../patcher/patcher.bin
	kos-cc -o $@ loader.s -nostartfiles -nostdlib -Ttext=0x8c010000 \
		-Wa,-I../patcher

loader.bin: loader.elf
	kos-objcopy -O binary $^ $@

include $(KOS_BASE)/Makefile.rules
