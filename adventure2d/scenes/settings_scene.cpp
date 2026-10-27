#include <string.h>
#include <cstdlib>
#include "settings_scene.h"
#include "../asset_manager.h"

struct settings_scene {
        scene_p scene;
        Context* ctx;
        scene_node_p ui_node;
        combobox_input_p ip_input;   // 可输入下拉列表框（本机 IP 选择）
        inputbox_p port_input;
        slider_p bgm_slider;
        slider_p sfx_slider;
        checkbox_p fullscreen_cb;
        button_p btn_back;
};

// 白底黑框按钮：与菜单层一致的无贴图风格
static void setup_button_style(button_p btn) {
        jcolor_t normal  = { 255, 255, 255, 255 };
        jcolor_t hover   = { 255, 255, 255, 255 };
        jcolor_t pressed = { 255, 255, 255, 255 };
        button_set_normal_color(btn, normal);
        button_set_hover_color(btn, hover);
        button_set_pressed_color(btn, pressed);
}

static void apply_fullscreen(Context* ctx)
{
	window_set_fullscreen(ctx->window, ctx->settings.fullscreen);
}

// 把当前 UI 值同步到 settings 并自动保存（变化即写盘）
static void sync_settings(settings_scene_p self)
{
        Context* ctx = self->ctx;
        bool changed = false;

        const char* ip = combobox_input_get_text(self->ip_input);
        if (strcmp(ip, ctx->settings.server_ip) != 0) {
                snprintf(ctx->settings.server_ip, sizeof(ctx->settings.server_ip), "%s", ip);
                changed = true;
        }

        int port = atoi(inputbox_get_text(self->port_input));
        if (port != ctx->settings.server_port) {
                ctx->settings.server_port = port;
                changed = true;
        }

        float bgm = slider_get_value(self->bgm_slider);
        if (bgm != ctx->settings.bgm_volume) {
                ctx->settings.bgm_volume = bgm;
                if (ctx->bgm) audio_bgm_set_gain(ctx->bgm, bgm);
                changed = true;
        }

        float sfx = slider_get_value(self->sfx_slider);
        if (sfx != ctx->settings.sfx_volume) {
                ctx->settings.sfx_volume = sfx;
                if (ctx->step_sound) audio_shot_set_gain(ctx->step_sound, sfx);
                changed = true;
        }

        bool fs = (checkbox_get_state(self->fullscreen_cb) == JOY_CHECKBOX_CHECKED);
        if (fs != ctx->settings.fullscreen) {
                ctx->settings.fullscreen = fs;
                apply_fullscreen(ctx);
                changed = true;
        }

        if (changed) settings_save(&ctx->settings);
}

static void on_event(scene_node_p n, const void* e)
{
        settings_scene_p self = (settings_scene_p)scene_node_get_userdata(n);
        jevent_callbacks_t cbs = {};
        cbs.on_process_event = [](const void* ev, void* arg) {
                settings_scene_p self = (settings_scene_p)arg;
                if (self->ip_input)    combobox_input_process_event(self->ip_input, ev);
                if (self->port_input)  inputbox_process_event(self->port_input, ev);
                if (self->bgm_slider)  slider_process_event(self->bgm_slider, ev);
                if (self->sfx_slider)  slider_process_event(self->sfx_slider, ev);
                if (self->fullscreen_cb) checkbox_process_event(self->fullscreen_cb, ev);
        };
        cbs.on_mousepressed = [](float x, float y, int btn, void* arg) {
                settings_scene_p self = (settings_scene_p)arg;
                if (self->btn_back && button_is_pressed(self->btn_back, x, y))
                        push_user_event(SETTINGS_EVENT, SETTINGS_EVENT_BACK);
                // 点击下拉列表项：选中后填入输入框
                if (self->ip_input) {
                        int hit = combobox_input_hit_item(self->ip_input, x, y);
                        if (hit >= 0) {
                                // 选中项已通过 process_event 内部处理，此处只需关闭下拉
                                // 但 process_event 处理的是 SDL 事件，而 mousepressed 是自定义回调
                                // 所以这里手动处理选中
                                char ips[16][JOY_MAX_IP];
                                int count = get_local_ips(ips, 16);
                                if (hit < count) {
                                        combobox_input_set_text(self->ip_input, ips[hit]);
                                }
                        }
                }
        };
        cbs.on_mousereleased = [](float x, float y, int btn, void* arg) {
                settings_scene_p self = (settings_scene_p)arg;
                if (self->btn_back) button_set_pressed(self->btn_back, false);
        };
        cbs.on_mousemoved = [](float x, float y, float dx, float dy, void* arg) {
                settings_scene_p self = (settings_scene_p)arg;
                if (self->btn_back) button_is_hovered(self->btn_back, x, y);
        };
        dispatch_event(cbs, e, self);
}

