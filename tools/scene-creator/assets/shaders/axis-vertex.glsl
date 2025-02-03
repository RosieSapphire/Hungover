#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

uniform mat4 u_proj_matrix;
uniform mat4 u_view_matrix;

out vec4 o_color;

void main(void)
{
	gl_Position = u_proj_matrix * u_view_matrix * vec4(a_position, 1.0);
	o_color = a_color;
}
