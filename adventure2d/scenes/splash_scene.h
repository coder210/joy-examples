#ifndef SPLASH_SCENE_H
#define SPLASH_SCENE_H
#include <joy/joy.h>
#include "../context.h"

typedef struct splash_scene splash_scene_t, *splash_scene_p;

splash_scene_p splash_scene_create(Context* ctx);
scene_p splash_scene_get_scene(splash_scene_p s);

#endif