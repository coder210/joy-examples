#ifndef CAMERA_FOLLOW_PASS_H
#define CAMERA_FOLLOW_PASS_H
#include "../components/global_state_component.h"
#include "../components/player_component.h"
#include "../components/transform_component.h"
#include "../protocol.h"
#include "../context.h"
#include "../systems/map_def.h"   // MAP_W/MAP_H：横版相机夹取与自动缩放用

// 将摄像机跟随指定玩家，使玩家尽量居中于屏幕。
// 视口尺寸（米）= 实际窗口 / (PPM*zoom)，zoom 由"窗口高度"推导使地图高度恰好铺满窗口
// （消除上下黑边）；宽度随窗口比例自适应。相机做"边界夹取"把视口夹进 [0,MAP_W]×[0,MAP_H]，
// 避免露出界外底色。地图比视口窄时退化为居中（两侧等量底色）。
static void camera_follow_pass(entt::registry& registry, Context* ctx, float /*dt*/) {
    auto& gs = registry.ctx().get<GlobalState>();
    if (gs.camera_follow_conv < 0) return;

    // 自动模式(view_zoom<=0)按当前窗口尺寸重算，使地图高度铺满窗口（适配全屏/缩放）。
    int lw = 480, lh = 320;
    r2d_get_logical_presentation(ctx->r2d, &lw, &lh, NULL);
    const float ppm = meters_to_pixels(1.0f);
    if (gs.view_zoom <= 0.0f) gs.view_zoom = side_auto_view_zoom(lw, lh);
    const float z = gs.view_zoom;
    // 视口在世界里的实际尺寸（米）= 窗口 / (每米像素 * zoom)
    const float view_w = (float)lw / (ppm * z);
    const float view_h = (float)lh / (ppm * z);

    registry.view<PlayerComponent, TransformComponent>().each(
        [&gs, view_w, view_h](auto /*e*/, PlayerComponent& pc, TransformComponent& t)
    {
        if (pc.conv == gs.camera_follow_conv) {
            // ---- X：跟随 + 前视偏置 → 夹取到地图边界 ----
            float cx = t.position.x - view_w * 0.5f + gs.camera_bias_x;
            const float max_x = (float)MAP_W - view_w;
            if (max_x <= 0.0f) {
                cx = max_x * 0.5f;                       // 地图比视口窄：居中
            } else {
                if (cx < 0.0f) cx = 0.0f;
                if (cx > max_x) cx = max_x;
            }
            gs.camera_position.x = cx;

            // ---- Y：side_view 跟随玩家；否则用 camera_lock_y 手动锁 / 随玩家上下 ----
            float cy;
            if (gs.side_view) {
                cy = t.position.y - view_h * 0.5f;
            } else {
                cy = (gs.camera_lock_y >= 0.0f) ? (gs.camera_lock_y - view_h * 0.5f)
                                                : (t.position.y - view_h * 0.5f);
            }
            const float max_y = (float)MAP_H - view_h;
            if (max_y <= 0.0f) {
                cy = max_y * 0.5f;                       // 地图比视口矮：居中
            } else {
                if (cy < 0.0f) cy = 0.0f;
                if (cy > max_y) cy = max_y;
            }
            gs.camera_position.y = cy;
        }
    });
}

#endif