#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	f_uv = a_uv;
}


///////////////////////////////////////


#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 0) uniform sampler2D u_diffuse;
layout(location = 1) uniform sampler2D u_emissive;

out vec4 frag_color;

void main() {
	vec3 texColor = texture(u_diffuse, f_uv).rgb + texture(u_emissive, f_uv).rgb;
	frag_color = vec4(texColor, 1.f);
}