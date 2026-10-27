#ifndef PLAYER_DEATH_SYSTEM_H
#define PLAYER_DEATH_SYSTEM_H
#include "../components/health_component.h"
#include "../components/spawn_point_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/player_component.h"
#include "../components/enemy_component.h"
#include "../components/action_state_component.h"
#include "../components/player_state_input.h"
#include "../components/transform_component.h"
#include <vector>
#include "../context.h"                           // GameModeKind（判定是否权威端）
#include "../components/global_state_component.h" // GlobalState.next_frame_id（权威端帧号）
#include "../components/net_component.h"          // NetComponent.frame_id（客户端的服务器权威帧号）

struct Context;

// 当前"权威逻辑帧号"：锁步下帧 N 两端含义完全一致，可安全作为倒计时基准。
//   - 权威端（单机/Host/服务端）：GlobalState.next_frame_id - 1
//     （tick() 里是 next_frame_id++ 分配本帧号后推进，故当前帧 = 值 - 1）
//   - 客户端：NetComponent.frame_id，即服务端下发的权威帧号
// 与 game_scene.cpp Render 里取帧号显示的口径一致。
static inline int current_logic_frame(Context* ctx)
{
    if (!ctx) return 0;
    if (ctx->mode != MODE_CLIENT) {
        auto& c = ctx->ecs.ctx();
        if (!c.contains<GlobalState>()) return 0;
        return c.get<GlobalState>().next_frame_id - 1;
    }
    auto& c = ctx->ecs.ctx();
    if (!c.contains<NetComponent>()) return 0;
    return c.get<NetComponent>().frame_id;
}

// 玩家死亡 / 重生（帧同步确定性：两端同一 tick 判定与传送，无需网络同步）
// 注册在 bullet_system 之后：子弹扣血 → 死亡判定 → 重生计时
static void player_death_system(entt::registry& registry, Context* ctx, float /*dt*/)
{
    auto view = registry.view<PlayerComponent, HealthComponent, LogicPositionComponent>();
    for (auto e : view) {
        auto& h   = view.get<HealthComponent>(e);
        auto& pos = view.get<LogicPositionComponent>(e);

        // 死亡判定：HP 归零（bullet_system 已把 HP 钳制在 0）
        if (!h.dead && h.hp <= 0) {
            h.dead = true;
            h.respawn_timer = PLAYER_RESPAWN_TICKS;
        }

        // 无敌保护倒计时（玩家可能未被判定死亡，但重生后有无敌窗口）
        if (h.spawn_invuln > 0) h.spawn_invuln -= 1;

        // 重生计时与传送回出生点
        if (h.dead) {
            h.respawn_timer -= 1;   // 每逻辑 tick 减 1
            if (h.respawn_timer <= 0) {
                h.dead = false;
                h.hp = h.max_hp;
                h.spawn_invuln = SPAWN_INVULN_TICKS;  // 重生无敌，防出生点被围殴"重生即死"
                const auto& sp = registry.get<SpawnPointComponent>(e);
                // SpawnPoint 存的是出生格中心；LogicPosition(左上角) = 中心 - 半宽高
                auto& rect = registry.get<LogicRectComponent>(e);
                pos.x = fp_sub(sp.x, fp_mul(rect.width, fp_half()));
                pos.y = fp_sub(sp.y, fp_mul(rect.height, fp_half()));
                // 清重生前残留输入/动作状态：
                // 防"死亡时按着方向键/出招中，重生瞬间自动冲出或摆出攻击姿势"
                if (registry.all_of<PlayerStateInput>(e))
                    registry.get<PlayerStateInput>(e).move_bits = 0;
                if (registry.all_of<ActionStateComponent>(e)) {
                    auto& act = registry.get<ActionStateComponent>(e);
                    act.state           = ActionStateComponent::IDLE;
                    act.attack_timer    = 0;
                    act.melee_cooldown  = 0;   // 清近战冷却，防重生瞬间立刻挥击
                }
                // 同步渲染坐标（碰撞盒中心 = 出生格中心），避免插值缓慢漂移
                if (registry.all_of<TransformComponent>(e)) {
                    auto& t    = registry.get<TransformComponent>(e);
                    const auto& sp = registry.get<SpawnPointComponent>(e);
                    t.position.x = fp_to_float(sp.x);
                    t.position.y = fp_to_float(sp.y);
                }
            }
        }
    }

    // 敌人死亡：标记死亡（不重生），达到"肃清全关"通关条件。
    // 尸体只保留 ENEMY_CORPSE_TICKS 个逻辑 tick（够播完死亡动画），随后销毁实体使其消失。
    //
    // 倒计时基准用"权威逻辑帧号"而非渲染时间/本地 tick 计数：
    //   两端都在同一逻辑帧把 hp 打到 0 → 记下同一个 death_frame → 同一帧销毁，确定性一致，
    //   且无需在协议里同步倒计时（帧号本身就是已同步的量）。
    // 唯一不一致是"中途加入的客户端"：它从快照接管尸体时拿不到原始死亡帧，
    //   以接管帧为起点，会比服务端晚几帧销毁；该差异是良性的——
    //   死亡敌人已排除出帧同步校验（CollectInput / verifyFrameSyncImpl），且尸体是惰性的。
    const int cur_frame = current_logic_frame(ctx);
    auto eview = registry.view<EnemyComponent, HealthComponent>();
    std::vector<entt::entity> corpses_to_destroy;
    for (auto e : eview) {
        auto& h = eview.get<HealthComponent>(e);
        if (!h.dead && h.hp <= 0) {
            h.dead = true;
            h.death_frame = cur_frame;      // 记录死亡帧（两端同一帧置位）
        }
        if (h.dead) {
            if (h.death_frame < 0) h.death_frame = cur_frame;   // 快照接管的尸体：以接管帧为起点
            if (cur_frame - h.death_frame >= ENEMY_CORPSE_TICKS)
                corpses_to_destroy.push_back(e);
        }
    }
    // 迭代外销毁（entt 不允许在 view 遍历中 destroy）
    for (auto e : corpses_to_destroy)
        registry.destroy(e);
}

#endif
