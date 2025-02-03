#version 330 core

in vec3 o_normal;
in vec2 o_texcoord;
in vec4 o_color;

uniform bool u_is_selected;
uniform vec4 u_light_color;
uniform vec3 u_light_dir;

uniform sampler2D u_texture;
uniform bool u_is_using_texture;

out vec4 fragment_color;

const float ambient_strength = 0.1;

void main(void)
{
	vec4 ambient_color = u_light_color * ambient_strength;
	vec4 diffuse_color = max(dot(o_normal, u_light_dir), 
				 0.0) * u_light_color;
	if (u_is_using_texture) {
		diffuse_color *= texture(u_texture, o_texcoord);
	}
	vec4 main_color = (ambient_color + diffuse_color) * o_color;
	fragment_color = mix(main_color, vec4(1.0, 1.0, 0.0, 1.0),
			     float(u_is_selected) * 0.75);
}
