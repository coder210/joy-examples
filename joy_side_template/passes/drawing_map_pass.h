#ifndef DRAWING_MAP_PASS_H
#define DRAWING_MAP_PASS_H

#include <cmath>
#include <joy/joy.h>
#include "../components/global_state_component.h"
#include "../systems/map_def.h"
#include "../protocol.h"

struct Context;

// 地图地形色块渲染：不再使用 Kenney tilesheet 图集，改为按 64×64 网格逐格画纯色块。
// 墙（'#'）=墨色块，地面（'.'/'P'/'E' 等）=浅宣纸色块。视觉为黑白水墨风；
// 格子尺寸每格 1 米（=50px），与实体/相机渲染的 1米=50px 精确一致，保证对齐。
// 地图地形渲染。世界缩放 view_zoom 直接乘进坐标与尺寸：
// 屏幕像素 = (世界米 - 相机米) * 50 * zoom，使整张地图能落进 480×320 逻辑屏。
static void draw_map_pass(Context* ctx)
{
    auto& gs = ctx->ecs.ctx().get<GlobalState>();
    int lw = 480, lh = 320;
    r2d_get_logical_presentation(ctx->r2d, &lw, &lh, NULL);
    // 自动模式(view_zoom<=0)按当前窗口尺寸重算，使地图高度铺满窗口（与相机一致）。
    const float z   = (gs.view_zoom > 0.0f) ? gs.view_zoom : side_auto_view_zoom(lw, lh);
    const float ppm = meters_to_pixels(1.0f) * z;   // 每米屏幕像素（含缩放）

    float cx = meters_to_pixels(gs.camera_position.x) * z;
    float cy = meters_to_pixels(gs.camera_position.y) * z;

    // 1) 地图外区域：整屏"界外"暗色（地图小于视口时不留白）
    r2d_set_color(ctx->r2d, 26, 26, 30, 255);
    jrect_t full = { 0, 0, (float)lw, (float)lh };
    shape_draw_rectangle(ctx->r2d, "fill", full);

    // 2) 地图范围内：浅宣纸底色（= 可通行地面）
    r2d_set_color(ctx->r2d, 250, 248, 244, 255);
    jrect_t floor = { -cx, -cy, ppm * (float)MAP_W, ppm * (float)MAP_H };
    shape_draw_rectangle(ctx->r2d, "fill", floor);

    // 首选：Tiled 瓦片渲染（每格 ppm 像素，相机同为屏幕像素单位，缩放精确对齐）
    if (g_map_info.tilemap) {
        if (!g_map_info.tilemap_render)
                g_map_info.tilemap_render = jtilemap_render_create(ctx->r2d, MAP_ASSET_BASE, NULL);
        if (g_map_info.tilemap_render) {
                jtilemap_render_set_tile_px(g_map_info.tilemap_render, ppm);
                jtilemap_render_tiles(g_map_info.tilemap_render, g_map_info.tilemap, cx, cy, (uint32_t)get_ticks());
                return;
        }
    }

    // 回退：逐格色块（墙 = 墨色）
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!map_is_collidable(map_tile_at(x, y))) continue;
            float wx = ppm * (float)(x + 1) - cx;
            float wy = ppm * (float)(y + 1) - cy;
            if (wx + ppm < 0.0f || wx - ppm > (float)lw) continue;
            if (wy + ppm < 0.0f || wy - ppm > (float)lh) continue;

            r2d_set_color(ctx->r2d, 40, 40, 45, 255);
            jrect_t cell = { wx - ppm, wy - ppm, ppm, ppm };
            shape_draw_rectangle(ctx->r2d, "fill", cell);
        }
    }
}

#endif