#ifndef NDEBUG

#include "debug_ui.hpp"

#include "mse/app.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        ImVec2 to_vec2(const glm::vec2 vec) { return ImVec2{ vec.x, vec.y }; }
        ImU32  to_imu32(const glm::u8vec4 vec) { return IM_COL32(vec.x, vec.y, vec.z, vec.w); }
    }

    DebugUI::~DebugUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void DebugUI::init() const
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io    = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui_ImplGlfw_InitForOpenGL(App::priv_ctx().window.native_handle(), true);
        ImGui_ImplOpenGL3_Init();
    }

    void DebugUI::begin_frame() const
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void DebugUI::render() const
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void DebugUI::draw_box(const aabb box, const glm::u8vec4 color) const
    {
        ImGui::GetForegroundDrawList()->AddRect(
            to_vec2(box.min),
            to_vec2(box.max),
            to_imu32(color),
            0.0f, {},
            1.0f);
    }
}

#endif
