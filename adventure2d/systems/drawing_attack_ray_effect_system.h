#ifndef DRAWING_ATTACK_RAY_EFFECT_SYSTEM_H
#define DRAWING_ATTACK_RAY_EFFECT_SYSTEM_H
#include "../components/attack_ray_effect_component.h"
#include "../components/global_state_component.h"
#include "../protocol.h"

struct Context;

static void 
drawing_attack_ray_effect_system(Context* ctx, 
        AttackRayEffectComponent& effect)
{
        auto& cam = ctx->ecs.ctx().get<GlobalState>().camera_position;
        jrect_t line;
        line.x = meters_to_pixels(effect.x - cam.x);
        line.y = meters_to_pixels(effect.y - cam.y);
        line.w = meters_to_pixels(effect.w);
        line.h = meters_to_pixels(effect.h);
        r2d_set_color(ctx->r2d, 255, 255, 255, 255);
        shape_draw_rectangle(ctx->r2d, "fill", line);
}


#endif
