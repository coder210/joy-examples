#ifndef SPAWN_DEF_H
#define SPAWN_DEF_H

// 敌人生成位置和参数（服务端和客户端共享，保证初始条件一致）
struct SpawnDef {
    float x, y;           // 位置
    float patrol_radius;  // 巡逻半径
    float detect_radius;  // 检测范围
    float attack_radius;  // 攻击范围
};

inline const SpawnDef ENEMY_SPAWNS[] = {
    { 10.0f, 10.0f, 3.0f, 5.0f, 1.5f },
    //{ 3.0f,  12.0f, 2.0f, 4.0f, 1.5f },
    // // { 15.0f, 8.0f,  4.0f, 6.0f, 1.5f },
};
inline constexpr int ENEMY_SPAWN_COUNT = 1;

#endif