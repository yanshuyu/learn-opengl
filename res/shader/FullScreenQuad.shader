#shader vertex
#version 450 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	frag_uv = a_uv;
}


////////////////////////////////////////////////


#shader fragment
#version 450 core

in vec2 frag_uv;
out vec4 frag_color;

vec3 GammaCorrection(vec3 rgb) {
	return pow(rgb, vec3(1.f / 2.2f));
}

uniform sampler2D u_color;

void main() {
	vec3 C = GammaCorrection(texture(u_color, frag_uv).rgb);
	frag_color = vec4(C, 1.f);
}
