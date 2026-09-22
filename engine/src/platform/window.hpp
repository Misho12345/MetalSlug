#pragma once
#include "mse/pch.hpp"
#include "mse/collision/aabb.hpp"

namespace mse
{
    struct WindowDesc final
    {
        int32_t     width{};
        int32_t     height{};
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
        [[nodiscard]]
        bool init(const WindowDesc& desc);

        [[nodiscard]]
        bool should_close() const;
        void poll_events() const;
        void swap_buffers() const;

        [[nodiscard]] int32_t width() const { return size_.x; }
        [[nodiscard]] int32_t height() const { return size_.y; }
        [[nodiscard]] glm::uvec2 size() const { return size_; }

        [[nodiscard]]
        aabb bounds() const { return aabb{ {}, size_ }; }

        [[nodiscard]] float aspect_ratio() const { return static_cast<float>(size_.x) / size_.y; }
        [[nodiscard]] GLFWwindow* native_handle() const { return handle_; }

        [[nodiscard]] glm::ivec2 output_offset() const { return output_offset_; }
        [[nodiscard]] glm::ivec2 output_size() const { return output_size_; }

    private:
        Window() = default;

        void update(glm::ivec2 new_size);

        glm::ivec2 size_{};

        glm::ivec2 output_offset_{};
        glm::ivec2 output_size_{};

        GLFWwindow* handle_{ nullptr };

        friend ::mse::PrivCtx;
    };
}
