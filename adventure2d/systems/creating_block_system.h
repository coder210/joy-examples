#ifndef CREATING_BLOCK_SYSTEM_H
#define CREATING_BLOCK_SYSTEM_H
#include "../context.h"

static void creating_block_system(entt::registry& registry, Context* ctx, float dt) {
        (void)dt;
        log_info("[creating_block] system");
        entt::entity e = registry.create();
        registry.emplace<LocalOnlyComponent>(e);
        registry.emplace<IdComponent>(e, 10000);
        registry.emplace<LogicRectComponent>(e, fp_from_float(0.5f), fp_from_float(0.5f));
        registry.emplace<LogicPositionComponent>(e, fp_from_float(5), fp_from_float(5));
        registry.emplace<TransformComponent>(e, 5.0f, 5.0f, 0.0f, 1.0f, 1.0f);
}

#endif
