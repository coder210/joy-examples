#ifndef GAME_SCENE_H
#define GAME_SCENE_H
#include <joy/joy.h>
#include "../context.h"

struct game_scene {
        scene_p scene;
        Context* ctx;
};
typedef struct game_scene game_scene_t, *game_scene_p;

game_scene_p game_scene_create(Context* ctx);
scene_p game_scene_get_scene(game_scene_p g);

#endif