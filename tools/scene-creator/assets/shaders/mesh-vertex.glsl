#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texcoord;
layout (location = 3) in vec4 a_color;

uniform mat4 u_proj_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_model_matrix;

out vec3 o_normal;
out vec2 o_texcoord;
out vec4 o_color;

void main(void)
{
	gl_Position = u_proj_matrix * u_view_matrix *
		      u_model_matrix * vec4(a_position, 1.0);
	o_normal = a_normal;
	o_texcoord = a_texcoord;
	o_color = a_color;
}
