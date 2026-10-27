#ifndef HEALTH_COMPONENT_H
#define HEALTH_COMPONENT_H
#include <joy/jmath.h>

// 玩家初始血量（远程敌人子弹伤害结算用）
static constexpr int PLAYER_MAX_HP = 100;
// 敌人初始血量（玩家远程子弹伤害结算用）
static constexpr int ENEMY_MAX_HP = 50;
// 重生耗时（tick 数）：3 秒 @ 20Hz 逻辑 tick（帧同步计数，避免浮点累积误差）
static constexpr int PLAYER_RESPAWN_TICKS = 60;
// 重生无敌保护（tick 数）：2 秒 @ 20Hz。避免玩家重生回出生点时
// 被周围敌人（尤其加多敌人后）瞬间集火，导致"重生血量 0"的假象。
static constexpr int SPAWN_INVULN_TICKS = 40;

// 敌人尸体保留时长（逻辑 tick）：死亡后先播完死亡动画，再销毁实体使其消失。
// 倒计时基准是"权威逻辑帧号"（见 player_death_system），不是渲染时间——
// 渲染 dt 两端不一致，用它触发销毁会导致两端销毁时机分叉。
// 取值 ≈ Dead Hit(6帧@12fps) + Dead Ground(4帧@12fps) ≈ 0.83s ≈ 17 tick，取 18 留余量。
static constexpr int ENEMY_CORPSE_TICKS = 18;

// 血量组件：子弹命中扣血，HP 通过快照同步给客户端 HUD 显示。
// 数值与计时均为整数（逻辑层不用浮点，保证两端确定性）。
// 出生点见 SpawnPointComponent（地图数据，与血量状态解耦）。
struct HealthComponent {
    int  hp;            // 当前血量
    int  max_hp;        // 最大血量
    bool dead;          // 死亡标记（帧同步确定性：两端同一 tick 置位/清除，不通过快照同步）
    int  respawn_timer; // 重生倒计时（tick 数），死亡期间由 player_death_system 每 tick 递减
    int  spawn_invuln;  // 重生无敌保护（tick 数），>0 时免疫伤害，由 player_death_system 每 tick 递减
    // 敌人死亡时所在的"权威逻辑帧号"，尸体销毁倒计时以它为基准（player_death_system）。
    // 不入协议：两端都在同一逻辑帧把 hp 打到 0 → 记下同一个帧号，天然一致；
    // 仅"中途加入的客户端"从快照接管尸体时以接管帧为起点（晚若干帧销毁），
    // 该差异是良性的——死亡敌人已排除出帧同步校验（见 CollectInput / verifyFrameSyncImpl）。
    int  death_frame = -1;

    HealthComponent() = default;
    HealthComponent(int h, int m, bool d = false, int rt = 0, int si = 0)
        : hp(h), max_hp(m), dead(d), respawn_timer(rt), spawn_invuln(si) {}
};

#endif
