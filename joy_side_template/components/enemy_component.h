#ifndef ENEMY_COMPONENT_H
#define ENEMY_COMPONENT_H

struct EnemyComponent {};
// 记录该敌人在 ENEMY_SPAWNS 中的索引，打包快照时写入 ai_type 字段
struct SpawnIdxComponent { int idx; };

// 敌人种类 id（EnemyCatalog 下标 / enemies.json 中的顺序）。
// 帧同步：种类由 Tiled 出生点的对象 type/name 确定性派生（两端解析同一份 .lua），
// 且已隐含在 SpawnDef（由快照的 ai_type=SpawnIdxComponent.idx 反查）里，
// 因此无需进快照、不改协议；组件只是给渲染层(draw_enemy_pass)直接取用，省去每次反查。
struct EnemyTypeComponent { int type = 0; };

#endif
