#include "game.h"
#include <rendering.h>
#include <stdint.h>

void render_point(game_t *game, int x, int y, register uint32_t color) {
    if ((color & 0xFFFFFF00) == 0)
        return;

    if (x > WIDTH || y > HEIGHT)
        return;

    if (x < 0 || y < 0) {
        return;
    }

    if (y * WIDTH + x >= WIDTH * HEIGHT) {
        return;
    }

    game->pixels[y * WIDTH + x] = color;
}

void render_line(game_t *game, register int x0, register int y0, int x1, int y1, register uint32_t color) {
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;){
        if (x0==x1 && y0==y1) break;
        if ((x1 >= WIDTH || y1 >= HEIGHT) && (x0 >= WIDTH || y0 >= HEIGHT)) break;
        
        render_point(game, x0, y0, color);

        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}
