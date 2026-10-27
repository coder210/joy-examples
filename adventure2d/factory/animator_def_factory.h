#pragma once
#include <joy/janim.h>
#include "../entt.hpp"
#include "../components/animator_def.h"
#include "anim_def_builder.h"

// ========== 创建默认的 warrior 动画定义，存入 registry.ctx() ==========
static inline void create_warrior_anim_def(entt::registry& registry, const char* tex_path) {
    static const int ATK_FRAMES[4] = { 6, 6, 6, 6 };   // attack_d/l/r/u 帧数
    static const int ATK_ROWS[4]   = { 4, 2, 2, 6 };   // attack_d/l/r/u 纹理行号
    janim_warrior_def_t def = {};
    build_actor_anim_def(def, tex_path, 6, 6, ATK_FRAMES, ATK_ROWS);
    registry.ctx().emplace<janim_warrior_def_t>(std::move(def));
}

// ========== 创建 goblin torch 动画定义，存入 registry.ctx() ==========
static inline void create_goblin_anim_def(entt::registry& registry, const char* tex_path) {
    static const int ATK_FRAMES[4] = { 6, 6, 6, 6 };   // attack_d/l/r/u 帧数
    static const int ATK_ROWS[4]   = { 3, 2, 2, 4 };   // 左攻击与右攻击同 row 用 flip_h
    janim_goblin_def_t def = {};
    build_actor_anim_def(def, tex_path, 7, 6, ATK_FRAMES, ATK_ROWS);
    registry.ctx().emplace<janim_goblin_def_t>(std::move(def));
}
