#include "chip8.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>


void chip8_load_font(Chip8 *chip8) {
    uint8_t fontset[80] = {
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

    for (int i = 0; i < 80; i++) {
        chip8->memory[i] = fontset[i];
    }

}


void chip8_init(Chip8 *chip8) {
    chip8->PC = 0x200;
    chip8->SP = 0;
    chip8->I = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;
    chip8->drawFlag = false;
    chip8->wait_key = false;
    memset(chip8->memory, 0, 4096);
    memset(chip8->V, 0, 16);
    memset(chip8->stack, 0, 32);
    memset(chip8->key, 0, 16);
    memset(chip8->screen, 0, 64 * 32);
    chip8_load_font(chip8);
    chip8->running = true;
}

void chip8_load_rom(Chip8 *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if(!file) {
        printf("Error cannot load ROM\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    if(size > (4096 - 0x200)) {
        printf("Error ROM too large");
        return;
    }
    size_t bytes_read = fread(&chip8->memory[0x200], 1, size, file);
    printf("ROM Loaded");
    fclose(file);
}

void chip8_run(Chip8 *chip8) {
    while(chip8->running) {
        if(chip8->wait_key) {
            int key_pressed = -1;
            for(int i = 0; i < 16; i++) {
                if(chip8->key[i] == 1) {
                    key_pressed = i;
                    chip8->key[i] = 0;
                    break;
                }
            }
            
            if(key_pressed != -1) {
                chip8->V[chip8->key_register] = key_pressed;
                chip8->wait_key = false;
                chip8->PC += 2;
                // Key sudah ditekan, lanjut ke chip8_cycle
            } else {
                continue;  // Belum ada key, skip chip8_cycle
                }
            }
        chip8_cycle(chip8);  // Eksekusi opcode (normal atau setelah key ditekan)
        }
    return;
    }

void chip8_cycle(Chip8 *chip8) {
    uint16_t opcode = (chip8->memory[chip8->PC] << 8) | chip8->memory[chip8->PC + 1];
    uint8_t type = (opcode & 0xF000) >> 12;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint16_t nnn = (opcode & 0x0FFF);
    uint8_t nn = (opcode & 0x00FF);
    uint8_t n = (opcode& 0x000F);
    // All Opcode Below


    switch(type) {
        case 0x0:
            switch(opcode & 0x00FF){

                case 0xE0:
                    memset(chip8->screen, 0, 64 * 32);
                    chip8->PC += 2;
                    break;

                case 0xEE:
                    chip8->PC = chip8->stack[chip8->SP];
                    chip8->SP--;
                    break;
            }
            break;


        case 0x1:
            chip8->PC = nnn;
            break;

        case 0x2:
            chip8->SP++;
            chip8->stack[chip8->SP] = chip8->PC;
            chip8->PC = nnn;
            break;

        case 0x3:
            chip8->PC += (chip8->V[x] == nn) ? 4 : 2;
            break;

        case 0x4:
            chip8->PC += (chip8->V[x] != nn) ? 4 : 2;
            break;

        case 0x5:
            chip8->PC += (chip8->V[x] == chip8->V[y]) ? 4 : 2;
            break;

        case 0x6:
            printf("LD V[%d] = %02X\n", x, nn);
            chip8->V[x] = nn;
            chip8->PC += 2;
            break;

        case 0x7:
            chip8->V[x] = chip8->V[x] + nn;
            chip8->PC += 2;
            break;


        case 0x8:
            switch(opcode & 0x000F) {
                case 0x0:
                    chip8->V[x] = chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x1:
                    chip8->V[x] = chip8->V[x] | chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x2:
                    chip8->V[x] = chip8->V[x] & chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x3:
                    chip8->V[x] = chip8->V[x] ^ chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x4:
                    chip8->V[0xF] = (chip8->V[x] + chip8->V[y] > 0xFF) ?1 : 0;
                    chip8->V[x] += chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x5:
                    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    chip8->PC += 2;
                    break;

                case 0x6:
                    chip8->V[0xF] = chip8->V[x] & 1;
                    chip8->V[x] >>= 1;
                    chip8->PC += 2;
                    break;

                case 0x7:
                    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->PC += 2;
                    break;

                case 0xE:
                    chip8->V[0xF] = (chip8->V[x] >> 7) ? 1 : 0;
                    chip8->V[x] <<= 1;
                    chip8->PC += 2;
                    break;
            }

                
        case 0x9:
            chip8->PC += (chip8->V[x] != chip8->V[y]) ? 4 : 2;
            break;

        case 0xA:
            chip8->I = nnn;
            chip8->PC += 2;
            break;

        case 0xB:
            chip8->PC = nnn + chip8->V[0x0];
            break;

        case 0xC:
            chip8->V[x] = rand() & nn;
            chip8->PC += 2;
            break;


        case 0xD: {
            uint8_t x_pos = chip8->V[x] % 64;
            uint8_t y_pos = chip8->V[y] % 32;
            chip8->V[0xF] = 0;
            static int draw_count = 0;
draw_count++;
printf("Draw #%d: x=%d, y=%d, n=%d\n", draw_count, chip8->V[x], chip8->V[y], n);

            for(int row = 0 ; row < n; row++){
                uint8_t sp_bytes = chip8->memory[chip8->I + row];
                for(int col = 0; col < 8; col++){
                    if(sp_bytes & (0x80 >> col)) {
                            uint8_t scr_x = (x_pos + col) % 64;
                            uint8_t scr_y = (y_pos + row) % 32;
                            uint16_t index = scr_y * 64 + scr_x;

                            if(chip8->screen[index] == 1){
                                chip8->V[0xF] = 1;
                            }
                            chip8->screen[index] ^= 1;
                        }
                }
            }
            chip8->drawFlag = true;
            chip8->PC += 2;
            break;  
        }


        case 0xE:
                  switch(opcode & 0x00FF) {
                    case 0x9E:
                          chip8->PC += (chip8->key[chip8->V[x]] == 1) ? 4 : 2;
                          break;
                    
                    case 0xA1:
                          chip8->PC += (chip8->key[chip8->V[x]] == 0) ? 4 : 2;
                          break;

                }
                  break;
        
        case 0xF:
                  switch(opcode & 0x00FF) {
                    case 0x07:
                          chip8->V[x] = chip8->delay_timer;
                          chip8->PC += 2;
                          break;

                    case 0x0A:
                          chip8->wait_key = true;
                          chip8->key_register = x;
                          break;

                    case 0x15:
                          chip8->delay_timer = chip8->V[x];
                          chip8->PC += 2;
                          break;

                    case 0x18:
                          chip8->sound_timer = chip8->V[x];
                          chip8->PC += 2;
                          break;

                    case 0x1E:
                          chip8->I += chip8->V[x];
                          chip8->PC += 2;
                          break;

                    case 0x29:
                          chip8->I = chip8->V[x] * 5;
                          chip8->PC += 2;
                          break;

                    case 0x33:
                          chip8->memory[chip8->I] = chip8->V[x] / 100;
                          chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                          chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
                          chip8->PC += 2;
                          break;

                    case 0x55:
                          for(int i = 0; i <= x; i++) {
                              chip8->memory[chip8->I + i] = chip8->V[i];
                          }
                          chip8->PC += 2;
                          break;

                    case 0x65:
                          for (int i = 0; i <= x; i ++) {
                              chip8->V[i] = chip8->memory[chip8->I + i];
                          }
                          chip8->PC +=  2;
                          break;
                }
                  break;
    }

    // Di chip8_cycle, setiap kali selesai eksekusi opcode:
    if (chip8->delay_timer > 0) chip8->delay_timer--;
    if (chip8->sound_timer > 0) chip8->sound_timer--;
}
