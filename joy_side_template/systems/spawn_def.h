#ifndef SPAWN_DEF_H
#define SPAWN_DEF_H

#include "map_def.h"

// 敌人生成位置和参数（服务端和客户端共享，保证初始条件一致）
// 位置对应 map_def.h 中的可通行格，避免敌人卡在障碍物里
struct SpawnDef {
    float x, y;           // 位置
    int   type;           // 敌人种类 id（EnemyCatalog 下标）；由 Tiled 出生点对象的 type/name 派生
    float patrol_radius;  // 巡逻半径
    float detect_radius;  // 检测范围
    float attack_radius;  // 攻击范围
    bool  ranged;         // 是否远程敌人
    float bullet_speed;   // 子弹速度（m/s）
    float bullet_interval;// 射击间隔（秒）
    float bullet_damage;  // 子弹伤害
    // ---- 以下三项按种类从 enemies.json 取，供 spawn_enemy_system 装配实体 ----
    float move_speed;     // 移动速度（m/s）
    int   melee_damage;   // 近战伤害
    int   max_hp;         // 最大血量
};

// ============================================================
// 敌人出生：由当前关卡('E' 格)确定性派生 SpawnDef（与 3D 关卡对齐）。
// 服务端/客户端解析同一关卡 + 同一 i 规则 → 生成的出生列表逐位一致，
// 保证帧同步初态一致；ai_type(=SpawnIdxComponent.idx) 即该数组下标。
// ============================================================
constexpr int ENEMY_SPAWN_COUNT = 256;   // 出生表容量上限（C++11 constexpr 常量）

static inline const SpawnDef* level_enemy_defs(int level, int& count)
{
    if (g_map_info.level != level) map_load_level(level);
    if (g_map_info.enemy_n > ENEMY_SPAWN_COUNT) g_map_info.enemy_n = ENEMY_SPAWN_COUNT;
    count = g_map_info.enemy_n;

    static SpawnDef defs[ENEMY_SPAWN_COUNT];
    for (int i = 0; i < count; ++i) {
        int cell = g_map_info.enemy_cells[i];
        int cx = cell % MAP_W, cy = cell / MAP_W;

        // 种类来自 Tiled 出生点（g_map_info.enemy_types 与 enemy_cells 一一对应）；
        // 全部数值改由 enemies.json 的该种类定义装配，不再用 (i % 2) 硬编码近战/远程。
        const EnemyDef& ed = g_enemyCatalog.get(g_map_info.enemy_types[i]);

        SpawnDef d;
        d.x = (float)cx + 0.5f;
        d.y = (float)cy + 0.5f;
        d.type           = ed.id;
        d.patrol_radius  = ed.patrol_radius;
        d.detect_radius  = ed.detect_radius;
        d.attack_radius  = ed.attack_radius;
        d.ranged         = ed.ranged;
        d.bullet_speed   = ed.bullet_speed;
        d.bullet_interval= ed.bullet_interval;
        d.bullet_damage  = ed.bullet_damage;
        d.move_speed     = fp_to_float(ed.move_speed);
        d.melee_damage   = ed.melee_damage;
        d.max_hp         = ed.max_hp;
        defs[i] = d;
    }
    return defs;
}

#endif
