#pragma once
#include "mse/pch.hpp"
#include "input/input.hpp"
#include "ecs/scene.hpp"

namespace mse
{
    /**
     * @brief Public engine systems accessible to game code.
     * @details Acts as a service locator owned by App, avoiding the need for static singletons.
     * High-level systems will be placed here that game code needs to access directly.
     */
    struct GameCtx
    {
        Scene scene;
    };

    /**
     * @brief Internal engine context
     * @see priv_ctx.hpp
     */
    struct PrivCtx;


    /**
     * @brief Base class for the main application of the engine
     */
    class MSE_API App
    {
    public:
        virtual ~App();

        void setup();
        void run();
        void shutdown();

        virtual void init() = 0;
        virtual void update(float dt) = 0;
        virtual void fixed_update() = 0;

        [[nodiscard]]
        static App& instance() { return *instance_; }

        /// @brief Access to public engine systems
        [[nodiscard]]
        static GameCtx& ctx() { return *instance_->ctx_; }

        /**
        * @brief Access to internal engine systems
        * @details Include priv_ctx.hpp for the definition of PrivCtx
        * @note These are not intended to be accessed directly by game code.
        * @see priv_ctx.hpp
        */
        [[nodiscard]]
        static PrivCtx& priv_ctx() { return *instance_->priv_ctx_; }

    protected:
        App();

    private:
        // the extending class will ensure that there is only one instance
        // maybe not the best design, but good enough
        static inline App* instance_;

        bool ok_{ true };

        unique_ptr<GameCtx> ctx_;
        unique_ptr<PrivCtx> priv_ctx_;
    };
}
