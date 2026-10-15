#include "mse/mse.hpp"
#include "pow.hpp"
#include "layer.hpp"

using namespace mse;
using anim::Pow;


namespace
{
    // moves so the bottom middle of the sprite to be at that tile
    ivec2 align(Scene& scene, const entity_id e, const ivec2 to)
    {
        Transform& tr = scene.get<Transform>(e);
        const aabb b  = scene.get<Sprite>(e).bounds(tr);

        const ivec2 shift = to - ivec2{ tr.position.x, b.max.y };
        tr.position += shift;
        return shift;
    }
}


struct POW::StateManager final
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

    static PowState idle(const PowSM::Ctx& c)
    {
        return c.scene.get<Data>(c.entity).shot ? PowState::Free : PowState::Idle;
    }


    static void free_enter(const PowSM::Ctx& c)
    {
        const bool s = sitting(c);
        play_at(c, s ? Pow::SittingFree : Pow::TreeFree, FREE_TICKS, s ? SITTING_FREE_X : TREE_FREE_X);
    }

    static PowState free(const PowSM::Ctx& c)
    {
        return c.scene.get<Sprite>(c.entity).anim_done() ? PowState::FadeOut : PowState::Free;
    }


    // TreeFree still draws the whole tree, so the falling apart one only takes over here
    static void spawn_tree(const PowSM::Ctx& c)
    {
        // read before set<Sprite>, which can move the pool
        const int32_t layer = c.scene.get<Sprite>(c.entity).layer - 1;
        const ivec2   at    = c.scene.get<Data>(c.entity).base + ivec2{ TREE_DESTROY_X, 0 };

        const entity_id tree = c.scene.create_entity();

        c.scene.set<Transform>(tree);
        Sprite& sprite = c.scene.set<Sprite>(tree, Pow::TreeDestroy, should_loop::no);

        sprite.layer = layer;
        sprite.play(Pow::TreeDestroy, TREE_TICKS);
        align(c.scene, tree, at);

        c.scene.get<Data>(c.entity).tree = tree;
    }

    static void fade_out_enter(const PowSM::Ctx& c)
    {
        const bool s = sitting(c);
        if (!s) spawn_tree(c);

        play_at(c, Pow::FadeOut, FADE_TICKS, s ? SITTING_FADE_X : TREE_FADE_X);
    }

    static PowState fade_out(const PowSM::Ctx& c)
    {
        Data& d = c.scene.get<Data>(c.entity);

        // the tree is shorter than the fade, so it goes first
        if (d.tree && c.scene.get<Sprite>(d.tree).anim_done())
        {
            c.scene.destroy_entity(d.tree);
            d.tree = {};
        }

        Sprite& sprite = c.scene.get<Sprite>(c.entity);

        const uint32_t t = c.state.ticks < FADE_TOTAL_TICKS ? c.state.ticks : FADE_TOTAL_TICKS;
        sprite.color.a   = (uint8_t)(255 - 255 * t / FADE_TOTAL_TICKS);

        if (sprite.anim_done())
        {
            if (d.tree) c.scene.destroy_entity(d.tree);
            c.scene.destroy_entity(c.entity);
        }

        return PowState::FadeOut;
    }
};


const POW::PowSM& POW::sm()
{
    using SM = StateManager;

    static constexpr PowSM pow_sm
    {
        PowSM::Behaviour{ SM::idle_enter, SM::idle, nullptr },         // Idle
        PowSM::Behaviour{ SM::free_enter, SM::free, nullptr },         // Free
        PowSM::Behaviour{ SM::fade_out_enter, SM::fade_out, nullptr }, // FadeOut
    };

    return pow_sm;
}

void POW::register_machine() { App::ctx().state_machines.add(sm()); }


entity_id POW::spawn(Scene& scene, const ivec2 pos, const Kind kind)
{
    const entity_id pow = scene.create_entity();

    scene.set<Transform>(pow);
    scene.set<Sprite>(pow, kind == Kind::Sitting ? Pow::SittingIdle : Pow::TreeIdle, should_loop::yes);
    scene.set<Data>(pow, kind, pos);
    align(scene, pow, pos);

    Collider& col = scene.set<Collider>(pow);
    col.layer |= Layer::POW;
    col.target_layer |= Layer::PlayerBullet;

    col.callback = +[](Scene& scene, const entity_id self, entity_id, uint32_t)
    {
        scene.get<Data>(self).shot = true;
        scene.get<Collider>(self).enabled = false;
    };

    sm().start(scene, pow, PowState::Idle);
    return pow;
}