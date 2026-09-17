#pragma once
#include "mse/pch.hpp"
#include "mse/aabb.hpp"

namespace mse
{
    struct WindowDesc final
    {
        uint32_t    width{};
        uint32_t    height{};
        const char* title{};
    };

    struct PrivCtx;

    /**
     * @brief Represents a window
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class Window final
    {
    public:
        ~Window();

        Window(const Window&)            = delete;
        Window(Window&&)                 = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&)      = delete;

        /// @brief Initialize GLFW and create the window
        bool init(const WindowDesc& desc);

        bool should_close() const;
        void poll_events() const;
        void swap_buffers();

        uint32_t width() const { return desc_.width; }
        uint32_t height() const { return desc_.height; }

        aabb bounds() const { return aabb{ { 0, 0 }, { width(), height() } }; }

        float aspect_ratio() const
        {
            return static_cast<float>(desc_.width) /
                    static_cast<float>(desc_.height);
        }

        GLFWwindow* native_handle() const { return handle_; }

    private:
        Window() = default;

        WindowDesc  desc_{};
        GLFWwindow* handle_{ nullptr };

        friend ::mse::PrivCtx;
    };
}
