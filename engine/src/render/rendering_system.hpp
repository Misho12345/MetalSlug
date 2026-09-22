#pragma once

#include "gl/vao.hpp"
#include "gl/buffer.hpp"
#include "gl/shader.hpp"
#include "gl/texture_2d.hpp"
#include "mse/ecs/scene.hpp"

namespace mse
{
    struct PrivCtx;

    /**
     * @brief Renderer class
     * @details It manages the rendering of sprites,
     * the SpriteAnimationData buffers, InstanceData, CameraData and sprite atlases
     * @note Owned by PrivCtx, not constructable by anything else and not movable or copyable
     */
    class RenderingSystem final
    {
    public:
        RenderingSystem(const RenderingSystem&)            = delete;
        RenderingSystem(RenderingSystem&&)                 = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;
        RenderingSystem& operator=(RenderingSystem&&)      = delete;

        void init_global() const;
        bool init_sprite_objects(const Scene& scene);
        void render_sprites(const Scene& scene);

    private:
        RenderingSystem();

        /// @brief struct for CameraData UBO
        struct alignas(16) CameraData final
        {
            glm::mat4 projection;
            glm::vec2 camera_pos;
        };

        /// @brief struct for InstanceData SSBO
        struct alignas(8) InstanceData final
        {
            glm::vec2 parallax_factor;

            glm::vec2 position;
            glm::vec2 scale;
            float     rotation;

            uint32_t  anim_idx;
            uint32_t  frame_idx;
        };

        void update_instances_buffer(const Scene& scene);

        void init_camera_data(const Scene& scene);
        void init_instance_data(const Scene& scene);
        void init_sprite_anim_data();
        bool init_sprite_atlases();

        static void set_blending(bool blending);

        gl::VAO sprite_vao_{};

        gl::Buffer camera_data_{};
        gl::Buffer instance_data_{};
        gl::Buffer sprite_anim_data_{};

        gl::Shader    sprite_shader_{};
        gl::Texture2D sprite_atlases_{};

        vector<vector<InstanceData>> instances_{};
        vector<int32_t> layers_{};
        vector<size_t>  order_{};

        friend ::mse::PrivCtx;
    };
}
