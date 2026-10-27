#ifndef SPAWN_ENEMY_SYSTEM_H
#define SPAWN_ENEMY_SYSTEM_H

#include "../context.h"
#include "../components/enemy_component.h"
#include "../components/ai_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/transform_component.h"
#include "../components/action_state_component.h"
#include "../components/animator_component.h"
#include "../components/animator_def.h"
#include "spawn_def.h"

// 在 OnStart 阶段运行，创建初始敌人实体
static void spawn_enemy_system(entt::registry& registry, Context* ctx, float /*dt*/) {
    (void)ctx;

    for (int i = 0; i < ENEMY_SPAWN_COUNT; i++) {
        auto& s = ENEMY_SPAWNS[i];
        entt::entity e = registry.create();
        registry.emplace<IdComponent>(e, get_entity_id(ctx));
        registry.emplace<EnemyComponent>(e);
        registry.emplace<SpawnIdxComponent>(e, i);
        registry.emplace<LogicPositionComponent>(e, fp_from_float(s.x), fp_from_float(s.y));
        registry.emplace<LogicRectComponent>(e, fp_from_float(1), fp_from_float(1));
        registry.emplace<TransformComponent>(e, s.x, s.y, 0.0f, 1.0f, 1.0f);

        registry.emplace<ActionStateComponent>(e);
        registry.emplace<AIComponent>(e,
            fp_from_float(s.x), fp_from_float(s.y),  // patrol_center
            fp_from_float(s.patrol_radius),
            fp_from_float(s.detect_radius),
            fp_from_float(s.attack_radius)
        );

        // 挂载动画组件（如果 goblin 动画定义已注册）
        if (registry.ctx().contains<janim_goblin_def_t>()) {
            registry.emplace<AnimatorComponent>(e);
            registry.emplace<AnimParamsComponent>(e);
        }
    }
}

#endif