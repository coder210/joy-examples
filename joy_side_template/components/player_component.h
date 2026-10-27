#ifndef PLAYER_COMPONENT_H
#define PLAYER_COMPONENT_H

#include "../systems/character_catalog.h"

// 玩家组件：仅含连接标识与角色标识。武器系统（旧持械渲染 + weapons.json 资源）已移除，
// 玩家不再持有可视化武器；近战/远程伤害由 melee_attack_system 等常量与敌人配置决定。
struct PlayerComponent
{
        int conv;
        // 角色 id：对应 character_catalog.h 运行时 catalog 的整数 id
        // （由 characters.json 中出现顺序分配，0 = 首个角色）
        int char_id = 0;
};

#endif
