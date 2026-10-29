#include "mse/pch.hpp"
#include "mse/particles/particle_system.hpp"

#include "priv_ctx.hpp"
#include "mse/app.hpp"

namespace mse
{
    namespace
    {
        template <typename T>
        void swap_pop(vector<T>& v, const size_t i)
        {
            v[i] = std::move(v.back());
            v.pop_back();
        }
    }


    void ParticleSystem::emit(const anim::Info info, const ParticleDesc& desc)
    {
        assert((desc.life > 0 || desc.frame_dur > 0) && "ParticleSystem::emit: nothing would ever end this particle");

        size_t idx = layers_.find([&](const Layer& l) { return l.layer == desc.layer; });

        if (idx == vector<>::npos)
        {
            layers_.emplace_back(Layer{ .layer = desc.layer });
            idx = layers_.size() - 1;
        }

        Layer& l = layers_[idx];

        l.instances.emplace_back(InstanceData{
            .parallax_factor = desc.parallax_factor,
            .position        = ivec2(glm::floor(desc.position)),
            .scale           = ivec2(desc.scale),
            .rotation        = 0.0f,
            .anim_idx        = anim::global_anim_id(info),
            .frame_idx       = 0,
            .flip_x          = desc.flip_x,
            .color           = desc.color.data_u32
        });

        l.motion.emplace_back(Motion{
            .position    = desc.position,
            .velocity    = desc.velocity,
            .gravity     = desc.gravity,
            .life        = desc.life,
            .frame_dur   = desc.frame_dur,
            .time        = 0,
            .frame_count = anim::frame_count(info)
        });
    }

    void ParticleSystem::update()
    {
        constexpr float dt = Target::FRAME_TIME;

        for (Layer& l : layers_)
        {
            // backwards, so the particle swapped into a dead one's place has already been updated
            for (size_t i = l.motion.size(); i-- > 0;)
            {
                Motion&       m    = l.motion[i];
                InstanceData& inst = l.instances[i];

                bool dead = m.life > 0 && --m.life == 0;

                if (!dead && m.frame_dur > 0 && ++m.time >= m.frame_dur)
                {
                    m.time = 0;

                    if (++inst.frame_idx >= m.frame_count)
                    {
                        inst.frame_idx = 0;
                        dead = m.life == 0; // still 0 here only if it was emitted without a life
                    }
                }

                if (dead)
                {
                    swap_pop(l.instances, i);
                    swap_pop(l.motion, i);
                    continue;
                }

                m.velocity.y += m.gravity * dt;
                m.position   += m.velocity * dt;

                inst.position = ivec2(glm::floor(m.position));
            }
        }
    }

    void ParticleSystem::clear()
    {
        for (Layer& l : layers_)
        {
            l.instances.clear();
            l.motion.clear();
        }
    }

    uvec2 ParticleSystem::frame_size(const anim::Info info)
    {
        return App::priv_ctx().sprite_data_registry.anim_data(info).frame_size;
    }
}
