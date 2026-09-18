#version 460 core

in vec2 v_TexCoord;

layout (binding = 0) uniform sampler2D u_Screen;

out vec4 FragColor;

void main()
{
    vec4 color = texture(u_Screen, v_TexCoord);
//    FragColor = vec4(1 - color.rgb, 1);
    FragColor = vec4(color.rgb, 1);
}