static void on_update(scene_p s, float dt) {
        (void)dt;
        settings_scene_p self = (settings_scene_p)scene_get_userdata(s);
        sync_settings(self);
}

static void on_render(scene_p s) {
        settings_scene_p self = (settings_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;
        jr2d_p r2d = ctx->r2d;
        font_p font24 = AssetManager::GetInstance()->GetSimheiFont24();
        font_p font28 = AssetManager::GetInstance()->GetSimheiFont28();

        // 标题
        jcolor_t title_c = { 30, 30, 40, 255 };
        jtext_printx(r2d, font28, "设置", (int)strlen("设置"), title_c, 216, 12);

        // 行标签
        jcolor_t label_c = { 50, 50, 60, 255 };
        jtext_printx(r2d, font24, "服务器 IP", (int)strlen("服务器 IP"), label_c, 20, 52);
        jtext_printx(r2d, font24, "端口", (int)strlen("端口"), label_c, 20, 92);
        jtext_printx(r2d, font24, "音乐", (int)strlen("音乐"), label_c, 20, 132);
        jtext_printx(r2d, font24, "音效", (int)strlen("音效"), label_c, 20, 172);

        // 音量百分比
        char pct[16];
        jcolor_t pct_c = { 50, 50, 60, 255 };
        snprintf(pct, sizeof(pct), "%d%%", (int)(slider_get_value(self->bgm_slider) * 100.0f));
        jtext_printx(r2d, font24, pct, (int)strlen(pct), pct_c, 330, 132);
        snprintf(pct, sizeof(pct), "%d%%", (int)(slider_get_value(self->sfx_slider) * 100.0f));
        jtext_printx(r2d, font24, pct, (int)strlen(pct), pct_c, 330, 172);

        // 控件（先画所有普通控件，下拉列表最后画以保证层级在最上层）
        if (self->ip_input) combobox_input_draw(self->ip_input);
        if (self->port_input) inputbox_draw(self->port_input);
        if (self->bgm_slider) slider_draw(self->bgm_slider);
        if (self->sfx_slider) slider_draw(self->sfx_slider);
        if (self->fullscreen_cb) checkbox_draw(self->fullscreen_cb, r2d, font24);
        if (self->btn_back) button_draw(self->btn_back);
        // 下拉列表最后绘制，确保不被其他控件遮挡
        // 普通控件绘制由 scene_node_render 完成，下拉列表由弹层节点自动渲染
}

static void on_destroy(scene_p s) {
        settings_scene_p self = (settings_scene_p)scene_get_userdata(s);
        if (self->ip_input) combobox_input_destroy(self->ip_input);
        if (self->port_input) inputbox_destroy(self->port_input);
        if (self->bgm_slider) slider_destroy(self->bgm_slider);
        if (self->sfx_slider) slider_destroy(self->sfx_slider);
        if (self->fullscreen_cb) checkbox_destroy(self->fullscreen_cb);
        if (self->btn_back) button_destroy(self->btn_back);
        delete self;
}

static void on_load(scene_p s) {
        settings_scene_p self = (settings_scene_p)scene_get_userdata(s);
        Context* ctx = self->ctx;
        log_info("[settings scene] start...");
        font_p font24 = AssetManager::GetInstance()->GetSimheiFont24();

        self->ui_node = scene_node_create();
        scene_node_set_userdata(self->ui_node, self);
        scene_node_set_zorder(self->ui_node, 100);
        scene_node_set_event_callback(self->ui_node, on_event);
        scene_add_root_node(self->scene, self->ui_node);

        // 可输入下拉列表框（服务器 IP）
                self->ip_input = combobox_input_create(ctx->r2d, 0.22917f, 0.15f, 0.52083f, 0.075f, font24, 256);
        combobox_input_set_text(self->ip_input, ctx->settings.server_ip);
        // 创建弹层节点并注册到场景
        scene_node_p popup = scene_node_create();
        scene_set_popup_node(self->scene, popup);
        combobox_input_set_popup_node(self->ip_input, popup);
        // 加载时自动获取本机 IP 填充下拉列表
        char ips[16][JOY_MAX_IP];
        int ip_count = get_local_ips(ips, 16);
        for (int i = 0; i < ip_count; i++)
                combobox_input_add_item(self->ip_input, ips[i], (int)strlen(ips[i]));

        // 端口输入框
                self->port_input = inputbox_create(ctx->r2d, 0.22917f, 0.275f, 0.20833f, 0.075f, font24);
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", ctx->settings.server_port);
        inputbox_set_text(self->port_input, port_str);

        // 音量滑块
        self->bgm_slider = slider_create(ctx->r2d, 0.22917f, 0.42813f, 0.41667f, 0.04375f, 0.0f, 1.0f, ctx->settings.bgm_volume);
        self->sfx_slider = slider_create(ctx->r2d, 0.22917f, 0.55313f, 0.41667f, 0.04375f, 0.0f, 1.0f, ctx->settings.sfx_volume);
        slider_set_colors(self->bgm_slider, {200,200,200,255}, {50,50,60,255}, {80,80,80,255}, {100,100,100,255});
        slider_set_colors(self->sfx_slider, {200,200,200,255}, {50,50,60,255}, {80,80,80,255}, {100,100,100,255});

        // 全屏复选框
        self->fullscreen_cb = checkbox_create(0.22917f, 0.66250f, 0.04167f, "全屏");
        checkbox_set_r2d(self->fullscreen_cb, ctx->r2d);
        checkbox_set_colors(self->fullscreen_cb, {200,200,200,255}, {50,50,60,255}, {180,180,180,255});
        checkbox_set_state(self->fullscreen_cb,
                ctx->settings.fullscreen ? JOY_CHECKBOX_CHECKED : JOY_CHECKBOX_UNCHECKED);

        // 返回按钮
        self->btn_back = button_create(ctx->r2d, 0.0f, 0.0f, 100.0f / 480.0f, 36.0f / 320.0f);
        setup_button_style(self->btn_back);
        button_set_text_offset(self->btn_back, 0, 3.0f);
        button_set_textx(self->btn_back, font24, "返回", (int)strlen("返回"), { 0, 0, 0, 255 });
        button_set_position(self->btn_back, 190, 270);
}

settings_scene_p settings_scene_create(Context* ctx) {
        settings_scene_p self = new settings_scene_t();
        self->scene = scene_create("Settings");
        self->ctx = ctx;

        scene_set_userdata(self->scene, self);
        scene_set_load_callback(self->scene, on_load);
        scene_set_update_callback(self->scene, on_update);
        scene_set_render_callback(self->scene, on_render);
        scene_set_destroy_callback(self->scene, on_destroy);

        return self;
}

scene_p settings_scene_get_scene(settings_scene_p s) {
        return s ? s->scene : NULL;
}