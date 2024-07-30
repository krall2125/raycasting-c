#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>

#include <game.h>

int main() {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            printf("%d ", map_data[i][j]);
        }
        printf("\n");
    }

    game_t *game = init_game("Raycasting in C", WIDTH, HEIGHT);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);

    player_t *player = malloc(sizeof(player_t));

    player->pos.x = 76.f;
    player->pos.y = 76.f;

    player->fov = 60.f;
    player->yaw = 90 * RAD;

    player->delta.x = cos((double) player->yaw) / 48;
    player->delta.y = sin((double) player->yaw) / 48;

    loop(game, player);

    SDL_DestroyWindow(game->window);
    SDL_DestroyRenderer(game->renderer);

    free(game->pixels);

    free(game);

    free(player);

    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    SDL_Quit();

    return 0;
}
