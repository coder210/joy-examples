#ifndef SETTINGS_SCENE_H
#define SETTINGS_SCENE_H
#include <joy/joy.h>
#include "../context.h"

// 设置场景事件（main.cpp 处理导航）
#define SETTINGS_EVENT (0x4)

enum settings_event_code {
        SETTINGS_EVENT_BACK = 1,
};

typedef struct settings_scene settings_scene_t, *settings_scene_p;

settings_scene_p settings_scene_create(Context* ctx);
scene_p settings_scene_get_scene(settings_scene_p s);

#endif