#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H
#include "../components/enemy_component.h"
#include "../components/logic_position_component.h"
#include "../components/logic_rect_component.h"
#include "../components/player_component.h"

struct Context;

// 玩家与 tilemap 碰撞处理 system
// 在 OnUpdate 阶段运行，在 movement_system 之后
// 检测所有玩家与地图碰撞层的碰撞，修正玩家位置
static void collision_system(entt::registry& registry, Context* ctx, float dt) {
    (void)ctx;
    (void)dt;

    // 玩家视图：所有带 PlayerComponent 的实体（同时需有 LogicRectComponent + LogicPositionComponent）
    auto player_view = registry.view<PlayerComponent, LogicRectComponent, LogicPositionComponent>();

    // 地图碰撞体视图：所有带 LogicRectComponent + LogicPositionComponent 的实体
    auto all_view = registry.view<LogicRectComponent, LogicPositionComponent>();

    for (auto player_entity : player_view) {
        auto& player_rect = player_view.get<LogicRectComponent>(player_entity);
        auto& player_pos = player_view.get<LogicPositionComponent>(player_entity);

        // 记录 movement 修改后的坐标（用于监测哪些玩家被修正）
        fp_t orig_x = player_pos.x;
        fp_t orig_y = player_pos.y;
        bool was_corrected = false;

        // 构建玩家碰撞盒子（x/y 跟随 LogicPositionComponent）
        rectanglef_t player_box = { player_pos.x, player_pos.y, player_rect.width, player_rect.height };

        for (auto tile_entity : all_view) {
            // 跳过玩家自身、敌人等非 tilemap 实体
            if (tile_entity == player_entity) continue;
            if (registry.all_of<PlayerComponent>(tile_entity)) continue;
            if (registry.all_of<EnemyComponent>(tile_entity)) continue;

            auto& tile_rect = all_view.get<LogicRectComponent>(tile_entity);
            auto& tile_pos = all_view.get<LogicPositionComponent>(tile_entity);

            if (tile_rect.width == fp_zero() || tile_rect.height == fp_zero()) continue;

            rectanglef_t tile_box = { tile_pos.x, tile_pos.y, tile_rect.width, tile_rect.height };

            contact2df_t contact;
            if (collision2df_get_rectangles(player_box, fp_zero(), tile_box, fp_zero(), &contact)) {
                // 碰撞发生，沿法线方向将玩家推出
                fp_t depth = contact.depth;
                if (depth < fp_zero()) {
                    depth = fp_sub(fp_zero(), depth);
                }
                fp_t nx = fp_sub(fp_zero(), contact.normal.x);
                fp_t ny = fp_sub(fp_zero(), contact.normal.y);
                player_pos.x = fp_add(player_pos.x, fp_mul(nx, depth));
                player_pos.y = fp_add(player_pos.y, fp_mul(ny, depth));

                // 更新 player_box 以继续与其他 tile 碰撞检测
                player_box.x = player_pos.x;
                player_box.y = player_pos.y;

                was_corrected = true;
            }
        }

        // 监测：该玩家的 movement 坐标被碰撞修正
        if (was_corrected) {
            /*
            log_info("COLLISION: player entity %d collision corrected (moved from (%.2f, %.2f) to (%.2f, %.2f))",
                (int)player_entity, fp_to_float(orig_x), fp_to_float(orig_y),
                fp_to_float(player_pos.x), fp_to_float(player_pos.y));
                */
        }
    }
}

#endif
