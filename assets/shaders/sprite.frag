#version 460 core

in vec2 v_TexCoord;
in flat uint v_AnimationIdx;
in flat uint v_FrameIdx;

struct SpriteAnimationData
{
    ivec2 offset;
    ivec2 frame_size;
    uint frame_count;
    uint atlas_idx;
};

layout (binding = 0) uniform sampler2DArray u_Atlases;

layout (std430, binding = 2) readonly buffer SpriteAnimationDataBuffer
{
    SpriteAnimationData sprite_data[];
};

out vec4 FragColor;

void main()
{
    SpriteAnimationData data = sprite_data[v_AnimationIdx];

    vec2 atlas_size = vec2(textureSize(u_Atlases, 0));

    vec2 pixel_min = vec2(
        data.offset.x + (float(v_FrameIdx) * data.frame_size.x),
        data.offset.y
    );

    vec2 pixel_max = pixel_min + data.frame_size;

    vec2 uv_min = pixel_min / atlas_size;
    vec2 uv_max = pixel_max / atlas_size;

    vec2 uv = mix(uv_min, uv_max, v_TexCoord);

    FragColor = texture(u_Atlases, vec3(uv, float(data.atlas_idx)));
}