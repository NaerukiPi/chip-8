#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"

int init_display(void);
void render_display(Chip8 *chip8);
void close_display(void);
void handle_input(Chip8 *chip8);

#endif
