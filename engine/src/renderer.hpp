#pragma once

#include "gl/vao.hpp"
#include "gl/buffer.hpp"
#include "gl/shader.hpp"
#include "gl/texture_2d.hpp"

#include "mse/sprite.hpp"

namespace mse
{
    class Renderer final
    {
    public:
        bool init_sprite_objects();
        void render_sprites();

    private:
        struct CameraData final
        {
            glm::mat4 projection;
        };

        struct alignas(8) InstanceData final
        {
            glm::vec2 offset;
            glm::vec2 scale;
            float     rotation;
            uint32_t  anim_idx;
            uint32_t  frame_idx;
        };

        void update_instances_buffer();

        void init_camera_data();
        void init_instance_data();
        void init_sprite_anim_data();
        bool init_sprite_atlases();

        static void fill_sprite_data(const Sprite& sprite, InstanceData* data);

        gl::VAO sprite_vao_{};

        gl::Buffer camera_data_{};
        gl::Buffer instance_data_{};
        gl::Buffer sprite_anim_data_{};

        gl::Shader    sprite_shader_{};
        gl::Texture2D sprite_atlases_{};
    };
}
