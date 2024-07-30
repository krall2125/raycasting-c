#include "game.h"
#include "rendering.h"
#include "vec2.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <raycast.h>
#include <stdint.h>
#include <stdio.h>

float dist(vec2_t a, vec2_t b, float ang) {
    return ( sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)) );
}

void horizontal_check(player_t *player, game_t *game, int map[32][32], float ray_angle, int dof, int mx, int my, int mp, vec2_t ro, vec2_t ray, vec2_t *pos_h, color_t *colord_h) {
    dof = 0;
    float aTan = -1 / tan(ray_angle);

    if (ray_angle > PI) {
        ray.y = (((int) player->pos.y >> 6) << 6) - 0.0001;
        ray.x = (player->pos.y - ray.y) * aTan + player->pos.x;
        ro.y = -64;
        ro.x = -ro.y * aTan;
    }
    else if (ray_angle < PI) {
        ray.y = (((int) player->pos.y >> 6) << 6) + 64;
        ray.x = (player->pos.y - ray.y) * aTan + player->pos.x;
        ro.y = 64;
        ro.x = -ro.y * aTan;
    }
    else {
        ray.x = player->pos.x;
        ray.y = player->pos.y;
        dof = 8;
    }

    while (dof < 8) {
        mx = (int) (ray.x) >> 6;
        my = (int) (ray.y) >> 6;
        mp = my * 32 + mx;

        if (mx <= -INT8_MAX || mx >= INT8_MAX) {
            pos_h->x = ray.x;
            pos_h->y = ray.y;

            colord_h->color = 0x009900FF;

            dof = 8;
            break;
        }

        if (my <= -INT8_MAX || my >= INT8_MAX) {
            pos_h->x = ray.x;
            pos_h->y = ray.y;

            colord_h->color = 0x009900FF;

            colord_h->glow = 0;

            dof = 8;
            break;
        }

        if (mx < 0 || mx >= 32 || my < 0 || my >= 32) {
            ray.x += ro.x;
            ray.y += ro.y;
            continue;
        }

        switch (map[my][mx]) {
            case 1:
                pos_h->x = ray.x;
                pos_h->y = ray.y;

                colord_h->color = 0x009900FF;

                colord_h->glow = 0.5;

                dof = 8;
                break;
            case 2:
                pos_h->x = ray.x;
                pos_h->y = ray.y;

                colord_h->color = 0x999999FF;

                colord_h->glow = 1;

                dof = 8;
            case 3:
                pos_h->x = ray.x;
                pos_h->y = ray.y;

                colord_h->color = 0x880000FF;

                colord_h->glow = 0.5;

                dof = 8;
                break; 
            default:
                ray.x += ro.x;
                ray.y += ro.y;
                break;
        }
    }
}

void vertical_check(player_t *player, game_t *game, int map[32][32], float ray_angle, int dof, int mx, int my, int mp, vec2_t ro, vec2_t ray, vec2_t *pos_v, color_t *colord_v) {
    dof = 0;
    float nTan = -tan(ray_angle);

    if (ray_angle > PI/2 && ray_angle < 3*PI/2) {
        ray.x = (((int) player->pos.x >> 6) << 6) - 0.0001;
        ray.y = (player->pos.x - ray.x) * nTan + player->pos.y;
        ro.x = -64;
        ro.y = -ro.x * nTan;
    }
    else if (ray_angle < PI/2 || ray_angle > 3*PI/2) {
        ray.x = (((int) player->pos.x >> 6) << 6) + 64;
        ray.y = (player->pos.x - ray.x) * nTan + player->pos.y;
        ro.x = 64;
        ro.y = -ro.x * nTan;
    }
    else {
        ray.x = player->pos.x;
        ray.y = player->pos.y;
        dof = 8;
    }

    while (dof < 8) {
        mx = (int) (ray.x) >> 6;
        my = (int) (ray.y) >> 6;
        mp = my * 32 + mx;

        if (mx <= -INT8_MAX || mx >= INT8_MAX) {
            pos_v->x = ray.x;
            pos_v->y = ray.y;

            colord_v->color = 0x00FF00FF;

            dof = 8;
            break;
        }

        if (my <= -INT8_MAX || my >= INT8_MAX) {
            pos_v->x = ray.x;
            pos_v->y = ray.y;

            colord_v->color = 0x00FF00FF;

            colord_v->glow = 0;

            dof = 8;
            break;
        }

        if (mx < 0 || mx >= 32 || my < 0 || my >= 32) {
            ray.x += ro.x;
            ray.y += ro.y;
            continue;
        }

        switch (map[my][mx]) {
            case 1:
                pos_v->x = ray.x;
                pos_v->y = ray.y;

                colord_v->color = 0x00FF00FF;

                colord_v->glow = 1.0;

                dof = 8;

                break;
            case 2:
                pos_v->x = ray.x;
                pos_v->y = ray.y;

                colord_v->color = 0xFFFFFFFF;

                colord_v->glow = 1;

                dof = 8;

                break;
            case 3:
                pos_v->x = ray.x;
                pos_v->y = ray.y;

                colord_v->color = 0xFF0000FF;

                colord_v->glow = 1.0;

                dof = 8;

                break; 
            default:
                ray.x += ro.x;
                ray.y += ro.y;
                break;
        }
    }
}

#define LINE_COUNT 4
#define RESOLUTION 1

