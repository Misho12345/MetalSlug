#pragma once
#include "mse/mse.hpp"

namespace ms
{
    enum class VfxStage { Alive, COUNT };

    // for fire and forget type sprites that play one animation and die
    // like most vfx are
    class Vfx final
    {
        static constexpr size_t POOL_SIZE{ 64 };
        using SM = mse::StateMachine<VfxStage>;

    public:
        Vfx() = default;

        void init(mse::Scene& scene)
        {
            using namespace mse;

            for (entity_id& e : vfx_)
            {
                e = scene.create_entity();
                scene.set<Transform>(e);
                scene.set<Sprite>(e, anim::Vfx{}).hidden = true;
            }
        }

        Vfx(const Vfx&) = delete;
        Vfx& operator=(const Vfx&) = delete;
        Vfx(Vfx&&) = delete;
        Vfx& operator=(Vfx&&) = delete;

        template <mse::anim::sprite_enum E>
        mse::entity_id create(mse::Scene& scene, const mse::ivec2 pos, E anim, uint8_t duration, const int32_t layer = 0)
        {
            assert(duration > 0 && "duration must be at least 1 tick");

            using namespace mse;
            entity_id e;

            for (uint32_t i = 0; i < POOL_SIZE; ++i)
            {
                const uint32_t slot = (next_ + i) % POOL_SIZE;

                if (scene.get<Sprite>(vfx_[slot]).hidden)
                {
                    next_ = (slot + 1) % POOL_SIZE;
                    e = vfx_[slot];
                    goto start;
                }
            }

            e = vfx_[next_];
            next_ = (next_ + 1) % POOL_SIZE;

            start:
            scene.get<Transform>(e).position = pos;

            Sprite& s = scene.get<Sprite>(e);
            s.play(anim, duration, is_reversed::no, should_loop::no, restart_if_same::yes);
            s.layer = layer;
            s.hidden = false;
            s.flip_x = false;

            sm.start(scene, e, VfxStage::Alive);

            return e;
        }

        static void register_machine() { mse::App::ctx().state_machines.add(sm); }

    private:
        static constexpr SM sm
        {
            SM::Behaviour{
                nullptr,
                +[](const SM::Ctx& ctx) -> VfxStage
                {
                    return ctx.scene.get<mse::Sprite>(ctx.entity).anim_done() ? VfxStage::COUNT : VfxStage::Alive;
                },
                +[](const SM::Ctx& ctx) { ctx.scene.get<mse::Sprite>(ctx.entity).hidden = true; }
            }
        };

        mse::entity_id vfx_[POOL_SIZE];
        uint32_t next_ = 0;
    };
}