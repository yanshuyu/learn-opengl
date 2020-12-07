#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	frag_uv = a_uv;
}



////////////////////////////////////



#shader fragment
#version 450 core


in vec2 frag_uv;
out vec4 frag_color;


uniform sampler2D u_hdrTexture;
uniform float u_Exposure;


void main() {
	const float gamma = 2.2f;
	vec3 hdrColor = texture(u_hdrTexture, frag_uv).rgb;

	// exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColor * u_Exposure);
	// gamma correction 
	mapped = pow(mapped, vec3(1.0 / gamma));

	frag_color = vec4(mapped, 1.f);
}