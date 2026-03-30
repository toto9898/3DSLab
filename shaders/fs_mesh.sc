$input v_color0, v_normal, v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_lightDir;   // xyz = light direction (toward light), w unused
uniform vec4 u_eyePos;     // xyz = camera position, w = specular power
uniform vec4 u_lightIntensity; // x = global light intensity multiplier
uniform vec4 u_doubleSided; // x = 1.0 -> enable two-sided normals

// Convert sRGB color to linear space (accurate piecewise sRGB curve)
vec3 srgb_to_linear(vec3 c)
{
    vec3 lt;
    lt.r = (c.r <= 0.04045) ? c.r / 12.92 : pow(max((c.r + 0.055) / 1.055, 0.0), 2.4);
    lt.g = (c.g <= 0.04045) ? c.g / 12.92 : pow(max((c.g + 0.055) / 1.055, 0.0), 2.4);
    lt.b = (c.b <= 0.04045) ? c.b / 12.92 : pow(max((c.b + 0.055) / 1.055, 0.0), 2.4);
    return lt;
}

// Convert linear color back to sRGB for display
vec3 linear_to_srgb(vec3 c)
{
    vec3 s;
    s.r = (c.r <= 0.0031308) ? c.r * 12.92 : 1.055 * pow(max(c.r, 0.0), 1.0 / 2.4) - 0.055;
    s.g = (c.g <= 0.0031308) ? c.g * 12.92 : 1.055 * pow(max(c.g, 0.0), 1.0 / 2.4) - 0.055;
    s.b = (c.b <= 0.0031308) ? c.b * 12.92 : 1.055 * pow(max(c.b, 0.0), 1.0 / 2.4) - 0.055;
    return s;
}

void main()
{
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightDir.xyz);
    vec3 V = normalize(u_eyePos.xyz - v_worldPos);
    vec3 H = normalize(L + V);

    // Optionally treat normals as double-sided by flipping normals that point away from the view
    if (u_doubleSided.x > 0.5 && dot(N, V) < 0.0)
        N = -N;

    // Read vertex color (assumed sRGB) and convert to linear for lighting
    vec3 color_srgb = v_color0.rgb;
    float alpha = v_color0.a;
    vec3 color = srgb_to_linear(color_srgb);

    // Lighting terms (compute in linear space)
    float ambient = 0.08; // lower ambient for better contrast
    float diff = max(dot(N, L), 0.0) * 1.0; // stronger diffuse
    float spec = pow(max(dot(N, H), 0.0), u_eyePos.w) * 1.0;

    vec3 lit_linear = (color * (ambient + diff) + spec * vec3(1.0, 1.0, 1.0)) * u_lightIntensity.x;
    lit_linear = max(lit_linear, vec3(0.0, 0.0, 0.0));

    // Convert back to sRGB for output
    vec3 lit = linear_to_srgb(lit_linear);
    lit = clamp(lit, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));

    gl_FragColor = vec4(lit, alpha);
}
