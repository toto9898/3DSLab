$input a_position, a_normal, a_color0, a_color1, a_texcoord1, a_texcoord0
$output v_color0, v_color1, v_normal, v_worldPos, v_specular, v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    vec4 worldPos = mul(u_model[0], vec4(a_position, 1.0));
    gl_Position = mul(u_viewProj, worldPos);
    v_worldPos = worldPos.xyz;
    v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    v_color0 = a_color0;
    v_color1 = a_color1;
    v_specular = a_texcoord1;
    v_texcoord0 = a_texcoord0;
}
