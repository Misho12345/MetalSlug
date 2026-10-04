#ifndef NDEBUG

#include "debug_ui.hpp"

#include "mse/app.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    namespace
    {
        ImVec2 to_vec2(const vec2 vec) { return ImVec2{ vec.x, vec.y }; }
        ImU32  to_imu32(const color vec) { return IM_COL32(vec.x, vec.y, vec.z, vec.w); }
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

    void DebugUI::draw_box(const aabb box, const color color, const float thickness) const
    {
        const aabb cam_local = box - App::ctx().scene.camera_pos_screen();
        if (!(cam_local & aabb::screen)) return;

        vec2 scale, off;
        get_screen_transform(scale, off);

        const aabb rel = cam_local * scale + off;

        ImGui::GetForegroundDrawList()->AddRect(
            to_vec2(rel.min), to_vec2(rel.max),
            to_imu32(color), 0.0f, {}, thickness);
    }

    void DebugUI::draw_line(ivec2 start, ivec2 end, const color color, const float thickness) const
    {
        const ivec2 cam = App::ctx().scene.camera_pos_screen();
        start -= cam;
        end -= cam;

        if (!(aabb::screen & aabb{ start, end }) &&
            !(aabb::screen & aabb{ end, start })) return;

        vec2 scale, off;
        get_screen_transform(scale, off);

        const vec2 start_f = vec2(start) * scale + off;
        const vec2 end_f   = vec2(end) * scale + off;

        ImGui::GetForegroundDrawList()->AddLine(
            to_vec2(start_f), to_vec2(end_f),
            to_imu32(color), thickness);
    }

    void DebugUI::get_screen_transform(vec2& out_scale, vec2& out_offset)
    {
        static constexpr vec2 res_f = Target::RESOLUTION;
        const Window& win = App::priv_ctx().window;

        out_scale = vec2(win.output_size()) / res_f;
        out_offset = win.output_offset();
    }
}

#endif
