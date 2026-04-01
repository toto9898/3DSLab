$input v_color0, v_color1, v_normal, v_worldPos, v_specular, v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 u_lightDir;   // xyz = light direction (toward light), w unused
uniform vec4 u_eyePos;     // xyz = camera position, w unused
uniform vec4 u_lightIntensity; // x = global light intensity multiplier
uniform vec4 u_doubleSided; // x = 1.0 -> enable two-sided normals
uniform vec4 u_hasTexture;  // x = 1.0 -> mesh has a diffuse texture
SAMPLER2D(s_texColor, 0);

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

    // Per-vertex material properties
    // v_color0.rgb = diffuse color (sRGB), v_color0.a = opacity (1 - transparency)
    // v_color1.rgb = ambient color (sRGB), v_color1.a = self-illumination factor
    // v_specular.rgb = specular color (linear), v_specular.a = shininess exponent
    vec3 diffuseColor = srgb_to_linear(v_color0.rgb);
    float alpha = v_color0.a;

    // Apply diffuse texture if present (UV sentinel -1e6 means untextured face)
    if (u_hasTexture.x > 0.5 && v_texcoord0.x > -1e5)
    {
        vec4 texColor = texture2D(s_texColor, v_texcoord0);
        diffuseColor *= srgb_to_linear(texColor.rgb);
        alpha *= texColor.a;
    }

    vec3 ambientColor = srgb_to_linear(v_color1.rgb);
    float selfIllum = v_color1.a;
    vec3 specularColor = v_specular.rgb;
    float shininess = max(v_specular.a, 1.0);

    // Lighting terms (compute in linear space)
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, shininess);

    vec3 lit_linear = ambientColor * diffuseColor
                    + NdotL * diffuseColor
                    + spec * specularColor;

    // Self-illumination: blend toward unshaded diffuse color
    lit_linear = mix(lit_linear, diffuseColor, selfIllum);

    lit_linear = lit_linear * u_lightIntensity.x;
    lit_linear = max(lit_linear, vec3(0.0, 0.0, 0.0));

    // Convert back to sRGB for output
    vec3 lit = linear_to_srgb(lit_linear);
    lit = clamp(lit, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));

    gl_FragColor = vec4(lit, alpha);
}
