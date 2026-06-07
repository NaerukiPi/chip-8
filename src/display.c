#include "chip8.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void display_init() {}

void display_render(Chip8 *chip) {
  system("clear");

  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 64; col++) {
      if (chip->gfx[row][col] == 1) {
        printf("@");
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }

  fflush(stdout);
  chip->drawFlag = false;
}

void display_clear() {}

void display_close() {}
