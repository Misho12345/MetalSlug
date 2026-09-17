#version 460 core

in vec2 v_TexCoord;
in flat uint v_AnimationIdx;
in flat uint v_FrameIdx;

struct SpriteAnimationData
{
    vec2 offset;
    vec2 image_size;
    uint frame_count;
    uint atlas_idx;
};

layout (binding = 0) uniform sampler2DArray u_Atlases;

layout (std430, binding = 2) readonly buffer SpriteAnimationDataBuffer
{
    SpriteAnimationData spriteData[];
};

out vec4 FragColor;

void main()
{
    SpriteAnimationData data = spriteData[v_AnimationIdx];

    vec2 atlasSize = vec2(textureSize(u_Atlases, 0));
    float frameWidth = data.image_size.x / float(data.frame_count);

    vec2 pixelMin = vec2(
        data.offset.x + (float(v_FrameIdx) * frameWidth),
        data.offset.y
    );

    vec2 pixelMax = pixelMin + vec2(frameWidth, data.image_size.y);

    vec2 uvMin = pixelMin / atlasSize;
    vec2 uvMax = pixelMax / atlasSize;

    vec2 uv = mix(uvMin, uvMax, v_TexCoord);

    FragColor = texture(u_Atlases, vec3(uv, float(data.atlas_idx)));
}