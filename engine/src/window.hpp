#pragma once
#include "mse/pch.hpp"

namespace mse
{
    struct WindowDesc
    {
        uint32_t    width{};
        uint32_t    height{};
        const char* title{};
    };

    class Window final
    {
    public:
        Window() = default;
        ~Window();

        Window(const Window&)            = delete;
        Window(Window&&)                 = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&)      = delete;

        bool init(const WindowDesc& desc);

        bool should_close() const;
        void poll_events() const;
        void swap_buffers();

        uint32_t width() const { return desc_.width; }
        uint32_t height() const { return desc_.height; }

        float aspect_ratio() const
        {
            return static_cast<float>(desc_.width) /
                    static_cast<float>(desc_.height);
        }

        GLFWwindow* native_handle() const { return handle_; }

    private:
        WindowDesc  desc_{};
        GLFWwindow* handle_{ nullptr };
    };
}
