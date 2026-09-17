#include "rendering_system.hpp"

#include "priv_ctx.hpp"

#include "mse/app.hpp"

namespace mse
{
    // just for the qsort lambda
    static int32_t* s_layers;

    RenderingSystem::RenderingSystem() : instances_{ 5 }
    {
        layers_.reserve(5);
        order_.reserve(5);
    }


    bool RenderingSystem::init_sprite_objects(const Scene& scene)
    {
        glEnable(GL_CULL_FACE);

        // empty vao because sprite vertices are hardcoded in shader
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
        }

        init_camera_data(scene);
        init_instance_data(scene);
        init_sprite_anim_data();

        if (!init_sprite_atlases()) return false;

        sprite_shader_.use();
        sprite_vao_.bind();

        return true;
    }

    void RenderingSystem::render_sprites(const Scene& scene)
    {
        update_instances_buffer(scene);

        sprite_shader_.use();
        sprite_vao_.bind();

        camera_data_.bind(0);
        instance_data_.bind(1);
        sprite_anim_data_.bind(2);

        sprite_atlases_.bind(0);

        set_blending(true);

        size_t acc_size = 0;
        for (const size_t layer_idx : order_)
        {
            const vector<InstanceData>& layer = instances_[layer_idx];

            // there are 6 hardcoded vertices in the vertex shader for the quad
            // with depth testing there would be only alpha discarding, which is not what the assignment asks for
            // that's why the scene layers are drawn in different draw calls -
            // to ensure proper order without manual sorting and to make alpha blending possible

            glDrawArraysInstancedBaseInstance(GL_TRIANGLES,
                0, 6,
                static_cast<GLsizei>(layer.size()), acc_size);

            acc_size += layer.size();
        }

        gl::VAO::unbind();
        gl::Shader::unuse();
    }


    void RenderingSystem::update_instances_buffer(const Scene& scene)
    {
        const ComponentPool<SpriteRenderer>& sprite_pool = scene.pool<SpriteRenderer>();
        const vector<SpriteRenderer>& sprites = sprite_pool.components();
        const vector<entity_id>& entities = sprite_pool.owners();

        for (vector<InstanceData>& instances_layer : instances_) instances_layer.clear();
        layers_.clear();

        assert(sprites.size() == entities.size());

        // collect all the instances
        for (size_t i = 0; i < sprites.size(); ++i)
        {
            const SpriteRenderer& sprite = sprites[i];

            if (sprite.hidden ||
                !scene.has<Transform>(entities[i]))
                continue;

            const Transform& transform = scene.get<Transform>(entities[i]);
            if (!(transform.bounds() & App::priv_ctx().window.bounds())) continue;

            size_t idx = layers_.find(sprite.layer);

            if (idx == vector<>::npos)
            {
                layers_.emplace_back(sprite.layer);

                if (instances_.size() < layers_.size())
                {
                    instances_.emplace_back();
                    instances_.back().reserve(25);
                }

                idx = layers_.size() - 1;
            }

            uint32_t anim_id_offset = 0;
            for (int j = 0; j < sprite.info().sprite_id; ++j)
            {
                anim_id_offset += anim::sprite_anim_count(j);
            }

            instances_[idx].emplace_back(
                    sprite.parallax_factor,
                    transform.top_left(),
                    transform.scale,
                    transform.rotation,
                    anim_id_offset + sprite.info().anim_id,
                    sprite.frame());
        }


        // transfer the data into the persistent buffer
        size_t total_size = 0;
        for (const vector<InstanceData>& instances_layer : instances_) total_size += instances_layer.size();
        instance_data_.increase_size(static_cast<GLsizeiptr>(total_size));


        // get storage order for layers
        order_.clear();
        for (size_t i = 0; i < layers_.size(); ++i)
        {
            if (instances_[layers_[i]].empty()) continue;
            order_.emplace_back(i);
        }

        s_layers = layers_.data();

        qsort(order_.data(), order_.size(), sizeof(size_t),
            [](const void* a, const void* b) -> int
            {
                const size_t idx_a = *static_cast<const size_t*>(a);
                const size_t idx_b = *static_cast<const size_t*>(b);

                return s_layers[idx_a] - s_layers[idx_b];
            });

        // fill the buffer (sorted)
        InstanceData* ptr = instance_data_.mapped_data<InstanceData>();

        for (const size_t layer_idx : order_)
        {
            const vector<InstanceData>& instances_layer = instances_[layer_idx];

            if (instances_layer.empty()) continue;
            memcpy(ptr, instances_layer.data(), instances_layer.size() * sizeof(InstanceData));
            ptr += instances_layer.size();
        }
    }


    void RenderingSystem::init_camera_data(const Scene& scene)
    {
        CameraData data
        {
            .projection = glm::ortho(
                0.0f,
                static_cast<float>(App::priv_ctx().window.width()),
                static_cast<float>(App::priv_ctx().window.height()),
                0.0f, -1.0f, 1.0f
            ),
            .camera_pos = scene.get<Transform>(scene.camera()).position
        };

        camera_data_.create<CameraData>(gl::BufferType::Uniform, { &data, 1 });
    }

    void RenderingSystem::init_instance_data(const Scene& scene)
    {
        const ComponentPool<SpriteRenderer>& sprite_pool = scene.pool<SpriteRenderer>();

        instance_data_.create_persistent<InstanceData>(
            gl::BufferType::Storage,
            static_cast<GLsizeiptr>(max(sprite_pool.components().size(), 100_zu)));

        update_instances_buffer(scene);
    }

    void RenderingSystem::init_sprite_anim_data()
    {
        const AnimationSystem& anim_sys = App::priv_ctx().animation_system;

        vector<SpriteAnimationData> anim_datas;
        anim_datas.reserve(anim::total_anim_count);

        for (uint32_t i = 0; i < anim::sprite_count; ++i)
        {
            for (uint32_t j = 0; j < anim::sprite_anim_count(i); ++j)
            {
                anim_datas.emplace_back(anim_sys.anim_data({
                    .sprite_id = static_cast<int>(i),
                    .anim_id = j
                }));
            }
        }

        sprite_anim_data_.create<SpriteAnimationData>(gl::BufferType::Storage, anim_datas);
    }

    bool RenderingSystem::init_sprite_atlases()
    {
        const AnimationSystem& anim_sys = App::priv_ctx().animation_system;

        vector<string> atlas_paths;
        atlas_paths.reserve(anim_sys.atlas_count());

        char buf[256];
        for (size_t i = 0; i < anim_sys.atlas_count(); ++i)
        {
            snprintf(buf, sizeof(buf), "texture_packer/atlas_%zu.png", i);
            atlas_paths.emplace_back(buf);
        }

        if (!sprite_atlases_.create(
            {
                .format = gl::TextureFormat::RGBA8,
                .wrap   = gl::TextureWrap::ClampToEdge,
                .filter = gl::TextureFilter::Nearest // not linear because pixels will be smeared
            }, atlas_paths, true))
        {
            printf("failed to create texture 2d array for atlases");
            return false;
        }

        return true;
    }


    void RenderingSystem::set_blending(const bool blending)
    {
        static bool enabled = false;

        if (enabled != blending)
        {
            enabled = blending;
            if (blending)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else glDisable(GL_BLEND);
        }
    }
}
