#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H
#include <joy/joy.h>
#include "../context.h"

// 主场景模式测试按钮事件（main.cpp 处理，切换运行模式后进入游戏场景）
#define MODE_EVENT (0x3)

enum mode_event_code {
        MODE_EVENT_SINGLE = 1,
        MODE_EVENT_HOST = 2,
        MODE_EVENT_CLIENT = 3,
        MODE_EVENT_SERVER = 4,
};

typedef struct main_scene main_scene_t, *main_scene_p;

main_scene_p main_scene_create(Context* ctx);
scene_p main_scene_get_scene(main_scene_p s);

#endif