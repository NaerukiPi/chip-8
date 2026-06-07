#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define START_PC 0x200
#define KEY 16
#define NUM_REGISTER 16
#define TIMER_RATE 60


typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[NUM_REGISTER];
    uint16_t PC;
    uint16_t stack [STACK_SIZE];
    uint16_t I;
    uint8_t SP;
    uint8_t soundTimer;
    uint8_t delayTimer;
    uint8_t gfx[32][64];
    bool key[KEY];
    bool drawFlag;
    uint16_t opcode;
    bool running;
}Chip8;

void chip8_init(Chip8 *chip);
void chip8_load_font(Chip8 *chip);
void chip8_load_rom(Chip8 *chip, const char *filename);
void chip8_cycle(Chip8 *chip);

#endif
