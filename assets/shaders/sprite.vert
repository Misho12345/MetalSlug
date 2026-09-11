#version 460 core

layout (std140, binding = 0) uniform CameraData
{
    mat4 u_Projection;
};

struct InstanceData
{
    vec2 offset;
    vec2 scale;
    float rotation;
    uint animationIdx;
    uint frameIdx;
};

layout (std430, binding = 1) readonly buffer InstanceDataBuffer
{
    InstanceData instances[];
};

out vec2 v_TexCoord;
out flat uint v_AnimationIdx;
out flat uint v_FrameIdx;

const vec2 QUAD_POS[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
    vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

vec2 rotate(in vec2 pos, in float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return vec2(
        pos.x * c - pos.y * s,
        pos.x * s + pos.y * c
    );
}

void main()
{
    InstanceData instance = instances[gl_InstanceID];

    vec2 basePos = QUAD_POS[gl_VertexID];

    v_TexCoord = basePos;
    v_AnimationIdx = instance.animationIdx;
    v_FrameIdx = instance.frameIdx;

    vec2 pos = rotate(basePos * instance.scale, instance.rotation) + instance.offset;

    gl_Position = u_Projection * vec4(pos, 0.0, 1.0);
}