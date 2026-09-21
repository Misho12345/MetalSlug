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
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

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
            desc.width, desc.height, desc.title,
            nullptr, nullptr);

        if (!handle_)
        {
            printf("Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }

        glfwSetWindowPos(
            handle_,
            (mode->width - desc.width) / 2,
            (mode->height - desc.height) / 2);

        glfwMakeContextCurrent(handle_);

        glfwSetWindowUserPointer(handle_, this);
        glfwSetWindowSizeCallback(handle_, [](GLFWwindow* window, const int width, const int height)
        {
            if (Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window)))
                win->update({ width, height });
        });

        update({ desc.width, desc.height });

        return true;
    }


    void Window::update(const glm::ivec2 new_size)
    {
        size_ = new_size;

        const float ar = aspect_ratio();

        if (ar > Target::DAR)
        {
            output_size_.x   = static_cast<int32_t>(size_.y * Target::DAR);
            output_size_.y   = size_.y;
            output_offset_.x = (size_.x - output_size_.x) / 2;
            output_offset_.y = 0u;
        }
        else
        {
            output_size_.x = size_.x;
            output_size_.y = static_cast<int32_t>(size_.x / Target::DAR);
            output_offset_.x = 0u;
            output_offset_.y = (size_.y - output_size_.y) / 2;
        }
    }

    bool Window::should_close() const { return glfwWindowShouldClose(handle_); }
    void Window::poll_events() const { glfwPollEvents(); }
    void Window::swap_buffers() const { glfwSwapBuffers(handle_); }
}
