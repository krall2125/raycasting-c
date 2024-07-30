#include "raycast.h"
#include "rendering.h"
#include "vec2.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <game.h>
#include <string.h>
#include <sys/types.h>

float clamp_angle(float angle) {
    if (angle > 2 * PI) {
        angle -= 2 * PI;
    }
    else if (angle < 0) {
        angle += 2 * PI;
    }

    return angle;
}

game_t *init_game(const char *title, int width, int height) {
    int rflags, wflags;

    rflags = SDL_RENDERER_ACCELERATED;

    wflags = SDL_WINDOW_BORDERLESS;

    game_t *game = malloc(sizeof(game_t));

    if (!game) {
        fprintf(stderr, "Couldn't allocate enough memory for the game!\n");
        exit(1);
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    game->window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            wflags);

    if (!game->window) {
        fprintf(stderr, "Couldn't open window: %s\n", SDL_GetError());
        free(game);
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    game->renderer = SDL_CreateRenderer(game->window, -1, rflags);

    if (!game->renderer) {
        fprintf(stderr, "Couldn't initialize renderer: %s\n", SDL_GetError());
        free(game);
        exit(1);
    }

    game->pixels = malloc(WIDTH * HEIGHT * sizeof(uint32_t));

    IMG_Init(IMG_INIT_PNG);

    return game;
}

void render_prepare(game_t *game) {
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
}

void render_present(game_t *game) {
    SDL_RenderPresent(game->renderer);
}

SDL_Surface *load_texture(const char *filename) {
    SDL_Surface *surface;

    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading texture: %s", filename);

    surface = IMG_Load(filename);

    return surface;
}

void draw_player(game_t *game, player_t *player) {
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    if (player->pos.x >= WIDTH || player->pos.y >= HEIGHT) return;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            SDL_RenderDrawPoint(game->renderer, (int) player->pos.x + i, (int) player->pos.y + j);
        }
    }

    SDL_RenderDrawLine(game->renderer, player->pos.x + 4, player->pos.y + 4, (player->pos.x + 4) + cos(player->yaw) * 48, (player->pos.y + 4) + sin(player->yaw) * 48);
}

void draw_box(game_t *game, int xo, int yo) {
    SDL_SetRenderDrawColor(game->renderer, 0, 255, 0, 255);
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (xo * 64 + x > WIDTH) return;
            if (yo * 64 + y > HEIGHT + 128) return;
            SDL_RenderDrawPoint(game->renderer, xo * 64 + x, yo * 64 + y);
        }
    }
}

void draw_map(int map[32][32], game_t *game) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            if (!map[i][j]) {
                continue;
            }
            draw_box(game, j, i);
        }
    }
}

void draw_grid(game_t *game) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            SDL_RenderDrawLine(game->renderer, i * 64, j * 64, (i + 1) * 64, j * 64);
            SDL_RenderDrawLine(game->renderer, i * 64, j * 64, i * 64, (j + 1) * 64);
        }
    }
}

