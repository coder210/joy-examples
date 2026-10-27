#ifndef ANIMATOR_UPDATE_SYSTEM_H
#define ANIMATOR_UPDATE_SYSTEM_H
#include <joy/janim.h>
#include "../components/player_state_input.h"
#include "../components/action_state_component.h"
#include "../components/enemy_component.h"
#include "../components/animator_def.h"

struct Context;

static void animator_update_system(entt::registry& registry, Context* ctx, float dt) {
	(void)ctx;

	auto view = registry.view<anim_t, anim_params_t, ActionStateComponent>();
	for (auto entity : view) {
		auto& a      = view.get<anim_t>(entity);
		auto& params = view.get<anim_params_t>(entity);
		auto& act    = view.get<ActionStateComponent>(entity);

		// 根据实体类型选择动画定义
		anim_def_t* def = nullptr;
		if (registry.all_of<EnemyComponent>(entity)) {
			if (registry.ctx().contains<janim_goblin_def_t>())
				def = &registry.ctx().get<janim_goblin_def_t>().layout;
		} else {
			if (registry.ctx().contains<janim_warrior_def_t>())
				def = &registry.ctx().get<janim_warrior_def_t>().layout;
		}
		if (!def) continue;

		// 这帧没有收到输入 → 空闲
		if (!registry.all_of<PlayerStateInput>(entity))
			params.speed = 0.0f;

		// 方向同步
		a.flip_h = (params.dir == 1);

		// 核心状态机更新（无 entt 依赖）
		anim_update(&a, def, &params, dt);

		// 攻击是一次性触发，消费后立即复位
		params.attack = false;

		// === ECS 专有逻辑：同步 act.state ===
		if (act.state == ActionStateComponent::ATTACK) {
			if (strncmp(def->states[a.current_state].name, "attack", 6) != 0) {
				act.state = ActionStateComponent::IDLE;
			}
		}
	}
}

#endif
