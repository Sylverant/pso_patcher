#ifndef GDEMU_H
#define GDEMU_H

#include "types.h"

#define GDEMU_IMG_NEXT 0x55
#define GDEMU_IMG_PREV 0x44

int gdemu_img_cmd(uint8 cmd);

#endif /* GDEMU_H */
