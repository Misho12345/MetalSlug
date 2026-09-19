#include "mse/pch.hpp"
#include "window.hpp"

namespace mse
{
    Window::~Window()
    {
        glfwDestroyWindow(handle_);
        glfwTerminate();
    }

    bool Window::init(const WindowDesc& desc)
    {
        glfwInit();

        // OpenGL 4.6 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();

        if (!primary_monitor)
        {
            printf("Failed to get GLFW primary monitor\n");
            glfwTerminate();
            return false;
        }

        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);

        if (!mode)
        {
            printf("Failed to get GLFW video mode\n");
            glfwTerminate();
            return false;
        }

        handle_ = glfwCreateWindow(
            static_cast<int>(desc.width),
            static_cast<int>(desc.height),
            desc.title,
            nullptr,
            nullptr);

        if (!handle_)
        {
            printf("Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }

        glfwSetWindowPos(
            handle_,
            static_cast<int>((mode->width - desc.width) / 2),
            static_cast<int>((mode->height - desc.height) / 2));

        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        glfwMakeContextCurrent(handle_);

        glfwSetWindowUserPointer(handle_, this);
        glfwSetWindowSizeCallback(handle_, [](GLFWwindow* window, const int width, const int height)
        {
            if (Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window)))
                win->update({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
        });

        update({ desc.width, desc.height });

        return true;
    }

    bool Window::should_close() const { return glfwWindowShouldClose(handle_); }
    void Window::poll_events() const { glfwPollEvents(); }
    void Window::swap_buffers() const { glfwSwapBuffers(handle_); }

    void Window::update(const glm::uvec2 new_size)
    {
        size_ = new_size;

        float ar = aspect_ratio();

        if (ar > Target::DAR)
        {
            output_size_.x = size_.y * Target::DAR;
            output_size_.y = size_.y;
            output_offset_.x = (size_.x - output_size_.x) * 0.5f;
            output_offset_.y = 0.0f;
        }
        else
        {
            output_size_.x = size_.x;
            output_size_.y = size_.x / Target::DAR;
            output_offset_.x = 0.0f;
            output_offset_.y = (size_.y - output_size_.y) * 0.5f;
        }
    }
}
