#include <string.h>
#include "gameplay_controls_layer.h"
#include "../asset_manager.h"

// 几何黑白风格按钮：不使用贴图，白色填充 + button_draw 内部的黑色描边
static void setup_button_texture(button_p btn, jr2d_p renderer) {
	(void)renderer;
	jcolor_t normal  = { 255, 255, 255, 255 };
	jcolor_t hover   = { 238, 238, 238, 255 };
	jcolor_t pressed = { 200, 200, 200, 255 };
	button_set_normal_color(btn, normal);
	button_set_hover_color(btn, hover);
	button_set_pressed_color(btn, pressed);
	button_set_image(btn, NULL);
	button_set_pressed_image(btn, NULL);
}

struct gameplay_controls_layer {
	scene_node_p scene_node;
	button_p btn_up, btn_down, btn_left, btn_right, btn_attack;
};


static void send_gameplay_control_event(int event_code)
{
	push_user_event(GAMEPLAY_CONTROL_EVENT, event_code);
}

static void on_event(scene_node_p n, const void* e)
{
	gameplay_controls_layer_p self = (gameplay_controls_layer_p)scene_node_get_userdata(n);
	jevent_callbacks_t cbs = {};
	cbs.on_mousepressed = [](float x, float y, int btn, void* arg) {
		auto* self = (gameplay_controls_layer_p)arg;
		auto hit = [&](button_p b) { return button_is_pressed(b, x, y); };
		if (self->btn_up    && hit(self->btn_up))    send_gameplay_control_event(GAMEPLAY_CONTROL_UP);
		if (self->btn_down  && hit(self->btn_down))  send_gameplay_control_event(GAMEPLAY_CONTROL_DOWN);
		if (self->btn_left  && hit(self->btn_left))  send_gameplay_control_event(GAMEPLAY_CONTROL_LEFT);
		if (self->btn_right && hit(self->btn_right)) send_gameplay_control_event(GAMEPLAY_CONTROL_RIGHT);
		if (self->btn_attack && hit(self->btn_attack)) {} // 攻击按钮按下只记录状态,松手才触发
	};
	cbs.on_mousereleased = [](float x, float y, int btn, void* arg) {
		auto* self = (gameplay_controls_layer_p)arg;
		auto rel = [&](button_p b, int evt) { button_set_pressed(b, false); send_gameplay_control_event(evt); };
		if (self->btn_up    && button_is_hovered(self->btn_up, x, y))
			rel(self->btn_up, GAMEPLAY_CONTROL_RELEASE_UP);
		if (self->btn_down  && button_is_hovered(self->btn_down, x, y))
			rel(self->btn_down, GAMEPLAY_CONTROL_RELEASE_DOWN);
		if (self->btn_left  && button_is_hovered(self->btn_left, x, y))
			rel(self->btn_left, GAMEPLAY_CONTROL_RELEASE_LEFT);
		if (self->btn_right && button_is_hovered(self->btn_right, x, y))
			rel(self->btn_right, GAMEPLAY_CONTROL_RELEASE_RIGHT);
		if (self->btn_attack && button_is_hovered(self->btn_attack, x, y))
			rel(self->btn_attack, GAMEPLAY_CONTROL_ATTACK);
	};
	cbs.on_mousemoved = [](float x, float y, float dx, float dy, void* arg) {
		auto* self = (gameplay_controls_layer_p)arg;
		if (self->btn_up)     button_is_hovered(self->btn_up, x, y);
		if (self->btn_down)   button_is_hovered(self->btn_down, x, y);
		if (self->btn_left)   button_is_hovered(self->btn_left, x, y);
		if (self->btn_right)  button_is_hovered(self->btn_right, x, y);
		if (self->btn_attack) button_is_hovered(self->btn_attack, x, y);
	};
	dispatch_event(cbs, e, self);
}

static void on_update(scene_node_p n, float dt)
{
	gameplay_controls_layer_p self;
	self = (gameplay_controls_layer_p)scene_node_get_userdata(n);
}

static void on_render(scene_node_p n, const void* arg)
{
	jr2d_p renderer = (jr2d_p)arg;
	gameplay_controls_layer_p self;
	self = (gameplay_controls_layer_p)scene_node_get_userdata(n);
}

