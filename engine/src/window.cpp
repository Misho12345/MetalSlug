#include "mse/pch.hpp"
#include "window.hpp"

namespace mse
{
    static bool existing_win;

    bool Window::init(const WindowDesc& desc)
    {
        assert(!existing_win && "Only one window is allowed");
        existing_win = true;

        glfwInit();

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

        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // TODO: make resize callback
        glfwMakeContextCurrent(handle_);

        desc_ = desc;
        return true;
    }

    bool Window::should_close() const { return glfwWindowShouldClose(handle_); }
    void Window::poll_events() const { glfwPollEvents(); }
    void Window::swap_buffers() { glfwSwapBuffers(handle_); }

    Window::~Window()
    {
        glfwDestroyWindow(handle_);
        glfwTerminate();
    }
}
