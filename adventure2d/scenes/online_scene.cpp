#include "online_scene.h"
#include "../asset_manager.h"

struct online_scene {
        scene_p scene;
        Context* ctx;
        scene_node_p ui_node;
        button_p btn_host, btn_client, btn_server, btn_back;
};

static void setup_button_style(button_p btn) {
        jcolor_t normal  = { 255, 255, 255, 255 };
        jcolor_t hover   = { 220, 225, 235, 255 };
        jcolor_t pressed = { 200, 200, 210, 255 };
        button_set_normal_color(btn, normal);
        button_set_hover_color(btn, hover);
        button_set_pressed_color(btn, pressed);
}

static scene_node_p make_button_node(button_p btn, int x, int y) {
        scene_node_p node = scene_node_create();
        scene_node_set_position(node, (float)x, (float)y);
        scene_node_set_userdata(node, btn);
        scene_node_set_zorder(node, 100);
        scene_node_set_update_callback(node, [](scene_node_p n, float dt) {
                button_p b = (button_p)scene_node_get_userdata(n);
                vec2_t world_pos;
                scene_node_get_world_position(n, &world_pos.x, &world_pos.y);
                button_set_position(b, world_pos.x, world_pos.y);
        });
        scene_node_set_render_callback(node, [](scene_node_p n, const void* arg) {
                button_p b = (button_p)scene_node_get_userdata(n);
                button_draw(b);
        });
        return node;
}

static void on_event(scene_node_p n, const void* e) {
        online_scene_p self = (online_scene_p)scene_node_get_userdata(n);
        jevent_callbacks_t cbs = {};
        cbs.on_mousepressed = [](float x, float y, int btn, void* arg) {
                auto* self = (online_scene_p)arg;
                auto hit = [&](button_p b) { return button_is_pressed(b, x, y); };
                if (self->btn_host && hit(self->btn_host))
                        push_user_event(ONLINE_EVENT, ONLINE_EVENT_HOST);
                if (self->btn_client && hit(self->btn_client))
                        push_user_event(ONLINE_EVENT, ONLINE_EVENT_CLIENT);
                if (self->btn_server && hit(self->btn_server))
                        push_user_event(ONLINE_EVENT, ONLINE_EVENT_SERVER);
                if (self->btn_back && hit(self->btn_back))
                        push_user_event(ONLINE_EVENT, ONLINE_EVENT_BACK);
        };
        cbs.on_mousereleased = [](float x, float y, int btn, void* arg) {
                auto* self = (online_scene_p)arg;
                if (self->btn_host)    button_set_pressed(self->btn_host, false);
                if (self->btn_client)  button_set_pressed(self->btn_client, false);
                if (self->btn_server)  button_set_pressed(self->btn_server, false);
                if (self->btn_back)    button_set_pressed(self->btn_back, false);
        };
        cbs.on_mousemoved = [](float x, float y, float dx, float dy, void* arg) {
                auto* self = (online_scene_p)arg;
                if (self->btn_host)    button_is_hovered(self->btn_host, x, y);
                if (self->btn_client)  button_is_hovered(self->btn_client, x, y);
                if (self->btn_server)  button_is_hovered(self->btn_server, x, y);
                if (self->btn_back)    button_is_hovered(self->btn_back, x, y);
        };
        dispatch_event(cbs, e, self);
}

static void on_load(scene_p s) {
        online_scene_p self = (online_scene_p)scene_get_userdata(s);
        log_info("[online scene] start...");

        int w, h;
        r2d_get_logical_presentation(self->ctx->r2d, &w, &h, NULL);
        font_p font = AssetManager::GetInstance()->GetSimheiFont24();

        self->ui_node = scene_node_create();
        scene_node_set_userdata(self->ui_node, self);
        scene_node_set_event_callback(self->ui_node, on_event);
        scene_node_set_zorder(self->ui_node, 100);
        scene_add_root_node(self->scene, self->ui_node);

        int btn_w = 120, btn_h = 50, gap = 20;
        int btn_x = (w - btn_w) / 2;
        int start_y = (h - btn_h * 4 - gap * 3) / 2;

        // 标题提示
        scene_node_p title_node = scene_node_create();
        scene_node_set_zorder(title_node, 90);
        scene_node_set_render_callback(title_node, [](scene_node_p n, const void* arg) {
                online_scene_p self = (online_scene_p)scene_node_get_userdata(n);
                jr2d_p r2d = (jr2d_p)arg;
                font_p font = AssetManager::GetInstance()->GetSimheiFont24();
                const char* text = "选择联机模式";
                int len = (int)strlen(text);
                int w_est = len * 14;
                int w, h;
                r2d_get_logical_presentation(r2d, &w, &h, NULL);
                jcolor_t c = { 200, 200, 210, 255 };
                jtext_printx(r2d, font, text, len, c, (w - w_est) / 2.0f, 30);
        });
        scene_node_set_userdata(title_node, self);
        scene_add_root_node(self->scene, title_node);

        // 房主按钮
        self->btn_host = button_create(self->ctx->r2d, 0.0f, 0.0f, (float)btn_w / 480.0f, (float)btn_h / 320.0f);
        setup_button_style(self->btn_host);
        button_set_text_offset(self->btn_host, 0, 3.0f);
        button_set_textx(self->btn_host, font, "房主", (int)strlen("房主"), { 0, 0, 0, 255 });
        scene_node_add_child(self->ui_node, make_button_node(self->btn_host, btn_x, start_y));

        // 客户端按钮
        self->btn_client = button_create(self->ctx->r2d, 0.0f, 0.0f, (float)btn_w / 480.0f, (float)btn_h / 320.0f);
        setup_button_style(self->btn_client);
        button_set_text_offset(self->btn_client, 0, 3.0f);
        button_set_textx(self->btn_client, font, "客户端", (int)strlen("客户端"), { 0, 0, 0, 255 });
        scene_node_add_child(self->ui_node, make_button_node(self->btn_client, btn_x, start_y + (btn_h + gap)));

        // 服务端按钮
        self->btn_server = button_create(self->ctx->r2d, 0.0f, 0.0f, (float)btn_w / 480.0f, (float)btn_h / 320.0f);
        setup_button_style(self->btn_server);
        button_set_text_offset(self->btn_server, 0, 3.0f);
        button_set_textx(self->btn_server, font, "服务端", (int)strlen("服务端"), { 0, 0, 0, 255 });
        scene_node_add_child(self->ui_node, make_button_node(self->btn_server, btn_x, start_y + (btn_h + gap) * 2));

        // 返回按钮
        self->btn_back = button_create(self->ctx->r2d, 0.0f, 0.0f, (float)btn_w / 480.0f, (float)btn_h / 320.0f);
        setup_button_style(self->btn_back);
        button_set_text_offset(self->btn_back, 0, 3.0f);
        button_set_textx(self->btn_back, font, "返回", (int)strlen("返回"), { 0, 0, 0, 255 });
        scene_node_add_child(self->ui_node, make_button_node(self->btn_back, btn_x, start_y + (btn_h + gap) * 3));
}

static void on_render(scene_p s) {
        (void)s;
}

static void on_destroy(scene_p s) {
        online_scene_p self = (online_scene_p)scene_get_userdata(s);
        delete self;
}

online_scene_p online_scene_create(Context* ctx) {
        online_scene_p self = new online_scene_t();
        self->scene = scene_create("Online");
        self->ctx = ctx;

        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);

        return self;
}

scene_p online_scene_get_scene(online_scene_p s) {
        return s ? s->scene : NULL;
}