#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define WINDOW_WIDTH 640   // 64 * 10
#define WINDOW_HEIGHT 320  // 32 * 10
#define PIXEL_SIZE 10

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// Mapping keyboard PC ke CHIP-8 keypad
uint8_t keymap(SDL_Keycode key) {
    switch(key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return 0xFF;
    }
}

// Inisialisasi SDL
int init_sdl(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Error: %s\n", SDL_GetError());
        return 0;
    }
    
    window = SDL_CreateWindow("CHIP-8 Emulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window Error: %s\n", SDL_GetError());
        return 0;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return 0;
    }
    
    return 1;
}

// Render screen ke SDL window
void render_screen(Chip8 *chip8) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8->screen[y * 64 + x]) {
                SDL_Rect rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    
    SDL_RenderPresent(renderer);
}

// Load ROM ke memory
int load_rom(Chip8 *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open ROM file '%s'\n", filename);
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size > (4096 - 0x200)) {
        printf("Error: ROM too large (max %d bytes)\n", 4096 - 0x200);
        fclose(file);
        return 0;
    }
    
    size_t bytes = fread(&chip8->memory[0x200], 1, size, file);
    printf("Loaded %zu bytes from '%s'\n", bytes, filename);
    
    fclose(file);
    return 1;
}

// Cleanup SDL
void cleanup_sdl(void) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    // Cek argument
    if (argc < 2) {
        printf("Usage: %s <rom_file.ch8>\n", argv[0]);
        printf("Example: %s games/pong.ch8\n", argv[0]);
        return 1;
    }
    
    // Inisialisasi SDL
    if (!init_sdl()) {
        return 1;
    }
    
    // Inisialisasi CHIP-8
    Chip8 chip8;
    chip8_init(&chip8);
    
    // Load ROM
    if (!load_rom(&chip8, argv[1])) {
        cleanup_sdl();
        return 1;
    }
    
    chip8.running = true;
    chip8.drawFlag = true;
    
    printf("\n=== CHIP-8 EMULATOR STARTED ===\n");
    printf("Controls:\n");
    printf("  1 2 3 4  ->  1 2 3 C\n");
    printf("  Q W E R  ->  4 5 6 D\n");
    printf("  A S D F  ->  7 8 9 E\n");
    printf("  Z X C V  ->  A 0 B F\n");
    printf("Press ESC to exit\n\n");
    
    // Timing variables
    uint32_t last_cycle_time = SDL_GetTicks();
    uint32_t last_timer_time = SDL_GetTicks();
    const uint32_t CYCLE_DELAY_MS = 2;    // 2ms per cycle (~500 Hz)
    const uint32_t TIMER_DELAY_MS = 16;   // 16ms = ~60Hz
    
    // Main loop
    while (chip8.running) {
        uint32_t current_time = SDL_GetTicks();
        
        // Handle input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                chip8.running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    chip8.running = false;
                }
                uint8_t key = keymap(event.key.keysym.sym);
                if (key != 0xFF) {
                    chip8.key[key] = 1;
                }
            }
            else if (event.type == SDL_KEYUP) {
                uint8_t key = keymap(event.key.keysym.sym);
                if (key != 0xFF) {
                    chip8.key[key] = 0;
                }
            }
        }
        
        // Execute CHIP-8 cycles with timing
        if (current_time - last_cycle_time >= CYCLE_DELAY_MS) {
            chip8_cycle(&chip8);
            last_cycle_time = current_time;
        }
        
        // Update timers at 60Hz
        if (current_time - last_timer_time >= TIMER_DELAY_MS) {
            if (chip8.delay_timer > 0) chip8.delay_timer--;
            if (chip8.sound_timer > 0) chip8.sound_timer--;
            last_timer_time = current_time;
        }
        
        // Render screen if needed
        if (chip8.drawFlag) {
            render_screen(&chip8);
            chip8.drawFlag = false;
        }
        
        // Small delay to prevent CPU hogging
        SDL_Delay(1);
    }
    
    cleanup_sdl();
    printf("Emulator closed.\n");
    
    return 0;
}
