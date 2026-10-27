#ifndef INPUT_CONSUME_INPUT_SYSTEM_H
#define INPUT_CONSUME_INPUT_SYSTEM_H

struct Context;

static void input_consume_input_system(entt::registry& registry, Context* /*ctx*/, float /*dt*/) {
	auto& net = registry.ctx().get<NetComponent>();
	if (net.frame_id < 0) return;

	auto view = registry.view<PlayerComponent>();
	for (int i = 0; i < net.num_inputs; i++) {
		auto& input = net.inputs[i];

		entt::entity player = entt::null;
		for (auto e : view) {
			if (registry.get<PlayerComponent>(e).conv == input.conv) {
				player = e;
				break;
			}
		}
		if (player == entt::null) continue;

		PlayerStateInput state_input;
		state_input.move_bits = input.dir;

		log_info("Raw dir: %d\n", input.dir);

		if (registry.all_of<PlayerStateInput>(player)) {
			auto& state = registry.get<PlayerStateInput>(player);
			state.move_bits = state_input.move_bits;
		}
		else {
			registry.emplace<PlayerStateInput>(player, state_input);
		}

		if (input.attack) {
			if (registry.all_of<PlayerEventInput>(player)) {
				auto& events = registry.get<PlayerEventInput>(player);
				events.pending_actions.push_back({ input.client_seq, (uint8_t)input.attack });
			}
			else {
				PlayerEventInput events;
				events.pending_actions.push_back({ input.client_seq, (uint8_t)input.attack });
				registry.emplace<PlayerEventInput>(player, events);
			}
		}

		// 更新动画参数：将原始方向位掩码翻译为 anim_params_t.dir / speed / attack
		if (registry.all_of<anim_params_t, ActionStateComponent>(player)) {
			auto& params = registry.get<anim_params_t>(player);
			auto& act    = registry.get<ActionStateComponent>(player);

			int  dir_idx = input_to_dir(input.dir);
			bool moving  = has_dir(input.dir);

			if (input.attack) {
				act.state     = ActionStateComponent::ATTACK;
				params.attack = true;
				params.speed  = 1.0f;
				if (moving) params.dir = dir_idx;
			} else {
				act.state  = moving ? ActionStateComponent::WALK
				                    : ActionStateComponent::IDLE;
				params.speed = moving ? 1.0f : 0.0f;
				if (moving) params.dir = dir_idx;
				params.attack = false;
			}
		}
	}
	net.num_inputs = 0;
}

#endif
