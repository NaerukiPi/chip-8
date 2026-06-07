#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"

void display_init(void);
void display_render(Chip8 *chip);
void display_clear(void);
void display_close(void);

#endif
