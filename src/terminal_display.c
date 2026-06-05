#include <SDL2/SDL.h>
#include "chip8.h"

#define WINDOW_WIDTH 640   // 64 * 10
#define WINDOW_HEIGHT 320  // 32 * 10
#define PIXEL_SIZE 10

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int init_display(void) {
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

void render_display(Chip8 *chip8) {
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

void close_display(void) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void handle_input(Chip8 *chip8) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            chip8->running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            // Mapping keyboard PC ke CHIP-8 keypad
            switch (event.key.keysym.sym) {
                case SDLK_1: chip8->key[0x1] = 1; break;
                case SDLK_2: chip8->key[0x2] = 1; break;
                case SDLK_3: chip8->key[0x3] = 1; break;
                case SDLK_4: chip8->key[0xC] = 1; break;
                case SDLK_q: chip8->key[0x4] = 1; break;
                case SDLK_w: chip8->key[0x5] = 1; break;
                case SDLK_e: chip8->key[0x6] = 1; break;
                case SDLK_r: chip8->key[0xD] = 1; break;
                case SDLK_a: chip8->key[0x7] = 1; break;
                case SDLK_s: chip8->key[0x8] = 1; break;
                case SDLK_d: chip8->key[0x9] = 1; break;
                case SDLK_f: chip8->key[0xE] = 1; break;
                case SDLK_z: chip8->key[0xA] = 1; break;
                case SDLK_x: chip8->key[0x0] = 1; break;
                case SDLK_c: chip8->key[0xB] = 1; break;
                case SDLK_v: chip8->key[0xF] = 1; break;
            }
        }
        else if (event.type == SDL_KEYUP) {
            // Reset key state
            switch (event.key.keysym.sym) {
                case SDLK_1: chip8->key[0x1] = 0; break;
                case SDLK_2: chip8->key[0x2] = 0; break;
                case SDLK_3: chip8->key[0x3] = 0; break;
                case SDLK_4: chip8->key[0xC] = 0; break;
                case SDLK_q: chip8->key[0x4] = 0; break;
                case SDLK_w: chip8->key[0x5] = 0; break;
                case SDLK_e: chip8->key[0x6] = 0; break;
                case SDLK_r: chip8->key[0xD] = 0; break;
                case SDLK_a: chip8->key[0x7] = 0; break;
                case SDLK_s: chip8->key[0x8] = 0; break;
                case SDLK_d: chip8->key[0x9] = 0; break;
                case SDLK_f: chip8->key[0xE] = 0; break;
                case SDLK_z: chip8->key[0xA] = 0; break;
                case SDLK_x: chip8->key[0x0] = 0; break;
                case SDLK_c: chip8->key[0xB] = 0; break;
                case SDLK_v: chip8->key[0xF] = 0; break;
            }
        }
    }
}
