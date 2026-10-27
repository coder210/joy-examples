#include "loading_scene.h"
#include <joy/joy.h>
#include "../layers/menu_layer.h"

struct logo_data {
        image_p image;
        int logical_w;
        int logical_h;
};

struct deco_data {
        image_p image;
        int logical_w;
        int logical_h;
};

struct loading_scene {
        scene_p scene;
        Context* ctx;
};

static void on_load(scene_p s) {
        loading_scene_p self = (loading_scene_p)scene_get_userdata(s);
        log_info("[loading scene] start...");

        // 添加菜单层
        menu_layer_p menu = create_menu_layer(self->ctx);
        scene_add_root_node(self->scene, menu_layer_get_node(menu));

        int logical_w, logical_h;
        r2d_get_logical_presentation(self->ctx->r2d, &logical_w, &logical_h, NULL);

        // ---- Logo ----
        logo_data* ld = new logo_data{};
        ld->image = image_create(self->ctx->r2d, "adventure2d/textures/logo.png");
        ld->logical_w = logical_w;
        ld->logical_h = logical_h;
        scene_node_p logo_node = scene_node_create();
        scene_node_set_position(logo_node, 0, 0);
        scene_node_set_zorder(logo_node, 50);
        scene_node_set_userdata(logo_node, ld);
        scene_node_set_render_callback(logo_node, [](scene_node_p n, const void* arg) {
                (void)arg;
                logo_data* ld = (logo_data*)scene_node_get_userdata(n);
                if (!ld || !ld->image) return;
                jrect_t dest = { (ld->logical_w - 400) / 2.0f, 40.0f, 100.0f, 100.0f };
                image_draw1(ld->image, NULL, &dest);
                });
        scene_node_set_destroy_callback(logo_node, [](scene_node_p n) {
                logo_data* ld = (logo_data*)scene_node_get_userdata(n);
                if (ld) {
                        if (ld->image) image_destroy(ld->image);
                        delete ld;
                }
                });
        scene_add_root_node(self->scene, logo_node);

        // ---- 左侧装饰 ----
        deco_data* dd_l = new deco_data{};
        dd_l->image = image_create(self->ctx->r2d, "adventure2d/textures/deco/01.png");
        dd_l->logical_w = logical_w;
        dd_l->logical_h = logical_h;
        scene_node_p deco_left = scene_node_create();
        scene_node_set_position(deco_left, 0, 0);
        scene_node_set_zorder(deco_left, 40);
        scene_node_set_userdata(deco_left, dd_l);
        scene_node_set_render_callback(deco_left, [](scene_node_p n, const void* arg) {
                (void)arg;
                deco_data* dd = (deco_data*)scene_node_get_userdata(n);
                if (!dd || !dd->image) return;
                jrect_t dest = { 20.0f, (dd->logical_h - 200) / 2.0f, 80.0f, 200.0f };
                image_draw1(dd->image, NULL, &dest);
                });
        scene_node_set_destroy_callback(deco_left, [](scene_node_p n) {
                deco_data* dd = (deco_data*)scene_node_get_userdata(n);
                if (dd) {
                        if (dd->image) image_destroy(dd->image);
                        delete dd;
                }
                });
        scene_add_root_node(self->scene, deco_left);

        // ---- 右侧装饰 ----
        deco_data* dd_r = new deco_data{};
        dd_r->image = image_create(self->ctx->r2d, "adventure2d/textures/deco/02.png");
        dd_r->logical_w = logical_w;
        dd_r->logical_h = logical_h;
        scene_node_p deco_right = scene_node_create();
        scene_node_set_position(deco_right, 0, 0);
        scene_node_set_zorder(deco_right, 40);
        scene_node_set_userdata(deco_right, dd_r);
        scene_node_set_render_callback(deco_right, [](scene_node_p n, const void* arg) {
                (void)arg;
                deco_data* dd = (deco_data*)scene_node_get_userdata(n);
                if (!dd || !dd->image) return;
                jrect_t dest = { (float)(dd->logical_w - 100), (dd->logical_h - 200) / 2.0f, 80.0f, 200.0f };
                image_draw1(dd->image, NULL, &dest);
                });
        scene_node_set_destroy_callback(deco_right, [](scene_node_p n) {
                deco_data* dd = (deco_data*)scene_node_get_userdata(n);
                if (dd) {
                        if (dd->image) image_destroy(dd->image);
                        delete dd;
                }
                });
        scene_add_root_node(self->scene, deco_right);
}

static void on_update(scene_p s, float dt) {
        (void)s;
        (void)dt;
}

static void on_render(scene_p s) {
        (void)s;
}

static void on_destroy(scene_p s) {
        loading_scene_p self = (loading_scene_p)scene_get_userdata(s);
        delete self;
}

loading_scene_p loading_scene_create(Context* ctx) {
        loading_scene_p self = new loading_scene_t();
        self->scene = scene_create("Loading");
        self->ctx = ctx;

        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_update_callback(self->scene, on_update);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);

        return self;
}

void loading_scene_destroy(loading_scene_p s) {
        if (s) {
                if (s->scene) {
                        scene_destroy(s->scene);
                }
                delete s;
        }
}

scene_p loading_scene_get_scene(loading_scene_p s) {
        return s ? s->scene : NULL;
}
