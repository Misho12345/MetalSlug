#include "mse/pch.hpp"

#include "mse/input/input.hpp"
#include "mse/app.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        Input& instance() { return App::priv_ctx().input; }
    }

    void Input::init()
    {
        glfwSetKeyCallback(
            App::priv_ctx().window.native_handle(),
            +[](GLFWwindow*, int k, int, const int action, int)
            {
                const Key key = static_cast<Key>(k);
                Input& inst = instance();

                if (action == GLFW_PRESS)
                {
                    set(inst.pressed_released_, key, true);
                    set(inst.up_down_, key, true);
                }
                else if (action == GLFW_RELEASE)
                {
                    set(inst.pressed_released_, key, true);
                    set(inst.up_down_, key, false);
                }
            });
    }

    void Input::clear()
    {
        memset(pressed_released_, 0, sizeof(pressed_released_));
    }

    bool Input::up(const Key key) { return !get(instance().up_down_, key); }
    bool Input::down(const Key key) { return get(instance().up_down_, key); }

    bool Input::just_pressed(const Key key) { return down(key) && get(instance().pressed_released_, key); }
    bool Input::just_released(const Key key) { return up(key) && get(instance().pressed_released_, key); }

    void Input::set(uint8_t* flags, const Key key, const bool value)
    {
        const uint32_t k = static_cast<uint32_t>(key);
        const int f = 1 << (k % 8);

        if (value) flags[k / 8] |= f;
        else flags[k / 8] &= ~f;
    }

    bool Input::get(const uint8_t* flags, const Key key)
    {
        const uint32_t k = static_cast<uint32_t>(key);
        return flags[k / 8] & (1 << (k % 8));
    }
}
