#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	f_uv = a_uv;
}


////////////////////////////////////


#shader fragment
#version 450 core

in vec2 f_uv;
out vec4 frag_color;


uniform sampler2D u_DiffuseMap;
uniform sampler2D u_NormalMap;

uniform vec3 u_AmbientSky;
uniform vec3 u_AmbientGround;


vec3 calcAmibientLight() {
	vec3 normalW = (texture(u_NormalMap, f_uv).xyz - 0.5) * 2;
	return mix(u_AmbientGround, u_AmbientSky, normalW.y * 0.5 + 0.5);
}


void main() {
	vec3 mainColor = texture(u_DiffuseMap, f_uv).rgb;
	frag_color = vec4(calcAmibientLight() * mainColor, 1.f);
}
