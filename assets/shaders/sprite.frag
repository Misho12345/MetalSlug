#version 460 core

in vec2 v_TexCoord;
in flat uint v_AnimationIdx;
in flat uint v_FrameIdx;

struct SpriteAnimationData
{
    ivec2 offset;
    uvec2 frame_size;
    uint frame_count;
    uint atlas_idx;
};

layout (binding = 0) uniform sampler2DArray u_Atlases;

layout (std430, binding = 2) readonly buffer SpriteAnimationDataBuffer
{
    SpriteAnimationData sprite_data[];
};

out vec4 FragColor;

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
    SpriteAnimationData data = sprite_data[v_AnimationIdx];

    uvec2 atlas_size = uvec2(textureSize(u_Atlases, 0).xy);

    vec2 pixel = vec2(frame_pos(data, v_FrameIdx, atlas_size.x)) + vec2(data.frame_size) * v_TexCoord;
    vec2 uv    = pixel / vec2(atlas_size);

    FragColor = texture(u_Atlases, vec3(uv, float(data.atlas_idx)));
}