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
        void swap_buffers() const;

        uint32_t width() const { return width_; }
        uint32_t height() const { return height_; }

        aabb bounds() const { return aabb{ {}, { width_, height_ } }; }

        float aspect_ratio() const { return static_cast<float>(width_) / height_; }
        GLFWwindow* native_handle() const { return handle_; }

    private:
        Window() = default;

        uint32_t width_{};
        uint32_t height_{};
        GLFWwindow* handle_{ nullptr };

        friend ::mse::PrivCtx;
    };
}
