#pragma once

#include "game.h"
#include <stdint.h>

void render_point(game_t *game, int x, int y, register uint32_t color);
void render_line(game_t *game, register int x0, register int y0, int x1, int y1, register uint32_t color);
