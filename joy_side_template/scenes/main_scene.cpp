#include "main_scene.h"
#include <joy/joy.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../layers/menu_layer.h"
#include "../asset_manager.h"
#include "../systems/map_def.h"      // MAP_ASSET_BASE：tilemap 贴图路径前缀

MainScene::MainScene(Context* ctx) : Scene(ctx, "Main") {}

void MainScene::Load() {
    auto* self = this;
    log_info("[main scene] start...");

    int logical_w, logical_h;
    r2d_get_logical_presentation(self->ctx()->r2d, &logical_w, &logical_h, NULL);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        self->particles[i].x = (float)(rand() % logical_w);
        self->particles[i].y = (float)(rand() % logical_h);
        self->particles[i].vx = ((float)rand() / RAND_MAX - 0.5f) * 6.0f;
        self->particles[i].vy = ((float)rand() / RAND_MAX - 0.5f) * 6.0f;
        self->particles[i].alpha = 0.08f + (float)rand() / RAND_MAX * 0.15f;
        self->particles[i].size = 1.5f + (float)rand() / RAND_MAX * 1.5f;
        self->particles[i].phase = (float)rand() / RAND_MAX * 6.2832f;
        self->particles[i].twinkle_speed = 0.8f + (float)rand() / RAND_MAX * 1.5f;
    }

    // ---- 背景 tilemap：levels/main_scene.lua（Tiled 导出，纯装饰）----
    // 路径走资源索引（category="maps"、id="main_scene"），缺失时回退内置默认路径。
    const char* map_rel = AssetManager::GetInstance()
        ? AssetManager::GetInstance()->GetResourcePath(AssetManager::kCatMaps, "main_scene")
        : nullptr;
    if (!map_rel) map_rel = "joy_side_template/levels/main_scene.lua";
    self->tilemap = jtilemap_load_file(map_rel);
    if (self->tilemap) {
        self->tilemap_render = jtilemap_render_create(self->ctx()->r2d, MAP_ASSET_BASE, NULL);
        log_info("[main scene] tilemap loaded: %s (%dx%d)", map_rel,
                 jtilemap_get_width(self->tilemap), jtilemap_get_height(self->tilemap));
    } else {
        log_info("[main scene] tilemap not found: %s (menu bg falls back to particles)", map_rel);
    }

    self->menu = new MenuLayer(self->ctx());   // 纯 button 菜单层，由 Render/HandleUiEvent 桥接
}

void MainScene::Update(float dt) {
    auto* self = this;
    self->elapsed += dt;

    int logical_w, logical_h;
    r2d_get_logical_presentation(self->ctx()->r2d, &logical_w, &logical_h, NULL);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        self->particles[i].x += self->particles[i].vx * dt;
        self->particles[i].y += self->particles[i].vy * dt;
        if (self->particles[i].x < 0) self->particles[i].x += (float)logical_w;
        if (self->particles[i].x > logical_w) self->particles[i].x -= (float)logical_w;
        if (self->particles[i].y < 0) self->particles[i].y += (float)logical_h;
        if (self->particles[i].y > logical_h) self->particles[i].y -= (float)logical_h;
    }
}

void MainScene::Render() {
    auto* self = this;
    jr2d_p r2d = self->ctx()->r2d;
    font_p font = AssetManager::GetInstance()->GetSimheiFont24();

    int w, h;
    r2d_get_logical_presentation(r2d, &w, &h, NULL);

    // ---- 背景 tilemap：等比放大铺满窗口（cover），多余部分居中裁切 ----
    if (self->tilemap && self->tilemap_render) {
        const int mw = jtilemap_get_width(self->tilemap);
        const int mh = jtilemap_get_height(self->tilemap);
        if (mw > 0 && mh > 0) {
            const float tpx_x = (float)w / (float)mw;
            const float tpx_y = (float)h / (float)mh;
            const float tpx   = (tpx_x > tpx_y) ? tpx_x : tpx_y;   // 取较长边铺满，不留边
            jtilemap_render_set_tile_px(self->tilemap_render, tpx);
            // tile0 画在屏幕 (ox,oy)：dst = tx*tilew - cam，故 cam = -ox
            const float ox = ((float)w - (float)mw * tpx) * 0.5f;
            const float oy = ((float)h - (float)mh * tpx) * 0.5f;
            jtilemap_render_tiles(self->tilemap_render, self->tilemap,
                                  -ox, -oy, (uint32_t)get_ticks());
        }
    }

    // ---- 细微粒子 ----
    jcolor_t pc = { 200, 200, 210, 0 };
    for (int i = 0; i < MAX_PARTICLES; i++) {
        struct particle* p = &self->particles[i];
        float twinkle = 0.5f + 0.5f * sinf(self->elapsed * p->twinkle_speed + p->phase);
        float a = p->alpha * twinkle;
        if (a < 0.01f) continue;
        pc.a = (uint8_t)(a * 255.0f);
        r2d_set_color(r2d, pc.r, pc.g, pc.b, pc.a);
        jrect_t pr = { p->x, p->y, p->size, p->size };
        shape_draw_rectangle(r2d, "fill", pr);
    }

    // ---- 标题 ----
    float pulse = 0.8f + 0.2f * sinf(self->elapsed * 1.8f);
    const char* title = "Project Proxy 2D";
    int title_len = (int)strlen(title);
    int tw_est = title_len * 14;
    float tx = (w - tw_est) / 2.0f;
    float ty = 28.0f;
    jcolor_t tc = { 200, 200, 210, (uint8_t)(pulse * 255.0f) };
    jtext_printx(r2d, font, title, title_len, tc, tx, ty);

    // 版本号（底部）
    font_p font_small = AssetManager::GetInstance()->GetSimheiFont12();
    if (font_small) {
        const char* ver = "v1.0.0";
        jcolor_t vc = { 120, 120, 130, 255 };
        jtext_printx(r2d, font_small, ver, (int)strlen(ver), vc, (w - 50) / 2.0f, h - 20);
    }
    // 渲染菜单层（按钮/标题等）：纯 button 层直接绘制
    if (this->menu) this->menu->Render();
}

void MainScene::Destroy() {
    if (this->menu) { delete this->menu; this->menu = NULL; }
    if (this->tilemap_render) { jtilemap_render_destroy(this->tilemap_render); this->tilemap_render = nullptr; }
    if (this->tilemap)        { jtilemap_destroy(this->tilemap); this->tilemap = nullptr; }
}

void MainScene::HandleUiEvent(const void* ev) {
    // 转发事件给 menu_layer（纯 button，内部用 dispatch_event 解析命中）
    if (this->menu) this->menu->HandleEvent(ev);
}




