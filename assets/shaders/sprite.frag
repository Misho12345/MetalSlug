#version 460 core

in vec2 v_TexCoord;
in flat uint v_AtlasIdx;
in flat vec4 v_Color;

layout (binding = 0) uniform sampler2DArray u_Atlases;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_Atlases, vec3(v_TexCoord, float(v_AtlasIdx))) * v_Color;
}