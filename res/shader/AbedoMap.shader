#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_uv;

layout(location = 0) uniform mat4 MVP;

out vec3 f_color;
out vec2 f_uv;

void main() {
	gl_Position = MVP * vec4(a_pos, 1.0);
	f_color = a_color;
	f_uv = a_uv;
}





#shader fragment
#version 450 core

in vec3 f_color;
in vec2 f_uv;

layout(location = 1) uniform sampler2D u_abedoMap;

out vec4 final_color;

void main() {
	vec4 texColor = texture(u_abedoMap, f_uv);
	final_color = vec4(mix(f_color, texColor.rgb, texColor.a), 1.0);
}