#include "mse/pch.hpp"

#include "mse/input.hpp"
#include "mse/app.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    void Input::init()
    {
        glfwSetKeyCallback(
            App::priv_ctx().window.native_handle(),
            +[](GLFWwindow*, int k, int, const int action, int)
            {
                const Key key = static_cast<Key>(k);

                if (action == GLFW_PRESS)
                {
                    instance().set(instance().pressed_released_, key, true);
                    instance().set(instance().up_down_, key, true);
                }
                else if (action == GLFW_RELEASE)
                {
                    instance().set(instance().pressed_released_, key, true);
                    instance().set(instance().up_down_, key, false);
                }
            });
    }

    void Input::update()
    {
        memset(instance().pressed_released_, 0, sizeof(instance().pressed_released_));
    }

    bool Input::up(const Key key) { return !get(instance().up_down_, key); }
    bool Input::down(const Key key) { return get(instance().up_down_, key); }

    bool Input::just_pressed(const Key key) { return down(key) && get(instance().pressed_released_, key); }
    bool Input::just_released(const Key key) { return up(key) && get(instance().pressed_released_, key); }

    void Input::set(char* flags, const Key key, const bool value)
    {
        const int k = static_cast<int>(key);
        if (value) flags[k / 8] |= (1 << (k % 8));
        else flags[k / 8]       &= ~(1 << (k % 8));
    }

    bool Input::get(const char* flags, const Key key)
    {
        const int k = static_cast<int>(key);
        return flags[k / 8] & (1 << (k % 8));
    }
}
