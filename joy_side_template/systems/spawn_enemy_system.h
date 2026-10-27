#ifndef SPAWN_ENEMY_SYSTEM_H
#define SPAWN_ENEMY_SYSTEM_H

#include "../context.h"
#include "../components/global_state_component.h"   // kEntityRenderScale：碰撞盒与渲染尺寸同步
#include "../components/enemy_component.h"
#include "../components/ai_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/transform_component.h"
#include "../components/action_state_component.h"
#include "../components/health_component.h"
#include "../components/logic_velocity_component.h"
#include "../components/spawn_point_component.h"
#include "../systems/spawn_def.h"

// 在 OnStart 阶段运行，按当前关卡 'E' 格确定性创建初始敌人实体
static void spawn_enemy_system(entt::registry& registry, Context* ctx, float /*dt*/) {
    int count = 0;
    const SpawnDef* ENEMY_SPAWNS = level_enemy_defs(ctx->selected_level, count);
    log_info("[spawn_enemy] level=%d enemy_count=%d", ctx->selected_level, count);

    for (int i = 0; i < count; i++) {
        auto& s = ENEMY_SPAWNS[i];
        entt::entity e = registry.create();
        registry.emplace<IdComponent>(e, get_entity_id(ctx));
        registry.emplace<EnemyComponent>(e);
        registry.emplace<SpawnIdxComponent>(e, i);
        // 种类：渲染层按它取 enemies.json 里的精灵目录与动画帧数
        registry.emplace<EnemyTypeComponent>(e, s.type);
        // 出生格中心 s.x/s.y；碰撞盒 0.8×0.8 以出生点居中，
        // 碰撞盒高度按各敌人可视高度(render_height)对齐，避免下半身穿入地面（穿模）。
        // 碰撞盒中心保持 = 出生点(s.x, s.y) = Transform 渲染中心（精灵以中心绘制），
        // 故 height = render_height 后盒底恰落在精灵脚底；宽度维持 0.8 不变（按需求只调高度）。
        // 逻辑层统一 fp_ 运算：render_height 是运行时浮点，不能与浮点相乘后再转定点，
        // 改为各自转定点再 fp_mul，保证帧同步两端完全一致。
        fp_t ehw = fp_mul(fp_from_float(g_enemyCatalog.get(s.type).render_height),
                          fp_from_float(kEntityRenderScale));
        if (ehw <= fp_zero()) ehw = fp_mul(fp_from_float(0.8f), fp_from_float(kEntityRenderScale));
        // 逻辑层统一用 fp_ 运算：禁止两个浮点直接相减（s.x - 0.4f），
        // 改为先转定点再 fp_sub，保证帧同步两端完全一致。
        const fp_t kEnemyHalfW = fp_from_float(0.4f);        // 半宽 = 宽度 0.8 / 2
        registry.emplace<LogicPositionComponent>(e,
            fp_sub(fp_from_float(s.x), kEnemyHalfW),         // 左上角 x = 中心 - 半宽
            fp_sub(fp_from_float(s.y), fp_mul(ehw, fp_from_float(0.5f))));  // 左上角 y = 中心 - 半高
        registry.emplace<LogicRectComponent>(e, fp_from_float(0.8f), ehw);
        registry.emplace<TransformComponent>(e, s.x, s.y, 0.0f, 1.0f, 1.0f);

        registry.emplace<ActionStateComponent>(e);
        // 横版重力：敌人挂垂直速度组件（落到地面/平台）
        registry.emplace<LogicVelocityComponent>(e, fp_from_float(0), fp_from_float(0));
        // 血量组件：按敌人种类取最大血量（enemies.json stats.hp）
        registry.emplace<HealthComponent>(e, s.max_hp, s.max_hp);
        registry.emplace<SpawnPointComponent>(e, fp_from_float(s.x), fp_from_float(s.y));
        registry.emplace<AIComponent>(e,
            fp_from_float(s.x), fp_from_float(s.y),  // patrol_center
            fp_from_float(s.patrol_radius),
            fp_from_float(s.detect_radius),
            fp_from_float(s.attack_radius),
            s.ranged,                                   // 是否远程敌人
            fp_from_float(s.bullet_speed),              // 子弹速度
            fp_from_float(s.bullet_damage),             // 子弹伤害
            (int)(s.bullet_interval * 20.0f + 0.5f),    // 射击间隔（tick）
            0,                                          // 射击冷却
            fp_from_float(s.move_speed),                // 移动速度（按种类）
            s.melee_damage                              // 近战伤害（按种类）
        );
    }
}

#endif