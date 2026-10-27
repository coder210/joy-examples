#ifndef INPUT_CONSUME_INPUT_SYSTEM_H
#define INPUT_CONSUME_INPUT_SYSTEM_H

#include "../components/action_state_component.h"
#include "../components/health_component.h"
#include "../components/player_state_input.h"
#include "../components/player_event_input.h"
#include "../protocol.h"

struct Context;

static void input_consume_input_system(entt::registry& registry, Context* ctx, float /*dt*/) {
	auto& net = registry.ctx().get<NetComponent>();
	if (net.frame_id < 0) return;

	// 攻击计时每逻辑 tick 无条件递减（与 ai_system 敌人部分对齐）。
	// 原实现只在"本帧有输入"时才递减，导致玩家按一次攻击后停手，
	// attack_timer 永久卡住、攻击状态永不退出、挥砍动画无法推进。
	auto pv = registry.view<PlayerComponent, ActionStateComponent>();
	for (auto pe : pv) {
		auto& pact = pv.get<ActionStateComponent>(pe);
		if (pact.attack_timer > 0) pact.attack_timer -= 1;
		if (pact.knockback_timer > 0) pact.knockback_timer -= 1;

		// 兜底：本帧没有该玩家的输入即视为静止（松开方向键 / 未攻击）。
		// 输入收集端在 dir/attack/ranged/ray 全为 0 时不发送输入（见
		// NetworkManager::CollectLocalInput / SendInput），若此处不兜底，
		// act.state 会滞留上一次的 WALK（或 ATTACK），动画永远停在 run。
		// 下方“本帧有输入”的分支会覆盖这里的值。
		pact.state = (pact.attack_timer > 0) ? ActionStateComponent::ATTACK
		                                     : ActionStateComponent::IDLE;

		// 兜底 2：本帧无输入 → 清空移动位。否则 move_bits 会滞留上一次的方向/跳跃位，
		// 导致松开键后仍持续移动，或反复起跳（横版里尤其明显）。
		if (registry.all_of<PlayerStateInput>(pe))
			registry.get<PlayerStateInput>(pe).move_bits = 0;
	}

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

		// 死亡期间不消费输入：尸体不转向、不出招、不留移动位。
		// 帧同步确定性：dead 由两端同一 tick 置位，故此处判定两端一致
		if (registry.all_of<HealthComponent>(player) &&
		    registry.get<HealthComponent>(player).dead) continue;

		PlayerStateInput state_input;
		state_input.move_bits = input.dir;

		// log_info("Raw dir: %d\n", input.dir);

		if (registry.all_of<PlayerStateInput>(player)) {
			auto& state = registry.get<PlayerStateInput>(player);
			state.move_bits = state_input.move_bits;
		}
		else {
			registry.emplace<PlayerStateInput>(player, state_input);
		}

		auto push_action = [&](int type) {
			PlayerEventInput::Action a;
			a.client_seq = input.client_seq;
			a.type       = (uint8_t)type;
			if (registry.all_of<PlayerEventInput>(player)) {
				auto& ev = registry.get<PlayerEventInput>(player);
				ev.pending_actions.push_back(a);
			} else {
				PlayerEventInput ev;
				ev.pending_actions.push_back(a);
				registry.emplace<PlayerEventInput>(player, ev);
			}
		};
		// 攻：投掷炸弹（主武器，替代近战；敌人近战仍由 ai_system 推 ATTACK_MELEE）。
		// 原"射"(枪械子弹)与"线"(射线大招)按钮及功能已移除，不再产生对应事件。
		if (input.attack) push_action(PlayerEventInput::ATTACK_BOMB);

		// 更新动作状态：attack_timer 驱动攻击持续（替代动画状态机复位），
		// 其余由移动位掩码决定 WALK/IDLE 与朝向
		if (registry.all_of<ActionStateComponent>(player)) {
			auto& act = registry.get<ActionStateComponent>(player);


			// 横版：朝向只取左右（bit2=左 bit3=右）；上(bit0)=跳、下(bit1)=预留，不再改变朝向
			const int horiz = input.dir & (INPUT_LEFT | INPUT_RIGHT);
			bool moving  = (horiz != 0);
			int  dir_idx = (horiz & INPUT_LEFT) ? 1 : 2;

			if (input.attack) {
				act.state        = ActionStateComponent::ATTACK;
				act.attack_timer = ActionStateComponent::ATTACK_DURATION;
				act.attack_dir   = moving ? dir_idx : act.dir;   // 锁定攻击起始朝向（站桩沿用当前朝向）
				if (moving) act.dir = dir_idx;
			} else {
				act.state = act.attack_timer > 0 ? ActionStateComponent::ATTACK
				                                 : (moving ? ActionStateComponent::WALK
				                                           : ActionStateComponent::IDLE);
				if (moving) act.dir = dir_idx;
			}
		}
	}
	net.num_inputs = 0;
}

#endif
