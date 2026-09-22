#include "mse/pch.hpp"
#include "animation_system.hpp"

#include "priv_ctx.hpp"
#include "mse/ecs/components.hpp"
#include "mse/app.hpp"


namespace mse
{
    void AnimationSystem::update(Scene& scene, const float dt) const
    {
        const SpriteDataRegistry& reg = App::priv_ctx().sprite_data_registry;

        for (SpriteRenderer& sprite_renderer : scene.pool<SpriteRenderer>().components())
        {
            sprite_renderer.update(dt, reg.anim_data(sprite_renderer.info()).frame_count);
        }
    }
}