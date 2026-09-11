#pragma once
#include "pch.hpp"

namespace mse
{
    struct GameCtx
    {
        // SoundSystem,
        // ParticleSystem,
        // TilemapSystem,
    };

    struct PrivCtx;

    class MSE_API App
    {
    public:
        virtual ~App();

        void setup();
        void run();
        void shutdown();

        virtual void init() = 0;
        virtual void tick(float dt) = 0;

        static App& instance() { return *instance_; }

        static GameCtx& ctx() { return *instance_->ctx_; }
        static PrivCtx& priv_ctx() { return *instance_->priv_ctx_; }

    protected:
        App();

    private:
        static inline App* instance_;

        bool ok_{ true };

        unique_ptr<GameCtx> ctx_;
        unique_ptr<PrivCtx> priv_ctx_;
    };
}