#ifndef INTERPOLATE_POSITION_SYSTEM_H
#define INTERPOLATE_POSITION_SYSTEM_H
#include "../components/logic_position_component.h"
#include "../components/transform_component.h"

// 包含 IdComponent，各端的 IdComponent 在不同位置
// 客户端：components/player_component.h
// 服务端：components/connection_component.h
// 此处统一用前向声明避免冲突，由调用方确保 IdComponent 已包含
#include "../components/id_component.h"

static void interpolate_position_system(entt::registry& registry, Context* /*ctx*/, float dt) {
	auto view = registry.view<IdComponent, LogicPositionComponent, TransformComponent>();
	for (auto e : view) {
		auto& lp = view.get<LogicPositionComponent>(e);
		auto& t  = view.get<TransformComponent>(e);
		float target_x = fp_to_float(lp.x);
		float target_y = fp_to_float(lp.y);
		float alpha = dt * 10.0f;
		if (alpha > 1.0f) alpha = 1.0f;
		t.position.x = ft_lerp(t.position.x, target_x, alpha);
		t.position.y = ft_lerp(t.position.y, target_y, alpha);
	}
}

#endif
