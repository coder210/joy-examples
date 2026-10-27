#ifndef INPUT_CONSUME_JOIN_SYSTEM_H
#define INPUT_CONSUME_JOIN_SYSTEM_H

#include "../components/global_state_component.h"
#include "../components/animator_def.h"

struct Context;

static void input_consume_join_system(entt::registry& registry, Context* ctx, float /*dt*/) {
	auto& net = registry.ctx().get<NetComponent>();
	if (net.frame_id < 0) return;

	auto& gs = registry.ctx().get<GlobalState>();

	for (int i = 0; i < net.num_joins; i++) {
		log_info("creating player %d", net.num_joins);
		auto& join = net.joins[i];
		entt::entity player = registry.create();
		registry.emplace<IdComponent>(player, get_entity_id(ctx));
		registry.emplace<PlayerComponent>(player, join.conv);
		registry.emplace<LogicPositionComponent>(player, join.position_x, join.position_y);
		registry.emplace<LogicRectComponent>(player, fp_from_float(1), fp_from_float(1));
		registry.emplace<TransformComponent>(player, fp_to_float(join.position_x), fp_to_float(join.position_y), 0.0f, 1.0f, 1.0f);

		// 挂载动画组件（如果 warrior 动画定义已注册）
		if (registry.ctx().contains<janim_warrior_def_t>()) {
			registry.emplace<AnimatorComponent>(player);
			registry.emplace<AnimParamsComponent>(player);
			registry.emplace<ActionStateComponent>(player);
		}

		// 首次加入的玩家作为摄像机跟随目标
		if (gs.camera_follow_conv < 0) {
			gs.camera_follow_conv = join.conv;
		}
	}
	net.num_joins = 0;
}

#endif