void draw_line(game_t *game, player_t *player, float line_h, float line_o, int r, float dis, color_t color) {
    dis /= 16;
    for (int i = 0; i < RESOLUTION; i++) {
        // SDL_RenderDrawLine(game->renderer, r * 1 + i, line_o, r * 1 + i, line_o + line_h);
        SDL_SetRenderDrawColor(game->renderer, color.color >> 24, color.color >> 16 & 0x000000FF, color.color >> 8 & 0x000000FF, 255);

        SDL_RenderDrawLine(game->renderer,
            // ((float) WIDTH / 2) +
            //     (r < (int) (player->fov * FOV_MULTIPLIER) / 2 ? -((int) (player->fov * FOV_MULTIPLIER) / 2 - r) :
            //         (r == (int) (player->fov * FOV_MULTIPLIER) / 2 ? 0 : r - (int) (player->fov * FOV_MULTIPLIER) / 2)) * LINE_COUNT + i,
            r + i,
            
            line_o,
                    
            // ((float) WIDTH / 2) +
            //     (r < (int) (player->fov * FOV_MULTIPLIER) / 2 ? -((int) (player->fov * FOV_MULTIPLIER) / 2 - r) :
            //         (r == (int) (player->fov * FOV_MULTIPLIER) / 2 ? 0 : r - (int) (player->fov * FOV_MULTIPLIER) / 2)) * LINE_COUNT + i,
            r + i,

            line_o + line_h);

        // render_line(game,
        //         ((float) WIDTH / 2) +
        //             (r < (int) (player->fov * FOV_MULTIPLIER) / 2 ? -((int) (player->fov * FOV_MULTIPLIER) / 2 - r) :
        //                 (r == (int) (player->fov * FOV_MULTIPLIER) / 2 ? 0 : r - (int) (player->fov * FOV_MULTIPLIER) / 2)) * LINE_COUNT + i,

        //         line_o,

        //         ((float) WIDTH / 2) +
        //             (r < (int) (player->fov * FOV_MULTIPLIER) / 2 ? -((int) (player->fov * FOV_MULTIPLIER) / 2 - r) :
        //                 (r == (int) (player->fov * FOV_MULTIPLIER) / 2 ? 0 : r - (int) (player->fov * FOV_MULTIPLIER) / 2)) * LINE_COUNT + i,

        //         line_o + line_h, color.color);
    }
}

void raycast(game_t *game, player_t *player, int map[32][32]) {
    float ray_angle = player->yaw - RAD * (player->fov / 2);

    if (ray_angle < 0) {
        ray_angle += 2 * PI;
    }
    if (ray_angle > 2 * PI) {
        ray_angle -= 2 * PI;
    }

    vec2_t ray, ro, pos_h, pos_v;

    int dof, mx, my, mp;

    float dis_actual;

    color_t colorh, colorv, actual_c;

    int x = 0;

    for (int r = 0; r < WIDTH / RESOLUTION; r++) {
        float dis_h = 1000000, dis_v = 1000000;

        pos_v.x = player->pos.x;
        pos_v.y = player->pos.y;
        vertical_check(player, game, map, ray_angle, dof, mx, my, mp, ro, ray, &pos_v, &colorv);
        dis_v = dist(player->pos, pos_v, ray_angle);

        pos_h.x = player->pos.x;
        pos_h.y = player->pos.y;
        horizontal_check(player, game, map, ray_angle, dof, mx, my, mp, ro, ray, &pos_h, &colorh);
        dis_h = dist(player->pos, pos_h, ray_angle);

        if (dis_v < dis_h) {
            ray.x = pos_v.x;
            ray.y = pos_v.y;
            dis_actual = dis_v;
            actual_c = colorv;
        }
        else if (dis_v > dis_h) {
            ray.x = pos_h.x;
            ray.y = pos_h.y;
            dis_actual = dis_h;
            actual_c = colorh;
        }
        else {
            ray.x = pos_v.x;
            ray.y = pos_v.y;
            dis_actual = dis_v;
            actual_c = colorv;
        }

        // SDL_SetRenderDrawColor(game->renderer, actual_c.color >> 24, (actual_c.color >> 16) & 0x000000FF, (actual_c.color >> 8) & 0x000000FF, actual_c.color & 0x000000FF);

        // SDL_RenderDrawLine(game->renderer, player->pos.x + 4, player->pos.y + 4, ray.x + 4, ray.y + 4);
        if (ray_angle < 0) {
            ray_angle += 2 * PI;
        }
        if (ray_angle > 2 * PI) {
            ray_angle -= 2 * PI;
        }


        float line_h = HEIGHT * 5024 / (dis_actual * player->fov);
        float line_o = (float) HEIGHT / 2 - line_h / 2;
        
        if (line_h > HEIGHT) {
            line_h = HEIGHT;
        }

        if (line_o > HEIGHT) {
            line_o = HEIGHT;
        }

        if (line_o < 0) {
            line_o = 0;
        }

        draw_line(game, player, line_h, line_o, x, dis_actual, actual_c);

        // render_line(game, player->pos.x + 4, player->pos.y + 4, ray.x + 4, ray.y + 4, 0xFFFFFFFF);

        ray_angle += RAD * (player->fov / (int) (WIDTH / RESOLUTION));
        x += RESOLUTION;
    }
}
