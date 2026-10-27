#include "mse/mse.hpp"
#include "pow.hpp"
#include "layer.hpp"
#include "metal_slug.hpp"
#include "spawn.hpp"

using namespace mse;
using anim::Pow;


namespace ms
{
    struct POWManager::StateManager final
    {
        static bool sitting(const PowSM::Ctx& c) { return c.scene.get<Data>(c.entity).kind == Kind::Sitting; }

        static void play_at(const PowSM::Ctx& c, const Pow animation, const uint8_t ticks, const int32_t x)
        {
            c.scene.get<Sprite>(c.entity).play(animation, ticks);
            const ivec2 shift = align(c.scene, c.entity, c.scene.get<Data>(c.entity).base + ivec2{ x, 0 });

            // the box stays put, it's what stands on the floor
            c.scene.get<Collider>(c.entity).offset -= shift;
        }


        static void idle_enter(const PowSM::Ctx& c)
        {
            c.scene.get<Sprite>(c.entity).play(
                sitting(c) ? Pow::SittingIdle : Pow::TreeIdle, IDLE_TICKS, is_reversed::no, should_loop::yes);
        }

        static PowState on_idle(const PowSM::Ctx& c)
        {
            return c.scene.get<Data>(c.entity).shot ? PowState::Free : PowState::Idle;
        }


        static void free_enter(const PowSM::Ctx& c)
        {
            Data& d  = c.scene.get<Data>(c.entity);
            d.base.y = c.scene.get<Sprite>(c.entity).bounds(c.scene.get<Transform>(c.entity)).max.y;

            // the points come at the debrief, 1000 each
            MetalSlug::user_ctx().player.free_pow();

            const bool s = sitting(c);
            play_at(c, s ? Pow::SittingFree : Pow::TreeFree, FREE_TICKS, s ? SITTING_FREE_X : TREE_FREE_X);
        }

        static PowState on_free(const PowSM::Ctx& c)
        {
            return c.scene.get<Sprite>(c.entity).anim_done() ? PowState::FadeOut : PowState::Free;
        }


        // TreeFree still draws the whole tree, so the falling apart one only takes over here
        static void spawn_tree(const PowSM::Ctx& c)
        {
            // read before set<Sprite>, which can move the pool
            const int32_t layer = c.scene.get<Sprite>(c.entity).layer - 1;
            const ivec2   at    = c.scene.get<Data>(c.entity).base + ivec2{ TREE_DESTROY_X, 0 };

            const entity_id tree = MetalSlug::user_ctx().vfx.create(c.scene, {}, anim::Vfx::TreeDestroy, TREE_TICKS, layer);
            align(c.scene, tree, at);
        }

        static void fade_out_enter(const PowSM::Ctx& c)
        {
            const bool s = sitting(c);
            if (!s) spawn_tree(c);

            play_at(c, Pow::FadeOut, FADE_TICKS, s ? SITTING_FADE_X : TREE_FADE_X);
        }

        static PowState on_fade_out(const PowSM::Ctx& c)
        {
            Sprite& sprite = c.scene.get<Sprite>(c.entity);

            const uint32_t t = c.state.ticks < FADE_TOTAL_TICKS ? c.state.ticks : FADE_TOTAL_TICKS;
            sprite.color.a   = (uint8_t)(255 - 255 * t / FADE_TOTAL_TICKS);

            if (sprite.anim_done()) c.scene.destroy_entity(c.entity);

            return PowState::FadeOut;
        }
    };


    const POWManager::PowSM& POWManager::sm()
    {
        using SM = StateManager;
        using B  = PowSM::Behaviour;

        static constexpr PowSM pow_sm
        {
            B{ SM::idle_enter, SM::on_idle, nullptr },         // Idle
            B{ SM::free_enter, SM::on_free, nullptr },         // Free
            B{ SM::fade_out_enter, SM::on_fade_out, nullptr }, // FadeOut
        };

        return pow_sm;
    }

    void POWManager::register_machine() const { App::ctx().state_machines.add(sm()); }


    entity_id POWManager::spawn(Scene& scene, const ivec2 pos, const Kind kind) const
    {
        const entity_id pow = spawn_standing(scene, pos, kind == Kind::Sitting ? Pow::SittingIdle : Pow::TreeIdle);
        scene.set<Data>(pow, kind, pos);

        Collider& col    = scene.set<Collider>(pow);
        col.layer        |= Layer::POW;
        col.target_layer |= Layer::PlayerProjectile;

        col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
        {
            scene.get<Data>(self).shot        = true;
            scene.get<Collider>(self).enabled = false;
        };

        sm().start(scene, pow, PowState::Idle);
        return pow;
    }
}
