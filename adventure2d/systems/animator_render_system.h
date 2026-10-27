#ifndef ANIMATOR_RENDER_SYSTEM_H
#define ANIMATOR_RENDER_SYSTEM_H
#include "../components/global_state_component.h"
#include "../components/enemy_component.h"
#include "../components/animator_def.h"
#include "../protocol.h"
#include <joy/janim.h>
#include <map>
#include <string>

struct Context;

static image_p get_or_load_image(jr2d_p r2d, const char* path) {
	static std::map<std::string, image_p> cache;
	auto it = cache.find(path);
	if (it != cache.end()) return it->second;
	image_p img = image_create(r2d, path);
	if (img) cache[path] = img;
	return img;
}

static void animator_render_system(Context* ctx, entt::registry& registry,
	TransformComponent& t, LogicRectComponent& r,
	anim_t& anim, ActionStateComponent& act,
	entt::entity e)
{
	(void)registry;
	(void)act;

	// 根据实体类型选择动画定义
	anim_def_t* def = nullptr;
	if (registry.all_of<EnemyComponent>(e)) {
		if (ctx->ecs.ctx().contains<janim_goblin_def_t>())
			def = &ctx->ecs.ctx().get<janim_goblin_def_t>().layout;
	} else {
		if (ctx->ecs.ctx().contains<janim_warrior_def_t>())
			def = &ctx->ecs.ctx().get<janim_warrior_def_t>().layout;
	}
	if (!def || def->num_states <= 0) return;

	anim_clip_t clip;
	if (!anim_get_current_clip(&anim, def, &clip)) return;

	// === 摄像头偏移 + PPM 换算（与其他渲染系统一致）===
	auto& cam = ctx->ecs.ctx().get<GlobalState>().camera_position;
	float scr_x = meters_to_pixels(t.position.x - cam.x);
	float scr_y = meters_to_pixels(t.position.y - cam.y);

	image_p img = get_or_load_image(ctx->r2d, clip.path);
	if (!img) {
		jrect_t dr = {
			scr_x - clip.src_rect.w / 2,
			scr_y - clip.src_rect.h / 2,
			(float)clip.src_rect.w, (float)clip.src_rect.h
		};
		r2d_set_color(ctx->r2d, 0, 255, 0, 180);
		shape_draw_rectangle(ctx->r2d, "fill", dr);
		return;
	}

	vec2_t pos = { scr_x, scr_y };
	vec2_t scale = { anim.flip_h ? -1.0f : 1.0f, 1.0f };
	vec2_t origin = { clip.src_rect.w / 2, clip.src_rect.h / 2 };

	image_draw_ex(img, &clip.src_rect, pos, 0.0f, scale, origin);

	// 绘制碰撞框（调试）
	float body_w = meters_to_pixels(fp_to_float(r.width));
	float body_h = meters_to_pixels(fp_to_float(r.height));
	jrect_t debug_rect = {
		scr_x - body_w / 2, scr_y - body_h / 2,
		body_w, body_h
	};
	r2d_set_color(ctx->r2d, 255, 0, 0, 100);
	shape_draw_rectangle(ctx->r2d, "line", debug_rect);
}

#endif // ANIMATOR_RENDER_SYSTEM_H
