/*
The GPLv3 License (GPLv3)

Copyright (c) 2022 Jonatha Gabriel <jonathagabrielns@gmail.com>

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
#include "utils/utils.h"
#include "utils/list.h"
#include "world/map/map.h"
#include "world/map/tile.h"
#include "world/entity/spawner.h"
#include "world/entity/player.h"

#ifdef PLATFORM_ANDROID
#include "ui/virtual_joystick.h"
#endif // PLATFORM_ANDROID

#define GAMEPLAY_LOAD_STAGES 5

struct scene_data {
    map_t map;

    // The first entity its always the player.
    entity_list_t entities;

    spawner_list_t spawners;

#ifdef PLATFORM_ANDROID
    virtual_joystick_t virtual_joystick;

    Texture attack;
    Rectangle attack_button;
#endif // PLATFORM_ANDROID

    Rectangle camera;
    Texture spritesheet;

    Texture pause;
    Texture unpause;
    Rectangle pause_button;

    Texture save;
    Rectangle save_button;

    Texture back;
    Rectangle back_button;

    bool paused;
    int loading_stage;
    int saving;
};

static void update_loading(scene_data_t *data);
static void update_game(scene_data_t *data);

static void draw_loading(scene_data_t *data);
static void draw_game(scene_data_t *data);

scene_data_t *gameplay_init(void)
{
    scene_data_t *data = malloc(sizeof(scene_data_t));

    data->loading_stage = 0;

    list_create(data->entities);

    data->camera = (Rectangle) {
        .x = 0,
        .y = 0,

        // +1 because it's needed to draw one line/column more to fill gaps when
        // drawing a camera that hasn't a integer value.
        .width = ceil((float) game_width() / TILE_DRAW_SIZE) + 1,
        .height = ceil((float) game_height() / TILE_DRAW_SIZE) + 1,
    };

    data->spritesheet = game_get_texture("tiles");

    data->pause = game_get_texture("pause-img");
    data->unpause = game_get_texture("unpause-img");
    data->pause_button = (Rectangle) {
        .x = game_width() - (game_width() / 20) * 1.2,
        .y = (game_width() / 20) * 0.2,

        .width = game_width() / 20,
        .height = game_width() / 20,
    };

    data->save = game_get_texture("save-img");
    data->save_button = (Rectangle) {
        .x = game_width() - (game_width() / 20) * 2.4,
        .y = (game_width() / 20) * 0.2,

        .width = game_width() / 20,
        .height = game_width() / 20,
    };

    data->back = game_get_texture("back-img");
    data->back_button = (Rectangle) {
        .x = (game_width() / 20) * 0.2,
        .y = (game_width() / 20) * 0.2,

        .width = game_width() / 20,
        .height = game_width() / 20,
    };

    data->paused = false;
    data->saving = 0;

#ifdef PLATFORM_ANDROID
    virtual_joystick_init(&data->virtual_joystick, 125, 175, game_height() - 175);

    data->attack = game_get_texture("attack-img");
    data->attack_button = (Rectangle) {
        .x = game_width() - 280,
        .y = game_height() - 250,

        .width = game_width() / 10,
        .height = game_width() / 10,
    };
#endif // PLATFORM_ANDROID

    return data;
}

void gameplay_deinit(scene_data_t *data)
{
    map_save(&data->map);
    player_save((player_t *) list_get(data->entities, 0));
    spawner_save(&data->spawners);

    map_destroy(&data->map);
    entity_destroy(&data->entities);
    spawner_destroy(&data->spawners);

    free(data);
}


void gameplay_update(scene_data_t *data)
{
    if (data->loading_stage < GAMEPLAY_LOAD_STAGES)
        update_loading(data);
    else
        update_game(data);
}

void gameplay_draw(scene_data_t *data)
{
    if (data->loading_stage < GAMEPLAY_LOAD_STAGES)
        draw_loading(data);
    else
        draw_game(data);
}

static void update_loading(scene_data_t *data)
{
    switch (data->loading_stage) {
    // Load map dimensions
    case 0:
        map_load(&data->map, MAP_LOAD_DIMENSIONS);
        break;

    // Load map layer 0
    case 1:
        map_load(&data->map, MAP_LOAD_LAYER_0);
        break;

    // Load map layer 1
    case 2:
        map_load(&data->map, MAP_LOAD_LAYER_1);
        break;

    // Load player state
    case 3:
        list_add(data->entities, (entity_t *) player_create((Vector2) { 0, 0 }));
        player_load((player_t *) list_get(data->entities, 0));
        break;

    // Load spawners
    case 4:
        spawner_create(&data->spawners);
        spawner_load(&data->spawners);
        break;
    }

    data->loading_stage++;
}

static void update_game(scene_data_t *data)
{
    Vector2 direction = { 0, 0 };
    player_t *player = (player_t *) list_get(data->entities, 0);

    if (!data->paused && data->saving == 0) {
#ifdef PLATFORM_ANDROID
        direction = virtual_joystick_update(&data->virtual_joystick);

        for (int i = 0; i < min(GetTouchPointCount(), 2); i++)
            if (CheckCollisionPointRec(game_virtual_touch(i),
                        data->attack_button)
                    && game_touch_pressed(game_touch_id(i))
                    && !player->attacked)
                player->attacking = GetTime();

        for (int i = 0; i < 2; i++)
            if (CheckCollisionPointRec(game_virtual_touch(i),
                        data->attack_button)
                    && game_touch_released(game_touch_id(i)))
                player->attacked = false;
#else
        if (IsKeyDown(KEY_W))
            direction.y = -1;
        else if (IsKeyDown(KEY_S))
            direction.y = 1;

        if (IsKeyDown(KEY_D))
            direction.x = 1;
        else if (IsKeyDown(KEY_A))
            direction.x = -1;

        if (IsKeyPressed(KEY_E) && !player->attacked)
            player->attacking = GetTime();
        else if (IsKeyReleased(KEY_E))
            player->attacked = false;
#endif // PLATFORM_ANDROID

        // Update the player state
        if ((direction.x != 0 || direction.y != 0)
                && player->base.state != ENTITY_STATE_DAMAGING) {
            player->base.direction = vec2ang(direction.x, direction.y);
            player->base.state = ENTITY_STATE_MOVING;
        } else if (player->base.state != ENTITY_STATE_DAMAGING) {
            player->base.state = ENTITY_STATE_IDLE;
        }

        // Update the game camera
        // NOTE: The +1 its to really centralize the camera.
        data->camera.x = player->base.position.x + 1
            - data->camera.width / 2;

        data->camera.y = player->base.position.y + 1
            - data->camera.height / 2;

        if (data->camera.x < 0)
            data->camera.x = 0;
        else if (data->camera.x >= data->map.width - data->camera.width)
            data->camera.x = data->map.width - data->camera.width;

        if (data->camera.y < 0)
            data->camera.y = 0;
        else if (data->camera.y >= data->map.height - data->camera.height)
            data->camera.y = data->map.height - data->camera.height;

        entity_update(&data->entities, &data->map, data->camera);
        spawner_update(&data->spawners, &data->entities);

        if (CheckCollisionPointRec(game_virtual_mouse(), data->save_button)
                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            data->saving = 3;
    }

    switch (data->saving) {
    case 1:
        map_save(&data->map);
        break;

    case 2:
        player_save(player);
        break;

    case 3:
        spawner_save(&data->spawners);
        break;
    }
    data->saving -= data->saving > 0;

    if (data->saving == 0) {
        if (CheckCollisionPointRec(game_virtual_mouse(), data->pause_button)
                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            data->paused = !data->paused;

        if (CheckCollisionPointRec(game_virtual_mouse(), data->back_button)
                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            game_set_scene("menu");
    }

    if (player->base.hearts <= 0)
        game_set_scene("gameover");
}

static void draw_loading(scene_data_t *data)
{
    switch (data->loading_stage) {
    // Draw loading map dimensions
    case 0:
        break;

    // Draw loading map layer 0
    case 1:
        break;

    // Draw loading map layer 1
    case 2:
        break;

    // Draw loading player state
    case 3:
        break;

    // Draw loading spawners
    case 4:
        break;
    }
}

static void draw_game(scene_data_t *data)
{
    int camera_x, camera_y;

    Rectangle tile = {
        .width = TILE_DRAW_SIZE,
        .height = TILE_DRAW_SIZE,
    };

    Rectangle sprite = {
        .width = 16,
        .height = 16,
    };

    ClearBackground(BLACK);

    for (int layer = 0; layer < MAP_MAX_LAYERS; layer++) {
        for (int y = 0; y < data->camera.height; y++) {
            camera_y = y + data->camera.y;

            for (int x = 0; x < data->camera.width; x++) {
                camera_x = x + data->camera.x;

                if (tile_empty(data->map.tiles[layer][camera_y][camera_x]))
                    continue;

                tile.x = (camera_x - data->camera.x) * tile.width;
                tile.y = (camera_y - data->camera.y) * tile.height;

                sprite.x = tile_x(data->map.tiles[layer][camera_y][camera_x])
                    * fabs(sprite.width);

                sprite.y = tile_y(data->map.tiles[layer][camera_y][camera_x])
                    * fabs(sprite.height);

                if (tile_flipped(data->map.tiles[layer][camera_y][camera_x], 0))
                    sprite.width = -fabs(sprite.width);
                else
                    sprite.width = fabs(sprite.width);

                if (tile_flipped(data->map.tiles[layer][camera_y][camera_x], 1))
                    sprite.height = -fabs(sprite.height);
                else
                    sprite.height = fabs(sprite.height);

                // Fix little gaps that appear when move the camera, some thin
                // black lines around all tiles.
                tile.width++;
                tile.height++;

                DrawTexturePro(data->spritesheet, sprite, tile,
                    (Vector2) { 0, 0, }, 0, WHITE);

                tile.width--;
                tile.height--;
            }
        }
    }

    entity_draw(&data->entities, data->camera);

    if (data->paused) {
        DrawTexturePro(data->unpause,
            (Rectangle) { 0, 0, data->unpause.width, data->unpause.height },
            data->pause_button, (Vector2) { 0, 0 }, 0, WHITE);
    } else {
        DrawTexturePro(data->save,
            (Rectangle) { 0, 0, data->save.width, data->save.height },
            data->save_button, (Vector2) { 0, 0 }, 0, WHITE);

        DrawTexturePro(data->pause,
            (Rectangle) { 0, 0, data->pause.width, data->pause.height },
            data->pause_button, (Vector2) { 0, 0 }, 0, WHITE);
    }

    DrawTexturePro(data->back,
        (Rectangle) { 0, 0, -data->back.width, data->back.height },
        data->back_button, (Vector2) { 0, 0 }, 0, WHITE);

#ifdef PLATFORM_ANDROID
    if (!data->paused && data->saving == 0) {
        virtual_joystick_draw(&data->virtual_joystick);

        DrawTexturePro(data->attack,
            (Rectangle) { 0, 0, data->attack.width, data->attack.height },
            data->attack_button, (Vector2) { 0, 0 }, 0, WHITE);
    }
#endif // PLATFORM_ANDROID
}

