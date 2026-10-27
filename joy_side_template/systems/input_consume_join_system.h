#ifndef INPUT_CONSUME_JOIN_SYSTEM_H
#define INPUT_CONSUME_JOIN_SYSTEM_H

#include "../components/global_state_component.h"
#include "../components/action_state_component.h"
#include "../components/health_component.h"
#include "../components/spawn_point_component.h"
#include "../components/logic_velocity_component.h"   // 横版重力/跳跃垂直速度
#include "../components/player_state_input.h"          // 移动位（需随玩家创建）
#include "../systems/character_catalog.h"

struct Context;

static void input_consume_join_system(entt::registry& registry, Context* ctx, float /*dt*/) {
	auto& net = registry.ctx().get<NetComponent>();
	if (net.frame_id < 0) return;

	auto& gs = registry.ctx().get<GlobalState>();

	for (int i = 0; i < net.num_joins; i++) {
		log_info("creating player %d", net.num_joins);
		auto& join = net.joins[i];

		// 该 conv 玩家可能已由加载快照创建（client 端）：不重复创建，仅刷新出生点
		bool exists = false;
		auto pview = registry.view<PlayerComponent>();
		for (auto pe : pview) {
			if (registry.get<PlayerComponent>(pe).conv == join.conv) {
				// 已有实体：仅刷新出生点（重连/重新加入场景）
				if (registry.all_of<HealthComponent>(pe)) {
					auto& sp = registry.get<SpawnPointComponent>(pe);
					sp.x = join.position_x;
					sp.y = join.position_y;
				}
				exists = true;
				break;
			}
		}
		if (exists) continue;

		entt::entity player = registry.create();
		registry.emplace<IdComponent>(player, get_entity_id(ctx));
		registry.emplace<PlayerComponent>(player, join.conv, join.char_id);

		// join.position 是出生格中心；碰撞盒 0.8×0.8 以出生点居中，
		// 碰撞盒高度与玩家可视高度(1.0m)对齐，避免下半身穿入地面（穿模）。
		// 玩家精灵以中心绘制、中心 = 出生点；盒高取 1.0m 后盒底恰落在脚底。
		// （玩家绘制归一化到 50px = 1m，见 draw_player_pass 的 scale = 50/src_rect.h）
		// 碰撞盒高度随实体渲染放大同步（宽度维持 0.8，保证仍能通过 1 格宽通道）
		// 逻辑层统一 fp_ 运算：半宽/半高用 fp_mul 取半，不写死 0.4、不做浮点相减/相乘。
		const fp_t kPlayerBoxW = fp_from_float(0.8f);
		const fp_t kPlayerBoxH = fp_from_float(1.0f * kEntityRenderScale);
		registry.emplace<LogicPositionComponent>(player,
			fp_sub(join.position_x, fp_mul(kPlayerBoxW, fp_from_float(0.5f))),
			fp_sub(join.position_y, fp_mul(kPlayerBoxH, fp_from_float(0.5f))));
		registry.emplace<LogicRectComponent>(player, kPlayerBoxW, kPlayerBoxH);
		registry.emplace<TransformComponent>(player, fp_to_float(join.position_x), fp_to_float(join.position_y), 0.0f, 1.0f, 1.0f);

		// 挂载动作状态组件（几何渲染的朝向 / 攻击表现依赖它）
		registry.emplace<ActionStateComponent>(player);

		// 挂载血量组件（远程敌人子弹命中扣血）；初次加入也带重生无敌，防出生点被围殴
		registry.emplace<HealthComponent>(player, PLAYER_MAX_HP, PLAYER_MAX_HP,
			false, 0, SPAWN_INVULN_TICKS);
		// 出生点 = join 位置（重生传送目标）
		registry.emplace<SpawnPointComponent>(player, join.position_x, join.position_y);

		// 横版重力/跳跃：垂直速度状态（两端由相同输入确定性推进 → 帧同步一致）
		registry.emplace<LogicVelocityComponent>(player, fp_from_float(0), fp_from_float(0));
		// 输入状态组件：必须随玩家一同创建。否则在"第一帧输入到来之前"该玩家不在
		// movement_system 的 view(PlayerStateInput+LogicPosition) 里 → 不受重力，
		// 会一直悬在出生点（半空）。
		registry.emplace<PlayerStateInput>(player);

		// 首次加入的玩家作为摄像机跟随目标
		if (gs.camera_follow_conv < 0) {
			gs.camera_follow_conv = join.conv;
		}
	}
	net.num_joins = 0;
}

#endif
