#ifndef BOMB_SHOOT_SYSTEM_H
#define BOMB_SHOOT_SYSTEM_H

#include "../components/action_state_component.h"
#include "../components/bomb_component.h"
#include "../components/health_component.h"
#include "../components/id_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/player_component.h"
#include "../components/player_event_input.h"
#include "../components/transform_component.h"
#include "../context.h"
#include <algorithm>

struct Context;

/* 玩家投掷炸弹：攻击键（INPUT_ATTACK）完整点击 → 沿当前朝向抛出一个炸弹实体并进入冷却。
 * 帧同步确定性：两端同一 tick 消费同一投掷事件、用 get_entity_id 生成相同 ID 的炸弹实体，
 * 与 bullet_system 完全一致（本地推进，不进快照）。 */
static void spawn_player_bomb(entt::registry& registry, Context* ctx,
                              fp_t px, fp_t py, int conv, int dir)
{
    // 玩家碰撞盒 0.8x0.8，中心 = 左上角 + 0.4
    fp_t cx = fp_add(px, fp_from_float(0.4f));
    fp_t cy = fp_add(py, fp_from_float(0.4f));

    // 朝向分量（0=下 1=左 2=右 3=上）
    fp_t dirx = 0, diry = 0;
    switch (dir) {
        case 0: diry =  fp_from_float(1.0f);  break; // 下
        case 1: dirx = -fp_from_float(1.0f);  break; // 左
        case 2: dirx =  fp_from_float(1.0f);  break; // 右
        default: diry = -fp_from_float(1.0f); break; // 上
    }

    // 沿朝向水平/垂直抛出，并叠加一个上抛分量（重力下落后形成抛物线，+y 向下故上抛为负）
    fp_t vx = fp_mul(dirx, fp_from_float(BOMB_THROW_SPEED));
    fp_t vy = fp_sub(fp_mul(diry, fp_from_float(BOMB_THROW_SPEED)), fp_from_float(BOMB_THROW_UP));

    // 出生点：从玩家中心沿朝向推出 0.3m，避免炸弹出生即与玩家/墙体重叠
    cx = fp_add(cx, fp_mul(dirx, fp_from_float(0.3f)));
    cy = fp_add(cy, fp_mul(diry, fp_from_float(0.3f)));

    vec2_t position = { fp_to_float(cx), fp_to_float(cy) };
    vec2_t scale = { BOMB_SIZE, BOMB_SIZE };

    entt::entity b = registry.create();
    registry.emplace<IdComponent>(b, get_entity_id(ctx));
    registry.emplace<BombComponent>(b,
        BOMB_ST_OFF, conv, vx, vy, BOMB_FUSE_TICKS, BOMB_EXPLODE_TICKS);
    registry.emplace<LogicPositionComponent>(b, fp_sub(cx,  fp_from_float(BOMB_HALF)), fp_sub(cy, fp_from_float(BOMB_HALF)));
    registry.emplace<LogicRectComponent>(b, fp_from_float(BOMB_SIZE), fp_from_float(BOMB_SIZE));
    registry.emplace<TransformComponent>(b, position, 0.0f, scale);
}

static void bomb_shoot_system(entt::registry& registry, Context* ctx, float /*dt*/)
{
    auto view = registry.view<PlayerComponent, PlayerEventInput, LogicPositionComponent, ActionStateComponent>();
    for (auto e : view) {
        auto& act = view.get<ActionStateComponent>(e);

        // 冷却递减（每逻辑 tick 减 1）
        if (act.bomb_cooldown > 0) act.bomb_cooldown -= 1;

        // 只消费投掷炸弹事件
        auto erase_bomb = [](std::vector<PlayerEventInput::Action>& v) {
            v.erase(std::remove_if(v.begin(), v.end(),
                [](const PlayerEventInput::Action& a){ return a.type == PlayerEventInput::ATTACK_BOMB; }),
                v.end());
        };
        auto& events = view.get<PlayerEventInput>(e);
        bool want_bomb = false;
        for (auto& a : events.pending_actions)
            if (a.type == PlayerEventInput::ATTACK_BOMB) { want_bomb = true; break; }
        if (!want_bomb) continue;

        // 死亡玩家不能投掷
        if (registry.all_of<HealthComponent>(e) && registry.get<HealthComponent>(e).dead) {
            erase_bomb(events.pending_actions); continue;
        }
        if (act.bomb_cooldown > 0) { erase_bomb(events.pending_actions); continue; }

        act.bomb_cooldown = ActionStateComponent::BOMB_COOLDOWN;
        auto& pos = view.get<LogicPositionComponent>(e);
        auto& pc  = view.get<PlayerComponent>(e);
        spawn_player_bomb(registry, ctx, pos.x, pos.y, pc.conv, act.dir);
        erase_bomb(events.pending_actions);
    }
}

#endif
