#shader vertex
#version 450 core
#include "Transform.glsl"

layout(location = 0) in vec3 a_pos;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;

out vec2 f_uv;

uniform mat4 u_VPMat;

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
	f_uv = a_uv;
}



///////////////////////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in vec2 f_uv;

uniform sampler2D u_diffuseMap;
uniform bool u_hasDiffuseMap;
uniform vec4 u_diffuseColor;

layout(location = 0) out vec4 frag_color;


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}


void main() {
	vec3 texColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	frag_color = vec4(u_diffuseColor.rgb * texColor, 1.0);
}
