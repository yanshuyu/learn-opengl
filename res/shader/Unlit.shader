#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 4) in vec2 a_uv;

layout(location = 0) uniform mat4 u_VPMat;
layout(location = 1) uniform mat4 u_ModelMat;

out vec2 f_uv;

invariant gl_Position;

void main() {
	gl_Position = u_VPMat * u_ModelMat * vec4(a_pos, 1.0);
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


vec3 sRGB2RGB(in vec3 color) {
	return color * color;
}


void main() {
	vec3 texColor = u_hasDiffuseMap ? sRGB2RGB(texture(u_diffuseMap, f_uv).rgb) : vec3(1.f);
	frag_color = vec4(u_diffuseColor.rgb * texColor, 1.0);
}
