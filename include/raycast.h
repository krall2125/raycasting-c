#pragma once

#include "vec2.h"
#include <game.h>
#include <math.h>

float dist(vec2_t a, vec2_t b, float ang);

typedef struct colordata {
    uint32_t color;
    float glow;
} color_t;

void draw_line(game_t *game, player_t *player, float line_h, float line_o, int r, float dis, color_t color);

void horizontal_check(player_t *player, game_t *game, int map[32][32], float ray_angle, int dof, int mx, int my, int mp, vec2_t ro, vec2_t ray, vec2_t *pos_h, color_t *colord_v);

void vertical_check(player_t *player, game_t *game, int map[32][32], float ray_angle, int dof, int mx, int my, int mp, vec2_t ro, vec2_t ray, vec2_t *pos_v, color_t *colord_h);

void raycast(game_t *game, player_t *player, int map[32][32]);
