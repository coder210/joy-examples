#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H
#include "../components/player_state_input.h"
#include "../components/logic_position_component.h"
#include "../components/animator_component.h"
#include <joy/jmath.h>

struct Context;

static const fp_t MOVE_SPEED = fp_from_float(5.0f);

static void movement_system(entt::registry& registry, Context* ctx, float /*dt*/) {
	auto view = registry.view<PlayerStateInput, LogicPositionComponent>();
	fp_t step = fp_mul(MOVE_SPEED, fp_from_float(ctx->FIXED_TIMESTEP));
	for (auto e : view) {
		auto& psi = view.get<PlayerStateInput>(e);
		auto& p   = view.get<LogicPositionComponent>(e);
		if (psi.move_bits == 0) { continue; }
		if (psi.move_bits & 0b0001) p.y = fp_sub(p.y, step);
		if (psi.move_bits & 0b0010) p.y = fp_add(p.y, step);
		if (psi.move_bits & 0b0100) p.x = fp_sub(p.x, step);
		if (psi.move_bits & 0b1000) p.x = fp_add(p.x, step);
		if (registry.all_of<AnimParamsComponent>(e))
			registry.get<AnimParamsComponent>(e).speed = 1.0f;
	}
}

#endif
