$input v_color0, v_normal, v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_lightDir;   // xyz = light direction (toward light), w unused
uniform vec4 u_eyePos;     // xyz = camera position, w = specular power

void main()
{
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightDir.xyz);
    vec3 V = normalize(u_eyePos.xyz - v_worldPos);
    vec3 H = normalize(L + V);

    // Ambient
    float ambient = 0.15;

    // Diffuse (two-sided)
    float diff = max(dot(N, L), 0.0) * 0.7;

    // Specular (Blinn-Phong)
    float spec = pow(max(dot(N, H), 0.0), u_eyePos.w) * 0.3;

    vec3 lit = v_color0.rgb * (ambient + diff) + vec3_splat(spec);
    gl_FragColor = vec4(lit, v_color0.a);
}
