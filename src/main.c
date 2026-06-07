#include "chip8.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage <romfile>.ch8");
    return 1;
  }
  Chip8 chip;
  chip8_init(&chip);
  chip8_load_rom(&chip, argv[1]);
  while (chip.running) {
    if (chip.delayTimer > 0)
      chip.delayTimer--;
    chip8_cycle(&chip);
    if (chip.drawFlag) {
      display_render(&chip);
      chip.drawFlag = false;
    }
    usleep(8000);
  }
  return 1;
};
