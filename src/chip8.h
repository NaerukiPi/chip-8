#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t memory[4096]; // Ukuran Memori
    uint16_t stack[16]; // Alamat sementara
    uint8_t V[16]; // Register
    uint16_t I; // Index Register Alamat Memori
    uint16_t PC; // Program Counter
    uint8_t SP; // Stack Pointer
    uint8_t key[16]; // Tombol
    uint8_t delay_timer; // Delay Timer
    uint8_t sound_timer; // Sound Timer
    uint8_t key_register; // Key Register
    bool drawFlag; // Inisiasi Gambar
    bool wait_key; // Tunggu Tombol
    bool running; // Emulator Run or Not
    uint8_t screen[64 * 32]; // Ukuran Layar
}Chip8;

void chip8_init(Chip8 *chip8);
void chip8_cycle(Chip8 *chip8);
void chip8_run (Chip8 *chip8);
void chip8_load_font(Chip8 *chip8);
void chip8_load_rom(Chip8 *chip8, const char *filename);
void chip8_render(Chip8 *chip8);


#endif