static void on_destroy(scene_node_p n)
{
	gameplay_controls_layer_p self;
	self = (gameplay_controls_layer_p)scene_node_get_userdata(n);
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

static void load_dpad_panel(gameplay_controls_layer_p self, Context* ctx)
{
	int logical_width, logical_height;
	r2d_get_logical_presentation(ctx->r2d, &logical_width, &logical_height, NULL);
	font_p simhei_font = AssetManager::GetInstance()->GetSimheiFont24();

	scene_node_p root_node = scene_node_create();
	scene_node_set_position(root_node, 0, logical_height - 150);
	scene_node_set_zorder(root_node, 100);
	scene_node_add_child(self->scene_node, root_node);

	self->btn_up = button_create(ctx->r2d, 0.0f, 0.0f, 50.0f / 480.0f, 50.0f / 320.0f);
	setup_button_texture(self->btn_up, ctx->r2d);
	button_set_textx(self->btn_up, simhei_font, "U", (int)strlen("U"), { 40, 40, 40, 255 });
	scene_node_add_child(root_node, make_button_node(self->btn_up, 50, 0));

	self->btn_down = button_create(ctx->r2d, 0.0f, 0.0f, 50.0f / 480.0f, 50.0f / 320.0f);
	setup_button_texture(self->btn_down, ctx->r2d);
	button_set_textx(self->btn_down, simhei_font, "D", (int)strlen("D"), { 40, 40, 40, 255 });
	scene_node_add_child(root_node, make_button_node(self->btn_down, 50, 100));

	self->btn_left = button_create(ctx->r2d, 0.0f, 0.0f, 50.0f / 480.0f, 50.0f / 320.0f);
	setup_button_texture(self->btn_left, ctx->r2d);
	button_set_textx(self->btn_left, simhei_font, "L", (int)strlen("L"), { 40, 40, 40, 255 });
	scene_node_add_child(root_node, make_button_node(self->btn_left, 0, 50));

	self->btn_right = button_create(ctx->r2d, 0.0f, 0.0f, 50.0f / 480.0f, 50.0f / 320.0f);
	setup_button_texture(self->btn_right, ctx->r2d);
	button_set_textx(self->btn_right, simhei_font, "R", (int)strlen("R"), { 40, 40, 40, 255 });
	scene_node_add_child(root_node, make_button_node(self->btn_right, 100, 50));
}


static void load_action_panel(gameplay_controls_layer_p self, Context* ctx)
{
	font_p simhei_font = AssetManager::GetInstance()->GetSimheiFont24();
	int logical_width, logical_height;
	r2d_get_logical_presentation(ctx->r2d, &logical_width, &logical_height, NULL);

	scene_node_p root_node = scene_node_create();
	scene_node_set_position(root_node, logical_width - 100, logical_height - 100);
	scene_node_set_zorder(root_node, 100);
	scene_node_add_child(self->scene_node, root_node);

	self->btn_attack = button_create(ctx->r2d, 0.0f, 0.0f, 50.0f / 480.0f, 50.0f / 320.0f);
	setup_button_texture(self->btn_attack, ctx->r2d);
	button_set_textx(self->btn_attack, simhei_font, "A", (int)strlen("A"), { 40, 40, 40, 255 });
	scene_node_add_child(root_node, make_button_node(self->btn_attack, 0, 0));
}

gameplay_controls_layer_p create_gameplay_controls_layer(Context* ctx)
{
	gameplay_controls_layer_p self;
	self = (gameplay_controls_layer_p)malloc(sizeof(gameplay_controls_layer_t));
	memset(self, 0, sizeof(*self));
	self->scene_node = scene_node_create();
	scene_node_set_userdata(self->scene_node, self);
	scene_node_set_event_callback(self->scene_node, on_event);
	scene_node_set_update_callback(self->scene_node, on_update);
	scene_node_set_render_callback(self->scene_node, on_render);
	scene_node_set_destroy_callback(self->scene_node, on_destroy);
	scene_node_set_position(self->scene_node, 0, 0);
	scene_node_set_zorder(self->scene_node, 1);
	scene_node_set_scale(self->scene_node, 1, 1);

	load_dpad_panel(self, ctx);
	load_action_panel(self, ctx);

	return self;
}

scene_node_p gameplay_controls_layer_get_node(gameplay_controls_layer_p gameplay_controls_layer)
{
	return gameplay_controls_layer->scene_node;
}
