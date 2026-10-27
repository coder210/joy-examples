// animator_def.h
#pragma once
// 本文件保持向后兼容，实质定义已迁至 joy/janim.h
#include <joy/janim.h>

using AnimCondMode           = anim_cond_mode_t;
using AnimCondDef            = anim_cond_t;
using AnimTransitionDef      = anim_transition_t;
using AnimClipDef            = anim_clip_def_t;
using AnimStateDef           = anim_state_t;

// 各纹理对应的动画定义：用 JANIM_DEFINE 生成独立具名类型，
// 可直接作为 registry.ctx() 组件（类型唯一，无需再包一层结构体）
JANIM_DEFINE(warrior);
JANIM_DEFINE(goblin);
