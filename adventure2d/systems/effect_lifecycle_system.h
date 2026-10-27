#ifndef EFFECT_LIFECYCLE_SYSTEM_H
#define EFFECT_LIFECYCLE_SYSTEM_H
#include "../components/attack_ray_effect_component.h"

struct Context;

static void effect_lifecycle_system(Context* ctx, entt::registry& registry, entt::entity e, AttackRayEffectComponent& effect, float dt)
{
	(void)ctx;
	effect.lifetime -= dt;
	if (effect.lifetime <= 0.0f) {
		registry.destroy(e);
	}
}


#endif
