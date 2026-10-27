#ifndef SNAPSHOT_SYSTEM_H
#define SNAPSHOT_SYSTEM_H
#include "../context.h"
#include "../components/ai_component.h"
#include "../components/bullet_component.h"
#include "../components/bomb_component.h"
#include "../components/health_component.h"
#include "../components/local_only_component.h"

static void snapshot_system(entt::registry& registry, Context* ctx, float dt) {
        (void)dt;
        game_runtime(ctx).num_entities = 0;

        auto body_view = registry.view<IdComponent, LogicRectComponent, LogicPositionComponent>();
        for (auto e : body_view) {
                if (registry.all_of<LocalOnlyComponent>(e))
                        continue;
                // 子弹为帧同步瞬时实体，不进入快照，避免客户端加载时重复创建
                if (registry.all_of<BulletComponent>(e))
                        continue;
                // 炸弹同上：帧同步瞬时实体（本地 get_entity_id 生成），不进快照，否则客户端重复创建
                if (registry.all_of<BombComponent>(e))
                        continue;
                if (game_runtime(ctx).num_entities >= 512)
                        return;

                auto& id = body_view.get<IdComponent>(e);
                auto& rect = body_view.get<LogicRectComponent>(e);
                auto& pos = body_view.get<LogicPositionComponent>(e);

                S2CEntity& ent = net_session(ctx).entities[game_runtime(ctx).num_entities++];
                memset(&ent, 0, sizeof(ent));
                ent.id = id.id;
                if (registry.all_of<PlayerComponent>(e)) {
                        ent.type = S2C_TYPE_PLAYER;
                        ent.player_conv = registry.get<PlayerComponent>(e).conv;
                        ent.player_char_id = registry.get<PlayerComponent>(e).char_id;
                } else if (registry.all_of<EnemyComponent>(e)) {
                        ent.type = S2C_TYPE_ENEMY;
                        if (registry.all_of<SpawnIdxComponent>(e))
                            ent.ai_type = registry.get<SpawnIdxComponent>(e).idx;
                        // 同步 AI 运行时状态（timer/state/target/shoot_cooldown），
                        // 客户端接入时恢复，避免持续累积的 AI 状态起点不同步
                        if (registry.all_of<AIComponent>(e)) {
                            auto& ai = registry.get<AIComponent>(e);
                            ent.ai_timer          = ai.timer;
                            ent.ai_state          = ai.state;
                            ent.ai_target_x       = ai.target_x;
                            ent.ai_target_y       = ai.target_y;
                            ent.ai_shoot_cooldown = ai.shoot_cooldown;
                            // 追击记忆 / 卡住计数：同为持续累积状态，必须随快照下发，
                            // 否则晚加入端会以不同起点演算（一端追击、一端巡逻）→ 位置分叉。
                            ent.ai_alert          = ai.alert_timer;
                            ent.ai_stuck          = ai.stuck_timer;
                        }
                } else {
                        ent.type = S2C_TYPE_NORMAL;
                }
                ent.hp = 0;
                if (registry.all_of<HealthComponent>(e))
                    ent.hp = registry.get<HealthComponent>(e).hp;
                ent.position_x = pos.x;
                ent.position_y = pos.y;
                ent.width = rect.width;
                ent.height = rect.height;
        }

        auto& gs = ctx->ecs.ctx().get<GlobalState>();
        int frame_id = gs.next_frame_id - 1;
        net_session(ctx).world_snapshots[frame_id] = pack_world(gs.next_entity_id, net_session(ctx).entities, game_runtime(ctx).num_entities);

        // 只保留最近 300 帧快照（≈15s @20Hz），防止无玩家时内存线性增长。
        // std::map 按帧号有序，begin() 即最旧帧；新玩家对齐用 lower_bound 命中
        // 最近帧即可，因此 300 帧缓冲完全够用。
        if (net_session(ctx).world_snapshots.size() > 300)
                net_session(ctx).world_snapshots.erase(net_session(ctx).world_snapshots.begin());
}

#endif
