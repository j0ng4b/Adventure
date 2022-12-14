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

#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"
#include "utils/list.h"
#include "world/map/map.h"
#include "world/map/tile.h"

#define ENTITY_HEART_BAR_WIDTH TILE_DRAW_SIZE
#define ENTITY_HEART_BAR_HEIGHT (TILE_DRAW_SIZE / 4.0)

#define ENTITY_TILE_SIZE      (TILE_DRAW_SIZE / 2.0)
#define ENTITY_SPRITE_SIZE    16.0

#define ENTITY_FRAME_DELAY (120.0 / 1000.0)

typedef struct entity entity_t;
typedef list(entity_t *) entity_list_t;

typedef enum {
    ENTITY_STATE_SPAWN,
    ENTITY_STATE_MOVING,
    ENTITY_STATE_IDLE,
    ENTITY_STATE_DAMAGING,
} entity_state_t;

struct entity {
    Vector2 position;
    float   velocity;
    float   direction;

    float damage_direction;

    float hearts;
    float max_hearts;

    float attack;
    float defense;

    entity_state_t state;

    struct {
        int   current;
        float delay;
        int   max;
    } frame;

    unsigned spawner_id;

    void (* update)(unsigned entity, entity_list_t *entities, map_t *map);
    void (* draw)(entity_t *entity, Rectangle camera);
    void (* destroy)(entity_t *entity);
};

void entity_update(entity_list_t *entities, map_t *map, Rectangle camera);
void entity_draw(entity_list_t *entities, Rectangle camera);
void entity_destroy(entity_list_t *entities);

#endif // !ENTITY_H

