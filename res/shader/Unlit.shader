#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec3 a_bitangent;
layout(location = 4) in vec2 a_uv;

layout(location = 0) uniform mat4 u_mvp;

out vec2 f_uv;

void main() {
	gl_Position = u_mvp * vec4(a_pos, 1.0);
	f_uv = a_uv;
}



///////////////////////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in vec2 f_uv;

layout(location = 1) uniform sampler2D u_diffuseMap;
layout(location = 2) uniform vec4 u_diffuseColor;

layout(location = 0) out vec4 frag_color;

void main() {
	vec4 texColor = texture(u_diffuseMap, f_uv);
	frag_color = vec4((u_diffuseColor * texColor).rgb, 1.0);
	//frag_color = vec4(1, 0, 0, 1);
}