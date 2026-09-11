#pragma once
#include "app.hpp"

class Game final : public mse::App
{
public:
    void init() override;
    void tick(float dt) override;

    static Game& instance()
    {
        static Game game;
        return game;
    }

private:
    Game() = default;
};

int main()
{
    Game::instance().setup();
    Game::instance().init();
    Game::instance().run();
    Game::instance().shutdown();
}