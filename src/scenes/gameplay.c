/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdlib.h>
#include "game.h"
#include "scene.h"
#include "world/map/map.h"
#include "world/entity/player.h"

#ifdef PLATFORM_ANDROID
#include "ui/virtual_joystick.h"
#endif // PLATFORM_ANDROID

#define GAMEPLAY_TILE_SIZE 64.0

struct scene_data {
    map_t map;
    player_t player;

#ifdef PLATFORM_ANDROID
    virtual_joystick_t virtual_joystick;
#endif // PLATFORM_ANDROID

    Rectangle camera;
    Texture spritesheet;
};

scene_data_t *gameplay_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    map_load(&data->map);
    player_load(&data->player, data->map);

#ifdef PLATFORM_ANDROID
    virtual_joystick_init(&data->virtual_joystick, 125, 175, game_height() - 175);
#endif // PLATFORM_ANDROID

    data->camera = (Rectangle) {
        .x = 0,
        .y = 0,

        // +1 because it's needed to draw one line/column more to fill gaps when
        // drawing a camera that hasn't a integer value.
        .width = ceil((float) game_width() / GAMEPLAY_TILE_SIZE) + 1,
        .height = ceil((float) game_height() / GAMEPLAY_TILE_SIZE) + 1,
    };

    data->spritesheet = game_get_texture("map-sprites");

    return data;
}

void gameplay_deinit(scene_data_t *data)
{
    map_save(&data->map);
    map_destroy(&data->map);

    player_save(&data->player);

    free(data);
}


void gameplay_update(scene_data_t *data)
{
    Vector2 direction = { 0, 0 };

#ifdef PLATFORM_ANDROID
    direction = virtual_joystick_update(&data->virtual_joystick);
#else
    if (IsKeyDown(KEY_W))
        direction.y = 1;
    else if (IsKeyDown(KEY_S))
        direction.y = -1;

    if (IsKeyDown(KEY_D))
        direction.x = 1;
    else if (IsKeyDown(KEY_A))
        direction.x = -1;
#endif // PLATFORM_ANDROID

    player_update(&data->player, direction);

    // Update the game camera
    data->camera.x = data->player.position.x + 1 - data->camera.width / 2;
    data->camera.y = data->player.position.y + 1 - data->camera.height / 2;

    if (data->camera.x < 0)
        data->camera.x = 0;
    else if (data->camera.x >= data->map.width - data->camera.width)
        data->camera.x = data->map.width - data->camera.width;

    if (data->camera.y < 0)
        data->camera.y = 0;
    else if (data->camera.y >= data->map.height - data->camera.height)
        data->camera.y = data->map.height - data->camera.height;
}

void gameplay_draw(scene_data_t *data)
{
    int camera_x, camera_y;
    int player_x, player_y;
    int tree_alpha;

    Rectangle tile = {
        .width = GAMEPLAY_TILE_SIZE,
        .height = GAMEPLAY_TILE_SIZE,
    };

    Rectangle sprite = {
        .width = 16,
        .height = 16,
    };

    Vector2 tile_rotation_origin = {
        .x = GAMEPLAY_TILE_SIZE / 2.0,
        .y = GAMEPLAY_TILE_SIZE / 2.0,
    };

    ClearBackground(BLACK);

    player_x = data->player.position.x;
    player_y = data->player.position.y;

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        for (int y = 0; y < data->camera.height; y++) {
            camera_y = y + data->camera.y;

            for (int x = 0; x < data->camera.width; x++) {
                tree_alpha = 255;
                camera_x = x + data->camera.x;

                if (layer == 1) {
                    // Draw the player
                    if (player_x == camera_x && player_y == camera_y)
                        player_draw(&data->player, &data->camera);

                    // Check if the player is behind a tree
                    if ((player_x == camera_x || player_x == camera_x - 1)
                        && (player_y == camera_y || player_y - 1 == camera_y
                            || player_y + 1 == camera_y)
                        && TILE_IS_EQUAL(data->map.tiles[1][camera_y][camera_x],
                            TILE_NEW(16, 10, 0)))
                        tree_alpha = 200;
                    else if ((player_x == camera_x || player_x == camera_x - 1)
                        && (player_y == camera_y || player_y + 1 == camera_y
                            || player_y + 2 == camera_y)
                        && TILE_IS_EQUAL(data->map.tiles[1][camera_y][camera_x],
                            TILE_NEW(16, 11, 0)))
                        tree_alpha = 200;

                }

                if (TILE_IS_EMPTY(data->map.tiles[layer][camera_y][camera_x]))
                    continue;

                // TODO: Maybe fix a little that appear when move the camera,
                // some thin black lines around all tiles.
                tile.x = tile_rotation_origin.x
                    + (camera_x - data->camera.x) * tile.width;

                tile.y = tile_rotation_origin.y
                    + (camera_y - data->camera.y) * tile.height;

                sprite.x = TILE_GET_X(data->map.tiles[layer][camera_y][camera_x]);
                sprite.x = sprite.x * sprite.width + 1 * sprite.x;

                sprite.y = TILE_GET_Y(data->map.tiles[layer][camera_y][camera_x]);
                sprite.y = sprite.y * sprite.height + 1 * sprite.y;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    tile_rotation_origin,
                    TILE_GET_ROTATION(data->map.tiles[layer][camera_y][camera_x]),
                    (Color) { 255, 255, 255, tree_alpha });
            }
        }
    }

#ifdef PLATFORM_ANDROID
    virtual_joystick_draw(&data->virtual_joystick);
#endif // PLATFORM_ANDROID
}

