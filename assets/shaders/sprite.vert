#version 460 core

// UBO and not free uniform because I want to use it for other shaders as well
layout (std140, binding = 0) uniform CameraData
{
    mat4 u_Projection;
    vec2 u_CameraPos;
};

struct InstanceData
{
    vec2 parallax_factor;

    ivec2 position;
    uvec2 scale;
    float rotation;

    uint anim_idx;
    uint frame_idx;

    bool flip_x;

    uint color;
};

struct SpriteAnimationData
{
    ivec2 offset;
    uvec2 frame_size;
    uint frame_count;
    uint atlas_idx;
};

layout (std430, binding = 1) readonly buffer InstanceDataBuffer
{
    InstanceData instances[];
};

layout (std430, binding = 2) readonly buffer SpriteAnimationDataBuffer
{
    SpriteAnimationData sprite_data[];
};

layout (binding = 0) uniform sampler2DArray u_Atlases;

out vec2 v_TexCoord;
out flat uint v_AtlasIdx;
out flat vec4 v_Color;

const vec2 QUAD_POS[6] = vec2[](
        vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0),
        vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0)
);

vec2 rotate(in vec2 pos, in float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, s, -s, c) * pos;
}

uvec2 frame_pos(SpriteAnimationData a, uint frame, uint atlas_size)
{
    uint first_row = (atlas_size - uint(a.offset.x)) / a.frame_size.x;
    if (frame < first_row) return uvec2(a.offset) + uvec2(frame * a.frame_size.x, 0u);

    uint per_row = atlas_size / a.frame_size.x;
    uint j = frame - first_row;
    return uvec2(j % per_row * a.frame_size.x, uint(a.offset.y) + a.frame_size.y * (1u + j / per_row));
}

void main()
{
    InstanceData instance = instances[gl_InstanceID + gl_BaseInstance];
    SpriteAnimationData data = sprite_data[instance.anim_idx];

    vec2 base_pos = QUAD_POS[gl_VertexID];
    vec2 frame_uv = abs(vec2(instance.flip_x, 0) - base_pos);

    uvec2 atlas_size = uvec2(textureSize(u_Atlases, 0).xy);
    vec2 pixel = vec2(frame_pos(data, instance.frame_idx, atlas_size.x)) + vec2(data.frame_size) * frame_uv;

    v_TexCoord = pixel / vec2(atlas_size);
    v_AtlasIdx = data.atlas_idx;
    v_Color = unpackUnorm4x8(instance.color);

    vec2 size = data.frame_size * instance.scale;
    vec2 world_pos = rotate((base_pos - 0.5) * size, instance.rotation) + instance.position;
    vec2 view_pos = world_pos - (u_CameraPos * instance.parallax_factor);

    gl_Position = u_Projection * vec4(ceil(view_pos), 0.0, 1.0);
}