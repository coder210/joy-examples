#include "main_scene.h"
#include <joy/joy.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../layers/menu_layer.h"
#include "../asset_manager.h"

#define MAX_PARTICLES 30

struct particle {
        float x, y;
        float vx, vy;
        float alpha;
        float size;
        float phase;
        float twinkle_speed;
};

struct main_scene {
        scene_p scene;
        Context* ctx;
        struct particle particles[MAX_PARTICLES];
        float elapsed;
};

static void on_load(scene_p s) {
        main_scene_p self = (main_scene_p)scene_get_userdata(s);
        log_info("[main scene] start...");

        int logical_w, logical_h;
        r2d_get_logical_presentation(self->ctx->r2d, &logical_w, &logical_h, NULL);

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

        menu_layer_p menu = create_menu_layer(self->ctx);
        scene_add_root_node(self->scene, menu_layer_get_node(menu));
}

static void on_update(scene_p s, float dt) {
        main_scene_p self = (main_scene_p)scene_get_userdata(s);
        self->elapsed += dt;

        int logical_w, logical_h;
        r2d_get_logical_presentation(self->ctx->r2d, &logical_w, &logical_h, NULL);

        for (int i = 0; i < MAX_PARTICLES; i++) {
                self->particles[i].x += self->particles[i].vx * dt;
                self->particles[i].y += self->particles[i].vy * dt;
                if (self->particles[i].x < 0) self->particles[i].x += (float)logical_w;
                if (self->particles[i].x > logical_w) self->particles[i].x -= (float)logical_w;
                if (self->particles[i].y < 0) self->particles[i].y += (float)logical_h;
                if (self->particles[i].y > logical_h) self->particles[i].y -= (float)logical_h;
        }
}

static void on_render(scene_p s) {
        main_scene_p self = (main_scene_p)scene_get_userdata(s);
        jr2d_p r2d = self->ctx->r2d;
        font_p font = AssetManager::GetInstance()->GetSimheiFont24();

        int w, h;
        r2d_get_logical_presentation(r2d, &w, &h, NULL);

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
        const char* title = "Adventure 2D";
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
}

static void on_destroy(scene_p s) {
        main_scene_p self = (main_scene_p)scene_get_userdata(s);
        delete self;
}

main_scene_p main_scene_create(Context* ctx) {
        main_scene_p self = new main_scene_t();
        self->scene = scene_create("Main");
        self->ctx = ctx;

        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_update_callback(self->scene, on_update);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);

        return self;
}

void main_scene_destroy(main_scene_p s) {
        if (s) {
                if (s->scene) {
                        scene_destroy(s->scene);
                }
                delete s;
        }
}

scene_p main_scene_get_scene(main_scene_p s) {
        return s ? s->scene : NULL;
}