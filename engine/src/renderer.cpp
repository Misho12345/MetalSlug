#include "renderer.hpp"

#include "mse/app.hpp"
#include "mse/sprite.hpp"
#include "priv_ctx.hpp"

namespace mse
{
    bool Renderer::init_sprite_objects()
    {
        sprite_vao_.create({});

        if (!sprite_vao_)
        {
            printf("failed to create sprite_vao_");
            return false;
        }

        if (!sprite_shader_.create(
            "assets/shaders/sprite.vert",
            "assets/shaders/sprite.frag"))
        {
            printf("failed to create sprite shader\n");
            return false;
        };

        init_camera_data();
        init_instance_data();
        init_sprite_anim_data();

        if (!init_sprite_atlases()) return false;

        sprite_shader_.use();
        sprite_vao_.bind();

        return true;
    }

    void Renderer::render_sprites()
    {
        update_instances_buffer();

        sprite_shader_.use();
        sprite_vao_.bind();

        camera_data_.bind(0);
        instance_data_.bind(1);
        sprite_anim_data_.bind(2);

        sprite_atlases_.bind(0);

        glDrawArraysInstanced(
            GL_TRIANGLES,
            0, 6,
            static_cast<GLsizei>(App::priv_ctx().sprite_manager.sprites().size()));

        gl::VAO::unbind();
        gl::Shader::unuse();
    }


    void Renderer::update_instances_buffer()
    {
        SpriteManager& sm = App::priv_ctx().sprite_manager;

        if (sm.dirty().empty()) return;

        if (instance_data_.size<InstanceData>() < sm.sprites().size())
        {
            instance_data_.increase_size(sm.sprites().size() * 2);
        }

        InstanceData* ptr = instance_data_.mapped_data<InstanceData>();

        for (size_t i = 0; i < sm.sprites().size(); ++i)
        {
            if (sm.dirty()[i])
            {
                fill_sprite_data(sm.sprites()[i], ptr + i);
                sm.dirty()[i] = false;
            }
        }
    }


    void Renderer::init_camera_data()
    {
        CameraData data
        {
            glm::ortho(
                0.0f,
                static_cast<float>(App::priv_ctx().window.width()),
                static_cast<float>(App::priv_ctx().window.height()),
                0.0f, -1.0f, 1.0f
            )
        };

        camera_data_.create<CameraData>(gl::BufferType::Uniform, { &data, 1 });
    }

    void Renderer::init_instance_data()
    {
        const SpriteManager& sm = App::priv_ctx().sprite_manager;
        const GLsizeiptr instance_alloc_size = static_cast<GLsizeiptr>(min(sm.sprites().size(), 100_zu));

        instance_data_.create_persistent<InstanceData>(gl::BufferType::Storage, instance_alloc_size);

        InstanceData* ptr = instance_data_.mapped_data<InstanceData>();

        for (size_t i = 0; i < sm.sprites().size(); ++i)
        {
            fill_sprite_data(sm.sprites()[i], ptr + i);
        }
    }

    void Renderer::init_sprite_anim_data()
    {
        const SpriteManager& sm = App::priv_ctx().sprite_manager;

        vector<SpriteAnimationData> anim_datas;
        anim_datas.reserve(anim::total_anim_count);

        for (int i = 0; i < anim::sprite_count; ++i)
        {
            for (int j = 0; j < anim::sprite_anim_count(i); ++j)
            {
                anim_datas.emplace_back(sm.anim_data({ i, j }));
            }
        }

        sprite_anim_data_.create<SpriteAnimationData>(gl::BufferType::Storage, anim_datas);
    }

    bool Renderer::init_sprite_atlases()
    {
        const SpriteManager& sm = App::priv_ctx().sprite_manager;

        vector<string> atlas_paths;
        atlas_paths.reserve(sm.atlas_count());

        char buf[256];
        for (size_t i = 0; i < sm.atlas_count(); ++i)
        {
            snprintf(buf, sizeof(buf), "texture_packer/atlas_%zu.png", i);
            atlas_paths.emplace_back(buf);
        }

        if (!sprite_atlases_.create(
            {
                .format = gl::TextureFormat::RGBA8,
                .wrap   = gl::TextureWrap::ClampToEdge,
                .filter = gl::TextureFilter::Nearest
            }, std::move(atlas_paths), true))
        {
            printf("failed to create texture 2d array for atlases");
            return false;
        }

        return true;
    }

    void Renderer::fill_sprite_data(const Sprite& sprite, InstanceData* data)
    {
        uint32_t count = 0;
        for (int c = 0; c < sprite.info().sprite_id; ++c) count += anim::sprite_anim_count(c);

        data->offset    = sprite.pos();
        data->scale     = sprite.size();
        data->rotation  = sprite.rot();
        data->anim_idx  = count + sprite.info().anim_id;
        data->frame_idx = sprite.frame();
    }
}
