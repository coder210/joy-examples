#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

#include "../components/ai_component.h"
#include "../components/enemy_component.h"
#include "../components/player_component.h"
#include "../components/logic_position_component.h"
#include "../components/action_state_component.h"
#include "../components/player_event_input.h"
#include "../components/animator_component.h"

struct Context;

static void ai_system(entt::registry& registry, Context* ctx, float /*dt*/) {
    fp_t step = fp_mul(fp_from_float(2.5f), fp_from_float(ctx->FIXED_TIMESTEP));

    auto view = registry.view<EnemyComponent, AIComponent, LogicPositionComponent, ActionStateComponent>();
    auto player_view = registry.view<PlayerComponent, LogicPositionComponent>();

    if (!ctx->is_server) {
        int i = 10;
    }

    for (auto e : view) {
        auto& ai  = view.get<AIComponent>(e);
        auto& pos = view.get<LogicPositionComponent>(e);
        auto& act = view.get<ActionStateComponent>(e);

        // 寻找最近玩家
        bool found_player = false;
        fp_t nearest_dist_sq = fp_mul(ai.detect_radius, ai.detect_radius);
        fp_t nearest_px = fp_zero(), nearest_py = fp_zero();

        for (auto p : player_view) {
            auto& ppos = player_view.get<LogicPositionComponent>(p);
            fp_t dx = fp_sub(ppos.x, pos.x);
            fp_t dy = fp_sub(ppos.y, pos.y);
            fp_t dist_sq = fp_add(fp_mul(dx, dx), fp_mul(dy, dy));
            if (dist_sq < nearest_dist_sq) {
                nearest_dist_sq = dist_sq;
                found_player = true;
                nearest_px = ppos.x;
                nearest_py = ppos.y;
            }
        }

        // 攻击半径判断
        fp_t attack_dist_sq = fp_mul(ai.attack_radius, ai.attack_radius);
        bool in_attack_range = found_player && (nearest_dist_sq < attack_dist_sq);

        if (in_attack_range) {
            // 状态 2：攻击
            ai.state = 2;
            act.state = ActionStateComponent::ATTACK;

            if (!registry.all_of<PlayerEventInput>(e)) {
                registry.emplace<PlayerEventInput>(e);
            }
            auto& events = registry.get<PlayerEventInput>(e);
            events.pending_actions.push_back({ 0, 0 });

            if (registry.all_of<anim_params_t>(e)) {
                auto& params = registry.get<anim_params_t>(e);
                fp_t dx = fp_sub(nearest_px, pos.x);
                fp_t dy = fp_sub(nearest_py, pos.y);
                params.dir = (fp_abs(dx) > fp_abs(dy))
                    ? (dx > fp_zero() ? 2 : 1)
                    : (dy > fp_zero() ? 0 : 3);
                params.attack = true;
                params.speed = 0.0f;
            }
        }
        else if (found_player) {
            // 状态 1：追击
            ai.state = 1;
            act.state = ActionStateComponent::WALK;

            fp_t dx = fp_sub(nearest_px, pos.x);
            fp_t dy = fp_sub(nearest_py, pos.y);

            if (fp_abs(dx) > fp_abs(dy)) {
                pos.x = dx > fp_zero() ? fp_add(pos.x, step) : fp_sub(pos.x, step);
            } else {
                pos.y = dy > fp_zero() ? fp_add(pos.y, step) : fp_sub(pos.y, step);
            }

            if (registry.all_of<anim_params_t>(e)) {
                auto& params = registry.get<anim_params_t>(e);
                params.dir = (fp_abs(dx) > fp_abs(dy))
                    ? (dx > fp_zero() ? 2 : 1)
                    : (dy > fp_zero() ? 0 : 3);
                params.speed = 1.0f;
                params.attack = false;
            }

            ai.timer = 0;
        }
        else {
            // 状态 0：巡逻
            ai.state = 0;
            act.state = ActionStateComponent::WALK;

            ai.timer--;
            if (ai.timer <= 0) {
                //int seed = (int)e;
                int seed = 0;
                fp_t rx = fp_from_float((fp_to_float(ai.patrol_radius) * ((seed % 7) - 3) / 3.0f));
                fp_t ry = fp_from_float((fp_to_float(ai.patrol_radius) * (((seed * 13) % 9) - 4) / 4.0f));
                ai.target_x = fp_add(ai.patrol_center_x, rx);
                ai.target_y = fp_add(ai.patrol_center_y, ry);
                ai.timer = 120;
            }

            fp_t dx = fp_sub(ai.target_x, pos.x);
            fp_t dy = fp_sub(ai.target_y, pos.y);
            if (fp_abs(dx) > fp_from_float(0.2f) || fp_abs(dy) > fp_from_float(0.2f)) {
                if (fp_abs(dx) > fp_abs(dy)) {
                    pos.x = dx > fp_zero() ? fp_add(pos.x, step) : fp_sub(pos.x, step);
                } else {
                    pos.y = dy > fp_zero() ? fp_add(pos.y, step) : fp_sub(pos.y, step);
                }
            }

            if (registry.all_of<anim_params_t>(e)) {
                auto& params = registry.get<anim_params_t>(e);
                params.dir = (fp_abs(dx) > fp_abs(dy))
                    ? (dx > fp_zero() ? 2 : 1)
                    : (dy > fp_zero() ? 0 : 3);
                params.speed = 1.0f;
                params.attack = false;
            }
        }
    }
}

#endif