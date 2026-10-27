#include "splash_scene.h"
#include <joy/joy.h>
#include <stdio.h>
#include "main_scene.h"
#include "../asset_manager.h"

struct splash_scene {
        scene_p scene;
        Context* ctx;
        image_p logo;
        float elapsed;
        float alpha;        // 透明度：0→1（淡入）→1→0（淡出）
        bool faded_in;
        bool fading_out;
};

static void on_load(scene_p s) {
        splash_scene_p self = (splash_scene_p)scene_get_userdata(s);
        self->logo = image_create(self->ctx->r2d, "adventure2d/textures/logo.png");
        log_info("[splash scene] start");
}

static void on_update(scene_p s, float dt) {
        splash_scene_p self = (splash_scene_p)scene_get_userdata(s);
        self->elapsed += dt;

        // 淡入：0~0.5s，透明度从 0→1
        if (self->elapsed < 0.5f) {
                self->alpha = self->elapsed / 0.5f;
        }
        // 保持：0.5s~2.5s，透明度 = 1
        else if (self->elapsed < 2.5f) {
                self->alpha = 1.0f;
                self->faded_in = true;
        }
        // 淡出：2.5s~3.0s，透明度从 1→0
        else if (self->elapsed < 3.0f) {
                self->alpha = 1.0f - (self->elapsed - 2.5f) / 0.5f;
                self->fading_out = true;
        }
        // 3.0s 后切换到主场景
        else {
                self->ctx->pending_scene = main_scene_get_scene(main_scene_create(self->ctx));
                push_user_event(TRANSITION_EVENT, TRANSITION_SPLASH_DONE);
        }
}

static void on_render(scene_p s) {
        splash_scene_p self = (splash_scene_p)scene_get_userdata(s);
        jr2d_p r2d = self->ctx->r2d;
        font_p font = AssetManager::GetInstance()->GetSimheiFont24();

        int w, h;
        r2d_get_logical_presentation(r2d, &w, &h, NULL);

        // 背景
        jcolor_t bg = { 30, 30, 40, 255 };
        r2d_set_color(r2d, bg.r, bg.g, bg.b, bg.a);
        jrect_t full = { 0, 0, (float)w, (float)h };
        shape_draw_rectangle(r2d, "fill", full);

        // Logo（居中）
        image_p logo = self->logo;
        if (logo) {
                jrect_t dest = { (w - 100) / 2.0f, 50, 100, 100 };
                image_draw1(logo, NULL, &dest);
        }

        // 游戏标题（位于 Logo 下方）
        const char* title = "Adventure 2D";
        jcolor_t title_c = { 200, 200, 200, 255 };
        jtext_printx(r2d, font, title, (int)strlen(title), title_c, (w - 200) / 2.0f, 170);

        // 版权信息（底部）
        font_p font_small = AssetManager::GetInstance()->GetSimheiFont12();
        if (font_small) {
                jcolor_t ver_c = { 120, 120, 130, 255 };
                const char* dev = "Developed by 大懒虫工作室, 228407354@qq.com";
                jtext_printx(r2d, font_small, dev, (int)strlen(dev), ver_c, (w - 320) / 2.0f, h - 24);
        }
}

static void on_destroy(scene_p s) {
        splash_scene_p self = (splash_scene_p)scene_get_userdata(s);
        if (self->logo) image_destroy(self->logo);
        delete self;
}

splash_scene_p splash_scene_create(Context* ctx) {
        splash_scene_p self = new splash_scene_t();
        self->scene = scene_create("Splash");
        self->ctx = ctx;
        self->alpha = 0.0f;

        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_update_callback(self->scene, on_update);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);

        return self;
}

scene_p splash_scene_get_scene(splash_scene_p s) {
        return s ? s->scene : NULL;
}