#ifndef CAMERA_FOLLOW_SYSTEM_H
#define CAMERA_FOLLOW_SYSTEM_H
#include "../components/global_state_component.h"
#include "../components/player_component.h"
#include "../components/transform_component.h"
#include "../protocol.h"

struct Context;

// 将摄像机跟随指定玩家，使玩家居中于屏幕
// 逻辑屏幕 480x320，PPM=50，偏移量 = (480/2/50, 320/2/50) = (4.8, 3.2)
static void camera_follow_system(entt::registry& registry, Context* /*ctx*/, float /*dt*/) {
    auto& gs = registry.ctx().get<GlobalState>();
    if (gs.camera_follow_conv < 0) return;

    auto view = registry.view<PlayerComponent, TransformComponent>();
    for (auto e : view) {
        if (registry.get<PlayerComponent>(e).conv == gs.camera_follow_conv) {
            auto& t = registry.get<TransformComponent>(e);
            gs.camera_position.x = t.position.x - 4.8f;
            gs.camera_position.y = t.position.y - 3.2f;
            return;
        }
    }
}

#endif