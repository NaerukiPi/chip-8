#include "chip8.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_load_font(Chip8 *chip) {
  for (int i = 0; i < 80; i++) {
    chip->memory[i] = fontset[i];
  }
};

void chip8_init(Chip8 *chip) {
  memset(chip->memory, 0, sizeof(chip->memory));
  chip8_load_font(chip);
  memset(chip->key, 0, sizeof(chip->key));
  memset(chip->V, 0, sizeof(chip->V));
  memset(chip->stack, 0, sizeof(chip->stack));
  memset(chip->gfx, 0, sizeof(chip->gfx));
  chip->PC = START_PC;
  chip->I = 0;
  chip->SP = 0;
  chip->running = true;
  chip->delayTimer = 0;
  chip->soundTimer = 0;
  chip->drawFlag = false;
  chip->opcode = 0;
};

void chip8_load_rom(Chip8 *chip, const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    printf("ROM Not Found");
    return;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  rewind(file);

  if (fileSize > MEMORY_SIZE - START_PC) {
    printf("ROM File Too Large");
    fclose(file);
    return;
  }

  if (fileSize <= MEMORY_SIZE - START_PC) {
    fread(chip->memory + START_PC, 1, fileSize, file);
    fclose(file);
  }
};

void chip8_cycle(Chip8 *chip) {
  uint16_t opcode = chip->memory[chip->PC] << 8 | chip->memory[chip->PC + 1];
  chip->PC += 2;
  uint8_t type = (opcode & 0xF000) >> 12;
  uint16_t nnn = (opcode & 0x0FFF);
  uint8_t nn = (opcode & 0x00FF);
  uint8_t n = (opcode & 0x000F);
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  switch (type) {
  case 0x0:
    switch (nn) {
    case 0xE0:
      memset(chip->gfx, 0, sizeof(chip->gfx));
      chip->drawFlag = true;
      break;
    case 0xEE:
      chip->PC = chip->stack[chip->SP];
      chip->SP--;
      break;
    }
    break;
  case 0x1:
    chip->PC = nnn;
    break;
  case 0x2:
    chip->SP++;
    chip->stack[chip->SP] = chip->PC;
    chip->PC = nnn;
    break;
  case 0x3:
    chip->PC += ((chip->V[x] == nn) ? 2 : 0);
    break;
  case 0x4:
    chip->PC += ((chip->V[x] != nn) ? 2 : 0);
    break;
  case 0x5:
    chip->PC += ((chip->V[x] == chip->V[y]) ? 2 : 0);
    break;
  case 0x6:
    chip->V[x] = nn;
    break;
  case 0x7:
    chip->V[x] = nn + chip->V[x];
    break;
  case 0x8:
    switch (n) {
    case 0x0:
      chip->V[x] = chip->V[y];
      break;
    case 0x1:
      chip->V[x] = chip->V[x] | chip->V[y];
      break;
    case 0x2:
      chip->V[x] = chip->V[x] & chip->V[y];
      break;
    case 0x3:
      chip->V[x] = chip->V[x] ^ chip->V[y];
      break;
    case 0x4:
      chip->V[0xF] = (chip->V[x] + chip->V[y] > 0xFF);
      chip->V[x] += chip->V[y];
      break;
    case 0x5:
      chip->V[0xF] = (chip->V[x] >= chip->V[y] ? 1 : 0);
      chip->V[x] -= chip->V[y];
      break;
    case 0x6:
      chip->V[0xF] = ((chip->V[x] & 0x1) ? 1 : 0);
      chip->V[x] >>= 1;
      break;
    case 0x7:
      chip->V[0xF] = ((chip->V[y] >= chip->V[x]) ? 1 : 0);
      chip->V[x] = chip->V[y] - chip->V[x];
      break;
    case 0xE:
      chip->V[0xF] = ((chip->V[x] >> 7) ? 1 : 0);
      chip->V[x] <<= 1;
      break;
    }
    break;
  case 0x9:
    chip->PC += ((chip->V[x] != chip->V[y]) ? 2 : 0);
    break;
  case 0xA:
    chip->I = nnn;
    break;
  case 0xB:
    chip->PC = nnn + chip->V[0x0];
    break;
  case 0xC:
    chip->V[x] = (rand() & 0xFF) & nn;
    break;
  case 0xD:
    chip->V[0xF] = 0;
    chip->drawFlag = true;
    uint8_t scr_x = chip->V[x] % 64;
    uint8_t scr_y = chip->V[y] % 32;
    for (int row = 0; row < n; row++) {
      uint8_t sprite_bytes = chip->memory[chip->I + row];
      for (int col = 0; col < 8; col++) {
        if ((sprite_bytes >> (7 - col)) & 1) {
          int x_pos = (scr_x + col) % 64;
          int y_pos = (scr_y + row) % 32;
          if (chip->gfx[y_pos][x_pos] == 1) {
            chip->V[0xF] = 1;
          }
          chip->gfx[y_pos][x_pos] ^= 1;
        }
      }
    }
    break;
  case 0xE:
    switch (nn) {
    case 0x9E:
      chip->PC += ((chip->key[chip->V[x]] != 0) ? 2 : 0);
      break;
    case 0xA1:
      chip->PC += ((chip->key[chip->V[x]] == 0) ? 2 : 0);
      break;
    };
    break;
  case 0xF:
    switch (nn) {
    case 0x07:
      chip->V[x] = chip->delayTimer;
      break;
    case 0x0A: {
      uint8_t keyPressed = 0;
      for (int i = 0; i < 16; i++) {
        if (chip->key[i]) {
          keyPressed = i;
          break;
        }
      }

      if (keyPressed) {
        chip->V[x] = keyPressed;
      } else {
        chip->PC -= 2;
      }
      break;
    }
    case 0x15:
      chip->delayTimer = chip->V[x];
      break;
    case 0x18:
      chip->soundTimer = chip->V[x];
      break;
    case 0x1E:
      chip->I = chip->I + chip->V[x];
      break;
    case 0x29:
      chip->I = chip->V[x] * 5;
      break;
    case 0x33:
      chip->memory[chip->I] = chip->V[x] / 100;
      chip->memory[chip->I + 1] = (chip->V[x] / 10) % 10;
      chip->memory[chip->I + 2] = chip->V[x] % 10;
      break;
    case 0x55:
      for (int i = 0; i <= x; i++) {
        chip->memory[chip->I + i] = chip->V[i];
      }
      break;
    case 0x65:
      for (int i = 0; i <= x; i++) {
        chip->V[i] = chip->memory[chip->I + i];
      }
      break;
    };
    break;
  }
};
