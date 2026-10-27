#ifndef BOMB_SYSTEM_H
#define BOMB_SYSTEM_H

#include "../components/bomb_component.h"
#include "../components/health_component.h"
#include "../components/player_component.h"
#include "../components/enemy_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/action_state_component.h"
#include "./map_def.h"
#include "../context.h"
#include <vector>

struct Context;

// 炸弹推进系统：飞行（重力 + joy 扫掠碰撞）→ 引信倒计时 → 爆炸（范围伤害）→ 销毁
// 两端确定性运行：逻辑只依赖相同输入与相同物理步进，无需网络同步（实体由 get_entity_id 本地生成）。
static void bomb_system(entt::registry& registry, Context* ctx, float /*dt*/)
{
    auto bomb_view = registry.view<BombComponent, LogicPositionComponent, LogicRectComponent>();
    auto enemy_view = registry.view<EnemyComponent, LogicPositionComponent, LogicRectComponent, HealthComponent>();

    const fp_t dt = fp_from_float(game_runtime(ctx).FIXED_TIMESTEP);
    const fp_t g  = fp_from_float(BOMB_GRAVITY);
    std::vector<entt::entity> to_destroy;

    for (auto e : bomb_view) {
        auto& bomb = bomb_view.get<BombComponent>(e);
        auto& pos  = bomb_view.get<LogicPositionComponent>(e);
        auto& rect = bomb_view.get<LogicRectComponent>(e);

        if (bomb.state == BOMB_ST_OFF) {
            // ---- 飞行阶段：重力 + 扫掠碰撞（joy 引擎 joy_tilemap_sweep_x/y，与玩家同一套）----
            // 扫掠逐行检查跨越的格、精确贴到平台顶，高速下落也不会穿透，稳定落在地板平面。
            bomb.vy = fp_add(bomb.vy, fp_mul(g, dt));
            if (bomb.vy > fp_from_float(BOMB_MAX_FALL)) bomb.vy = fp_from_float(BOMB_MAX_FALL);

            bool landed = false;

            // X 轴：扫掠推进，撞墙贴齐并清水平速度
            fp_t dx = fp_mul(bomb.vx, dt);
            if (joy_tilemap_sweep_x(&pos.x, &pos.y, dx, rect.width, rect.height, map_solid_at)) bomb.vx = fp_zero();

            // Y 轴：扫掠推进，落地/撞顶都贴齐并清 vy；下落撞到实体才视为落板
            bool was_falling = bomb.vy > fp_zero();
            if (joy_tilemap_sweep_y(&pos.x, &pos.y, &bomb.vy, dt, rect.width, rect.height, map_solid_at)) {
                if (was_falling) landed = true;
            }

            if (landed) {
                bomb.state = BOMB_ST_ON;
                bomb.fuse_ticks = BOMB_FUSE_TICKS;
                bomb.vx = 0; bomb.vy = 0;
            }
        }
        else if (bomb.state == BOMB_ST_ON) {
            // ---- 引信阶段：静止等待引爆 ----
            bomb.fuse_ticks--;
            if (bomb.fuse_ticks <= 0) {
                bomb.state = BOMB_ST_EXPLODING;
                bomb.explode_ticks = BOMB_EXPLODE_TICKS;

                // 范围伤害：对半径内敌人一次性结算（跳过已死亡）。无友伤（与子弹一致）。
                const fp_t bcx = fp_add(pos.x, fp_from_float(BOMB_HALF));
                const fp_t bcy = fp_add(pos.y, fp_from_float(BOMB_HALF));
                const fp_t R = fp_from_float(BOMB_RADIUS);
                const fp_t R2 = fp_mul(R, R);
                const int dmg = (int)BOMB_DAMAGE;
                for (auto en : enemy_view) {
                    auto& h = enemy_view.get<HealthComponent>(en);
                    if (h.dead) continue;
                    auto& ep = enemy_view.get<LogicPositionComponent>(en);
                    auto& er = enemy_view.get<LogicRectComponent>(en);
                    fp_t ecx = fp_add(ep.x, fp_mul(er.width,  fp_from_float(0.5f)));
                    fp_t ecy = fp_add(ep.y, fp_mul(er.height, fp_from_float(0.5f)));
                    fp_t dx = fp_sub(ecx, bcx);
                    fp_t dy = fp_sub(ecy, bcy);
                    if (fp_add(fp_mul(dx, dx), fp_mul(dy, dy)) <= R2) {
                        h.hp -= dmg; if (h.hp < 0) h.hp = 0;
                    }
                }
            }
        }
        else { // BOMB_ST_EXPLODING
            bomb.explode_ticks--;
            if (bomb.explode_ticks <= 0) to_destroy.push_back(e);
        }
    }

    for (auto e : to_destroy) registry.destroy(e);
}

#endif
