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

    vec2 position;
    vec2 scale;
    float rotation;

    uint anim_idx;
    uint frame_idx;
};

layout (std430, binding = 1) readonly buffer InstanceDataBuffer
{
    InstanceData instances[];
};

out vec2 v_TexCoord;
out flat uint v_AnimationIdx;
out flat uint v_FrameIdx;

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

void main()
{
    InstanceData instance = instances[gl_InstanceID + gl_BaseInstance];

    vec2 basePos = QUAD_POS[gl_VertexID];

    v_TexCoord = basePos;
    v_AnimationIdx = instance.anim_idx;
    v_FrameIdx = instance.frame_idx;

    vec2 worldPos = rotate(basePos * instance.scale, instance.rotation) + instance.position;
    vec2 viewPos = worldPos - (u_CameraPos * instance.parallax_factor);

    gl_Position = u_Projection * vec4(viewPos, 0.0, 1.0);
}