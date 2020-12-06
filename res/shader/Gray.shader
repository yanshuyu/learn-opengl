#shader vertex
#version 450 core


layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	frag_uv = a_uv;
}



////////////////////////////////////////



#shader fragment
#version 450 core

in vec2 frag_uv;
out vec4 frag_color;

uniform sampler2D u_Texture;


void main() {
	float lum = dot(texture(u_Texture, frag_uv).rgb, vec3(0.2126f, 0.7152f, 0.0722f));
	frag_color = vec4(vec3(lum), 1.f);
}



