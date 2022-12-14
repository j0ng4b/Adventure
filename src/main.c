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

#include <stdlib.h>
#include "game.h"
#include "scene.h"

int main (int argc, char *argv[])
{
    (void) argc; (void) argv;

    if (!game_init(1280, 720))
        return EXIT_FAILURE;

    { // Scenes
        SCENE_IMPORT(logo);
        game_register_scene(SCENE(logo));

        SCENE_IMPORT(tutorial);
        game_register_scene(SCENE(tutorial));

        SCENE_IMPORT(menu);
        game_register_scene(SCENE(menu));

        SCENE_IMPORT(genmap);
        game_register_scene(SCENE(genmap));

        SCENE_IMPORT(gameplay);
        game_register_scene(SCENE(gameplay));

        SCENE_IMPORT(gameover);
        game_register_scene(SCENE(gameover));
    }

    { // Resources
        game_load_texture("tela_logo.png", "tela_logo-img");

        game_load_texture("joystick.png", "joystick-img");
        game_load_texture("cloud.png", "cloud-img");
        game_load_texture("gram.png", "gram-img");
        game_load_texture("TreeTwo.png", "TreeTwo-img");
        game_load_texture("Tree.png", "Tree-img");
        game_load_texture("plantone.png", "plantone-img");
        game_load_texture("planttwo.png", "planttwo-img");
        game_load_texture("cogu.png", "cogu-img");
        game_load_texture("mushroom.png", "mushroom-img");

        game_load_texture("key.png","key-img");
        game_load_texture("wasd.png","wasd-img");
        game_load_texture("map_with_player.png", "map_with_player-img");
        game_load_texture("mapt.png","mapt-img");

        game_load_texture("tiles.png", "tiles");

        game_load_texture("joystick_base.png", "joy-base");
        game_load_texture("joystick_top.png", "joy-top");

        game_load_texture("back.png", "back-img");
        game_load_texture("pause.png", "pause-img");
        game_load_texture("unpause.png", "unpause-img");
        game_load_texture("save.png", "save-img");
        game_load_texture("attack.png", "attack-img");

        game_load_texture("slime_spawn.png", "slime-spawn");
        game_load_texture("slime_move.png", "slime-moving");
        game_load_texture("slime_damaging.png", "slime-damaging");
        game_load_texture("slime_idle.png", "slime-idle");

        game_load_texture("player_move.png", "player-moving");
        game_load_texture("player_damaging.png", "player-damaging");
        game_load_texture("player_idle.png", "player-idle");
        game_load_texture("sword.png", "player-sword");
    }

    game_set_scene("logo");
    game_run();

    game_deinit();
    return EXIT_SUCCESS;
}

