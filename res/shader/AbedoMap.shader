#shader vertex
#version 450 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(a_pos, 0.0, 1.0);
	f_uv = a_uv;
}





#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 0) uniform sampler2D u_abedoMap;

out vec4 final_color;

void main() {
	final_color = texture(u_abedoMap, f_uv);
}