void key_input(game_t *game, player_t *player) {
    const uint8_t *keys = SDL_GetKeyboardState(NULL);

    const uint8_t mouse = SDL_GetMouseState(NULL, NULL);

    int xo = 0;
    if (player->delta.x < 0) {
        xo = -5;
    }
    else {
        xo = 5;
    }

    int yo = 0;
    if (player->delta.y < 0) {
        yo = -5;
    }
    else {
        yo = 5;
    }

    int ipx = player->pos.x / 64.0,
        ipx_add_xo = (player->pos.x + xo) / 64.0,
        ipx_sub_xo = (player->pos.x - xo) / 64.0;

    int ipy = player->pos.y / 64.0,
        ipy_add_yo = (player->pos.y + yo) / 64.0,
        ipy_sub_yo = (player->pos.y - yo) / 64.0;

    float side_dx = cos(clamp_angle(player->yaw - 90 * RAD)) / 48,
          side_dy = sin(clamp_angle(player->yaw - 90 * RAD)) / 48;

    int side_xo = 0;
    if (side_dx < 0) {
        side_xo = -5;
    }
    else {
        side_xo = 5;
    }

    int side_yo = 0;
    if (side_dy < 0) {
        side_yo = -5;
    }
    else {
        side_yo = 5;
    }

    int side_ipx = player->pos.x / 64.0,
        side_ipx_add_xo = (player->pos.x + side_xo) / 64.0,
        side_ipx_sub_xo = (player->pos.x - side_xo) / 64.0;

    int side_ipy = player->pos.y / 64.0,
        side_ipy_add_yo = (player->pos.y + side_yo) / 64.0,
        side_ipy_sub_yo = (player->pos.y - side_yo) / 64.0;

#define SPEED 10

    if (keys[SDL_SCANCODE_A]) {
        if (side_ipy < 0 || side_ipy >= 32 || side_ipx_add_xo < 0 || side_ipx_add_xo >= 32 || map_data[side_ipy][side_ipx_add_xo] == 0) {
            player->pos.x += side_dx * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
        if (side_ipy_add_yo < 0 || side_ipy_add_yo >= 32 || side_ipx < 0 || side_ipx >= 32 || map_data[side_ipy_add_yo][side_ipx] == 0) {
            player->pos.y += side_dy * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
    }
    
    if (keys[SDL_SCANCODE_D]) {
        if (side_ipy < 0 || side_ipy >= 32 || side_ipx_sub_xo < 0 || side_ipx_sub_xo >= 32 || map_data[side_ipy][side_ipx_sub_xo] == 0) {
            player->pos.x -= side_dx * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
        if (side_ipy_sub_yo < 0 || side_ipy_sub_yo >= 32 || side_ipx < 0 || side_ipx >= 32 || map_data[side_ipy_sub_yo][side_ipx] == 0) {
            player->pos.y -= side_dy * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
    }

    if (keys[SDL_SCANCODE_W]) {
        if (ipy < 0 || ipy >= 32 || ipx_add_xo < 0 || ipx_add_xo >= 32 || map_data[ipy][ipx_add_xo] == 0) {
            player->pos.x += player->delta.x * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
        if (ipy_add_yo < 0 || ipy_add_yo >= 32 || ipx < 0 || ipx >= 32 || map_data[ipy_add_yo][ipx] == 0) {
            player->pos.y += player->delta.y * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
    }

    if (keys[SDL_SCANCODE_S]) {
        if (ipy < 0 || ipy >= 32 || ipx_sub_xo < 0 || ipx_sub_xo >= 32 || map_data[ipy][ipx_sub_xo] == 0) {
            player->pos.x -= player->delta.x * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
        if (ipy_sub_yo < 0 || ipy_sub_yo >= 32 || ipx < 0 || ipx >= 32 || map_data[ipy_sub_yo][ipx] == 0) {
            player->pos.y -= player->delta.y * (4 + SPEED * keys[SDL_SCANCODE_LSHIFT]) * game->fps;
        }
    }
#undef SPEED
}

void mouse_move(game_t *game, player_t *player, int xrel, int yrel) {
    player->yaw += (float) xrel / 1000;
    player->pitch += (float) yrel / 1000;
    player->delta.x = cos(player->yaw) / 48;
    player->delta.y = sin(player->yaw) / 48;
}

void loop(game_t *game, player_t *player) {
    SDL_Event event;

    uint32_t frame1, frame2;
    float fps;

    SDL_Texture *tex = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    while (1) {
        frame2 = SDL_GetTicks();
        fps = frame2 - frame1;
        frame1 = SDL_GetTicks();
        game->fps = fps;

        memset(game->pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));

        render_prepare(game);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    printf("Quitting!\n");
                    SDL_DestroyTexture(tex);
                    return;

                case SDL_MOUSEMOTION:
                    mouse_move(game, player, event.motion.xrel, event.motion.yrel);
                    break;

                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(game->renderer, 80, 80, 80, 255);

        for (int i = 0; i < HEIGHT / 2; i++) {
            SDL_RenderDrawLine(game->renderer, 0, i, WIDTH, i);
        }

        SDL_SetRenderDrawColor(game->renderer, 50, 50, 50, 255);

        for (int i = 0; i < HEIGHT / 2; i++) {
            SDL_RenderDrawLine(game->renderer, 0, HEIGHT / 2 + i, WIDTH, HEIGHT / 2 + i);
        }

        key_input(game, player);

        player->yaw = clamp_angle(player->yaw);

        // render_point(game, WIDTH / 2, HEIGHT / 2, 0xFF0000FF);
 
        raycast(game, player, map_data);

        // draw_map(map_data, game);
        
        // draw_grid(game);

        // draw_player(game, player);

        // void *px;
        // int pitch;
        // SDL_LockTexture(tex, NULL, &px, &pitch);
        // {
        //     for (size_t y = 0; y < HEIGHT; y++) {
        //         memcpy(
        //             &((uint8_t*) px)[y * pitch],
        //             &game->pixels[y * WIDTH],
        //             WIDTH * sizeof(uint32_t));
        //     }
        // }
        // SDL_UnlockTexture(tex);
  
        // SDL_SetRenderTarget(game->renderer, NULL);
        // SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 0xFF);
  
        SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
        // SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  
        // SDL_RenderCopyEx(game->renderer, tex, NULL, NULL, 0.0, NULL, SDL_FLIP_NONE);
        
        render_present(game);
    }

    SDL_DestroyTexture(tex);
}
