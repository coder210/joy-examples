#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H
#include <joy/jmath.h>

struct AIComponent {
    fp_t patrol_center_x, patrol_center_y; // 巡逻中心点
    fp_t patrol_radius;   // 巡逻范围
    fp_t detect_radius;   // 检测玩家范围
    fp_t attack_radius;   // 攻击范围
    int  state;           // 0=巡逻, 1=追击, 2=攻击
    int  timer;           // 计时器
    fp_t target_x, target_y; // 当前移动目标
};

#endif
