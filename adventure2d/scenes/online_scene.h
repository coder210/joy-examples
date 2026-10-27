#ifndef ONLINE_SCENE_H
#define ONLINE_SCENE_H
#include <joy/joy.h>
#include "../context.h"

#define ONLINE_EVENT (0x5)

enum online_event_code {
        ONLINE_EVENT_HOST = 1,
        ONLINE_EVENT_CLIENT = 2,
        ONLINE_EVENT_SERVER = 3,
        ONLINE_EVENT_BACK = 4,
};

typedef struct online_scene online_scene_t, *online_scene_p;

online_scene_p online_scene_create(Context* ctx);
scene_p online_scene_get_scene(online_scene_p s);

#endif