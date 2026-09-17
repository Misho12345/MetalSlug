#pragma once

#include "app.hpp"

/**
 * @brief The main game class, derived from mse::App
 * @note The user has to define Game::init and Game::tick
 * @see app.hpp
 */
class Game final : public mse::App
{
public:
    void init() override;
    void update(float dt) override;
    void fixed_update() override;

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