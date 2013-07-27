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

CC = sh-elf-gcc -ml -m4-nofpu
OBJCOPY = sh-elf-objcopy -O binary -R .stack

TARGET = loader.bin

all: rm-elf $(TARGET)

loader.elf: loader.s ../patcher/patcher.bin
	$(CC) -o $@ loader.s -nostartfiles -nostdlib -Ttext=0x8c010000 \
		-Wa,-I../patcher

loader.bin: loader.elf
	$(OBJCOPY) $^ $@

.PHONY : clean
clean:
	-rm -f $(TARGET) loader.bin loader.elf $(OBJS)

.PHONY : rm-elf
rm-elf:
	-rm -f $(TARGET) loader.elf
