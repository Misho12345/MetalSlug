#version 460 core

// CRT implementation is from here
// https://github.com/libretro/glsl-shaders/blob/master/crt/shaders/crt-lottes.glsl
//
// Kept only what seemed enough for metal slug: no bloom, one mask (aperture grille), the original's simple
// pow 2.2 gamma path, and Gaus with the kernel shape fixed at 2 (a plain Gaussian).

in vec2 v_TexCoord;

layout (binding = 0) uniform sampler2D u_Screen;

uniform bool  u_Enabled;
uniform vec2  u_Warp;
uniform float u_HardScan;
uniform float u_HardPix;
uniform float u_MaskDark;
uniform float u_MaskLight;
uniform float u_BrightBoost;

out vec4 FragColor;

vec2 sourceSize;

// Nearest emulated sample given floating point position and texel offset.
vec3 Fetch(vec2 pos, vec2 off)
{
    pos = (floor(pos * sourceSize + off) + 0.5) / sourceSize;
    return u_BrightBoost * pow(texture(u_Screen, pos).rgb, vec3(2.2));
}

// Distance in emulated pixels to nearest texel.
vec2 Dist(vec2 pos)
{
    pos *= sourceSize;
    return -((pos - floor(pos)) - 0.5);
}

// 1D Gaussian.
float Gaus(float pos, float scale)
{
    return exp2(scale * pos * pos);
}

// 3-tap Gaussian filter along horz line.
vec3 Horz3(vec2 pos, float off)
{
    vec3 b = Fetch(pos, vec2(-1.0, off));
    vec3 c = Fetch(pos, vec2(0.0, off));
    vec3 d = Fetch(pos, vec2(1.0, off));
    float dst = Dist(pos).x;

    float wb = Gaus(dst - 1.0, u_HardPix);
    float wc = Gaus(dst + 0.0, u_HardPix);
    float wd = Gaus(dst + 1.0, u_HardPix);

    return (b * wb + c * wc + d * wd) / (wb + wc + wd);
}

// 5-tap Gaussian filter along horz line.
vec3 Horz5(vec2 pos, float off)
{
    vec3 a = Fetch(pos, vec2(-2.0, off));
    vec3 b = Fetch(pos, vec2(-1.0, off));
    vec3 c = Fetch(pos, vec2(0.0, off));
    vec3 d = Fetch(pos, vec2(1.0, off));
    vec3 e = Fetch(pos, vec2(2.0, off));
    float dst = Dist(pos).x;

    float wa = Gaus(dst - 2.0, u_HardPix);
    float wb = Gaus(dst - 1.0, u_HardPix);
    float wc = Gaus(dst + 0.0, u_HardPix);
    float wd = Gaus(dst + 1.0, u_HardPix);
    float we = Gaus(dst + 2.0, u_HardPix);

    return (a * wa + b * wb + c * wc + d * wd + e * we) / (wa + wb + wc + wd + we);
}

// Return scanline weight.
float Scan(vec2 pos, float off)
{
    return Gaus(Dist(pos).y + off, u_HardScan);
}

// Allow nearest three lines to effect pixel.
vec3 Tri(vec2 pos)
{
    vec3 a = Horz3(pos, -1.0);
    vec3 b = Horz5(pos, 0.0);
    vec3 c = Horz3(pos, 1.0);

    return a * Scan(pos, -1.0) + b * Scan(pos, 0.0) + c * Scan(pos, 1.0);
}

// Distortion of scanlines.
vec2 Warp(vec2 pos)
{
    pos = pos * 2.0 - 1.0;
    pos *= vec2(1.0 + pos.y * pos.y * u_Warp.x, 1.0 + pos.x * pos.x * u_Warp.y);
    return pos * 0.5 + 0.5;
}

// Aperture grille (shadowMask 2 in the original)
vec3 Mask(vec2 pos)
{
    vec3 mask = vec3(u_MaskDark);
    pos.x = fract(pos.x / 3.0);

    if (pos.x < 0.333) mask.r = u_MaskLight;
    else if (pos.x < 0.666) mask.g = u_MaskLight;
    else mask.b = u_MaskLight;

    return mask;
}

void main()
{
    if (!u_Enabled)
    {
        FragColor = vec4(texture(u_Screen, v_TexCoord).rgb, 1.0);
        return;
    }

    sourceSize = vec2(textureSize(u_Screen, 0));
    vec2 pos = Warp(v_TexCoord);

    if (any(lessThan(pos, vec2(0.0))) || any(greaterThan(pos, vec2(1.0))))
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 color = Tri(pos) * Mask(gl_FragCoord.xy);
    FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
