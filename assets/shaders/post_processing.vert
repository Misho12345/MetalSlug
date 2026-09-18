#version 460 core

const vec2 QUAD_POS[6] = vec2[](
        vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0),
        vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0)
);

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = QUAD_POS[gl_VertexID];
    gl_Position = vec4(v_TexCoord * 2.0 - 1.0, 0.0, 1.0);
}