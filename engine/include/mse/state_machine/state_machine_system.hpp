#pragma once
#include "mse/pch.hpp"
#include "mse/api.hpp"

#include "state_machine.hpp"

namespace mse
{
    class MSE_API StateMachineSystem final
    {
    public:
        template <typename E>
        void add(const StateMachine<E>& sm)
        {
            runners_.emplace_back(&sm, +[](Scene& scene, const void* m)
            {
                const StateMachine<E>& sm = *(const StateMachine<E>*)m;
                const vector<entity_id>& entities = scene.pool<State<E>>().owners();

                // reversed in case update() results in switch_to removing the component, which would swap and pop back
                for (size_t i = entities.size(); i-- > 0;) sm.update(scene, entities[i]);
            });
        }

        void update(Scene& scene)
        {
            for (const Runner& r : runners_) r.run(scene);
        }

    private:
        struct Runner final
        {
            const void* machine_;
            void (*run_)(Scene&, const void*);

            void run(Scene& scene) const { run_(scene, machine_); }
        };

        vector<Runner> runners_;
    };
}