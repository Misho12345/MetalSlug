#pragma once
#include "mse/pch.hpp"
#include "mse/ecs/scene.hpp"

namespace mse
{
    template <typename E = void>
    struct State
    {
        E current{};
        E previous{};
        uint16_t ticks{};
    };

    template <typename E> requires std::is_enum_v<E>
    class StateMachine
    {
        static constexpr size_t N = (size_t)E::Count;

    public:
        struct Ctx
        {
            Scene&    scene;
            entity_id entity;
            State<E>& state;
        };

        struct Behaviour
        {
            void (*enter)(const Ctx&) = nullptr;
            E    (*stay)(const Ctx&);
            void (*exit)(const Ctx&)  = nullptr;
        };

        template <typename... Bs> requires (sizeof...(Bs) == N && (std::same_as<std::remove_cvref_t<Bs>, Behaviour> && ...))
        StateMachine(Bs&&... bs) : bs_{ std::forward<Bs>(bs)... } {}

        void start(Scene& scene, const entity_id entity, E initial) const
        {
            State<E>& s = scene.set<State<E>>(entity, initial, initial, 0);

            Ctx ctx{ scene, entity, s };

            if (const Behaviour& b = bs_[(size_t)initial];
                b.enter)
                b.enter(ctx);
        }

        void update(Scene& scene, const entity_id entity) const
        {
            State<E>& s = scene.get<State<E>>(entity);
            ++s.ticks;

            if (const Behaviour& b = bs_[(size_t)s.current];
                b.stay)
            {
                Ctx ctx{ scene, entity, s };

                const E next = b.stay(ctx);
                if (next != s.current) switch_to(ctx, next);
            }
        }

        void force(Scene& scene, const entity_id entity, E next) const
        {
            State<E>& s = scene.get<State<E>>(entity);
            Ctx c{ scene, entity, s };
            if (next != s.current) switch_to(c, next);
        }

    private:
        void switch_to(Ctx& c, E next) const
        {
            if (const Behaviour& from = bs_[(size_t)c.state.current];
                from.exit)
                from.exit(c);

            c.state.previous = c.state.current;
            c.state.current  = next;
            c.state.ticks    = 0;

            if (const Behaviour& to = bs_[(size_t)next];
                to.enter)
                to.enter(c);
        }

        Behaviour bs_[N];
    };
}