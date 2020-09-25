#shader vertex
#version 450 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

layout(location = 0) uniform mat4 u_MVP;

out vec2 f_uv;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 0.f, 1.f);
	f_uv = a_uv;
}


/////////////////////////////////////////


#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 1) uniform sampler2D u_texture;

out vec4 frag_color;

void main() {
	vec3 texColor = texture(u_texture, f_uv).rgb;
	frag_color = vec4(texColor, 1.f);
}