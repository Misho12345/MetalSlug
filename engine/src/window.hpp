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

        uint32_t width() const { return size_.x; }
        uint32_t height() const { return size_.y; }
        glm::uvec2 size() const { return size_; }

        aabb bounds() const { return aabb{ {}, size_ }; }

        float aspect_ratio() const { return static_cast<float>(size_.x) / size_.y; }
        GLFWwindow* native_handle() const { return handle_; }

        glm::uvec2 output_offset() const { return output_offset_; }
        glm::uvec2 output_size() const { return output_size_; }

    private:
        Window() = default;

        void update(glm::uvec2 new_size);

        glm::uvec2 size_{};

        glm::uvec2 output_offset_{};
        glm::uvec2 output_size_{};

        GLFWwindow* handle_{ nullptr };

        friend ::mse::PrivCtx;
    };
}
