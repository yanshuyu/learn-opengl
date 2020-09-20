#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 4) in vec2 a_uv;

layout(location = 0) uniform mat4 u_MVP;

out vec2 f_uv;

invariant gl_Position;

void main() {
	gl_Position = u_MVP * vec4(a_pos, 1.0);
	f_uv = a_uv;
}



///////////////////////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 1) uniform sampler2D u_diffuseMap;
layout(location = 2) uniform bool u_hasDiffuseMap;
layout(location = 3) uniform vec4 u_diffuseColor;

layout(location = 0) out vec4 frag_color;


void correctGamma(in vec4 i_color, out vec4 o_color) {
	o_color = vec4((i_color * i_color).rgb, i_color.a);
}



void main() {
	vec4 texColor = u_hasDiffuseMap ? texture(u_diffuseMap, f_uv) : vec4(1.f, 1.f, 1.f, 1.f);

	correctGamma(texColor, texColor);
	
	frag_color = vec4(u_diffuseColor.rgb * texColor.rgb, 1.0);
}
