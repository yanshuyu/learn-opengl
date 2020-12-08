#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 frag_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	frag_uv = a_uv;
}


//////////////////////////////////////////


#shader fragment
#version 450 core

const int MAX_WEIGHTS = 19;

in vec2 frag_uv;
out vec4 frag_color;

uniform int u_Pass;
uniform int u_NumWeights;
uniform float u_Weights[MAX_WEIGHTS];
uniform sampler2D u_Texture;


void main() {
	vec2 pixSz = 1.f / textureSize(u_Texture, 0);
	vec3 sum = texture(u_Texture, frag_uv).rgb * u_Weights[0];

	if (u_Pass == 0) { // vertical pass
		for (int i = 1; i < u_NumWeights; i++) {
			sum += texture(u_Texture, frag_uv + vec2(pixSz.x, 0) * i).rgb * u_Weights[i];
			sum += texture(u_Texture, frag_uv - vec2(pixSz.x, 0) * i).rgb * u_Weights[i];
		}

	} else if (u_Pass == 1) { // horizontal pass
		for (int i = 1; i < u_NumWeights; i++) {
			sum += texture(u_Texture, frag_uv + vec2(0, pixSz.y) * i).rgb * u_Weights[i];
			sum += texture(u_Texture, frag_uv - vec2(0, pixSz.y) * i).rgb * u_Weights[i];
		}
	}

	frag_color = vec4(sum, 1.f);
}
