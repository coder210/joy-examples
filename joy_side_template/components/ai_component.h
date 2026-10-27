#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H
#include <joy/jmath.h>

// 追击记忆时长（tick）：失去目标（出侦测范围或被墙挡住）后，
// 仍朝"最后已知位置"追击并保持警戒的时长。2 秒 @20Hz。
// 原行为是一出范围立刻回巡逻，观感很呆。
static constexpr int AI_ALERT_TICKS = 40;
// 卡住判定阈值（tick）：连续这么多 tick "想动却没动"即视为被地形卡住。
static constexpr int AI_STUCK_TICKS  = 6;
// 卡住时尝试跳跃的初速（m/s，向上为负）。横版里用来翻越矮墙/台阶。
static constexpr float AI_JUMP_SPEED = 8.0f;

struct AIComponent {
    fp_t patrol_center_x, patrol_center_y; // 巡逻中心点
    fp_t patrol_radius;   // 巡逻范围
    fp_t detect_radius;   // 检测玩家范围
    fp_t attack_radius;   // 攻击范围
    int  state;           // 0=巡逻, 1=追击, 2=攻击
    int  timer;           // 计时器
    fp_t target_x, target_y; // 当前移动目标
    // 远程攻击参数（近战敌人由调用方传 false/0）
    bool ranged;          // 是否远程敌人
    fp_t bullet_speed;    // 子弹速度（m/s）
    fp_t bullet_damage;   // 子弹伤害
    int  shoot_interval;  // 射击间隔（tick）
    int  shoot_cooldown;  // 射击冷却倒计时
    // ---- 按敌人种类取值（enemies.json）----
    fp_t move_speed;      // 移动速度（m/s）：ai_system 用它算每 tick 水平步长
    int  melee_damage;    // 近战伤害：melee_attack_system 结算敌人攻击时用

    // ---- 追击记忆 / 卡住检测 ----
    // target_x/target_y 复用为"最后看到玩家的位置"：看到玩家时写入，失去目标后仍朝它移动。
    // 巡逻与追击不会同时发生，故无需再开字段（巡逻点在回巡逻时重摇，见 ai_system）。
    int  alert_timer = 0; // 警戒倒计时（tick）：>0 表示刚失去目标、仍在追最后已知位置
    int  stuck_timer = 0; // 连续"有移动意图但没位移"的 tick 数（卡住检测）

    // 上一 tick 的 x，用于卡住检测。不同步进快照：它每 tick 都会被覆写，
    // 晚加入端最多误判一次（立刻自愈），不需要占用协议字段。
    fp_t prev_x = 0;

    AIComponent() = default;
    AIComponent(fp_t cx, fp_t cy, fp_t pr, fp_t dr, fp_t ar,
                bool rng, fp_t bs, fp_t bd, int si, int sc,
                fp_t ms, int md)
        : patrol_center_x(cx), patrol_center_y(cy), patrol_radius(pr),
          detect_radius(dr), attack_radius(ar), state(0), timer(0),
          target_x(cx), target_y(cy), ranged(rng), bullet_speed(bs),
          bullet_damage(bd), shoot_interval(si), shoot_cooldown(sc),
          move_speed(ms), melee_damage(md) {}
};

#endif
