#pragma once
#include "../context.h"
#include "../asset_manager.h"
#include "../protocol.h"
#include "../components/sprite_sheet_component.h"
#include "../components/action_state_component.h"
#include "../components/transform_component.h"
#include "../components/global_state_component.h"
#include "../components/logic_rect_component.h"

static void sprite_render_system(Context* ctx,
    TransformComponent& t, LogicRectComponent& r,
    SpriteSheetComponent& ss, AnimationFrameComponent& af,
    ActionStateComponent& act)
{
    auto& cam = ctx->ecs.ctx().get<GlobalState>().camera_position;
    float scr_x = meters_to_pixels(t.position.x - cam.x);
    float scr_y = meters_to_pixels(t.position.y - cam.y);

    image_p tex = AssetManager::GetInstance()->tex_cache_get(ctx->r2d, ss.path);
    if (!tex) return;

    jrect_t src = {
        (float)(af.cur_frame * ss.frame_w),
        (float)(af.cur_row * ss.frame_h),
        (float)ss.frame_w, (float)ss.frame_h
    };

    float body_w = meters_to_pixels(fp_to_float(r.width));
    float body_h = meters_to_pixels(fp_to_float(r.height));
    float off_x = (ss.frame_w - body_w) / 2.0f;
    float off_y = (ss.frame_h - body_h) / 2.0f;

    float draw_x = scr_x - off_x;
    float draw_y = scr_y - off_y;
    float scale_x = (act.dir == 1) ? -1.0f : 1.0f;
    float cx = draw_x + ss.frame_w * 0.5f;
    float cy = draw_y + ss.frame_h * 0.5f;
    image_draw_ex(tex, &src, { cx, cy }, 0, { scale_x, 1 }, { ss.frame_w * 0.5f, ss.frame_h * 0.5f });
}
