// anim_def_builder.h
#pragma once
#include <joy/janim.h>

// 通用"类人形"动画定义构建器：
// 布局 = idle + walk + 4 方向攻击 + 全套过渡，所有角色共用同一结构，
// 仅帧数/纹理行号不同。DefT 是 JANIM_DEFINE(name) 生成的类型（内含 anim_def_t layout）。
template<typename DefT>
static inline void build_actor_anim_def(DefT& def, const char* tex_path,
    int n_idle, int n_walk, const int atk_frames[4], const int atk_rows[4])
{
    const float DUR = 0.15f;
    const int FW = 192, FH = 192;
    anim_def_t& d = def.layout;

    // idle (row 0)
    int s_idle = anim_add_state(&d, "idle", true);
    for (int i = 0; i < n_idle; i++) {
        jrect_t r = { (float)(i * FW), 0, (float)FW, (float)FH };
        anim_add_clip(&d, s_idle, tex_path, DUR, r);
    }

    // walk (row 1)
    int s_walk = anim_add_state(&d, "walk", true);
    for (int i = 0; i < n_walk; i++) {
        jrect_t r = { (float)(i * FW), (float)(1 * FH), (float)FW, (float)FH };
        anim_add_clip(&d, s_walk, tex_path, DUR, r);
    }

    // attack directions（行号由各角色纹理决定）
    const char* atk_names[] = { "attack_d","attack_l","attack_r","attack_u" };
    int s_atk[4];
    for (int k = 0; k < 4; k++) {
        s_atk[k] = anim_add_state(&d, atk_names[k], false);
        for (int i = 0; i < atk_frames[k]; i++) {
            jrect_t r = { (float)(i * FW), (float)(atk_rows[k] * FH), (float)FW, (float)FH };
            anim_add_clip(&d, s_atk[k], tex_path, DUR, r);
        }
    }

    // transitions
    int t;
    t = anim_add_transition(&d, s_idle, s_walk);
    anim_transition_add_cond(&d, t, "speed", ANIM_COND_GREATER, 0.5f);
    anim_transition_set_exit_time(&d, t, 0.05f);

    t = anim_add_transition(&d, s_walk, s_idle);
    anim_transition_add_cond(&d, t, "speed", ANIM_COND_LESS, 0.5f);
    anim_transition_set_exit_time(&d, t, 0.05f);

    for (int k = 0; k < 4; k++) {
        t = anim_add_transition(&d, -1, s_atk[k]);
        anim_transition_add_cond(&d, t, "attack", ANIM_COND_TRIGGER, 0);
        anim_transition_add_cond(&d, t, "dir", ANIM_COND_EQUALS, (float)k);
    }
    for (int k = 0; k < 4; k++) {
        t = anim_add_transition(&d, s_atk[k], s_idle);
        anim_transition_set_exit_time(&d, t, (float)(atk_frames[k] * DUR));
    }

    anim_set_default_state(&d, s_idle);
}
