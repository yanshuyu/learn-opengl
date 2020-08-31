#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 0) uniform mat4 u_mvp;

out vec3 f_normal;
out vec2 f_uv;

void main() {
	gl_Position = u_mvp * vec4(a_pos, 1.0);
	f_uv = a_uv;
	f_normal = (u_mvp * vec4(a_normal, 0.0)).xyz;
}





#shader fragment
#version 450 core

in vec3 f_normal;
in vec2 f_uv;

layout(location = 1) uniform sampler2D u_abedoMap;

out vec4 final_color;

void main() {
	final_color = vec4(f_normal, 1.0) * texture(u_abedoMap, f_uv);
}