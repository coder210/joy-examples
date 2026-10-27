#ifndef DRAWING_ENTITY_SYSTEM_H
#define DRAWING_ENTITY_SYSTEM_H
#include "../components/global_state_component.h"
#include "../components/id_component.h"
#include "../components/player_component.h"
#include "../components/logic_rect_component.h"
#include "../components/transform_component.h"
#include "../protocol.h"

struct Context;

static void drawing_entity_system(Context* ctx, entt::entity e,
	IdComponent& id,
	LogicRectComponent& rect,
	TransformComponent& t)
{
	(void)id;
	jrect_t body_rect = { 0 };
	auto& cam = ctx->ecs.ctx().get<GlobalState>().camera_position;
	body_rect.x = meters_to_pixels(t.position.x - cam.x);
	body_rect.y = meters_to_pixels(t.position.y - cam.y);
	body_rect.w = meters_to_pixels(fp_to_float(rect.width));
	body_rect.h = meters_to_pixels(fp_to_float(rect.height));
	if (ctx->ecs.all_of<PlayerComponent>(e)) {
		r2d_set_color(ctx->r2d, 140, 195, 196, 255);
		shape_draw_rectangle(ctx->r2d, "fill", body_rect);
		return;
	}
	else {
		r2d_set_color(ctx->r2d, 140, 195, 196, 255);
		shape_draw_rectangle(ctx->r2d, "fill", body_rect);
	}
}

#endif
