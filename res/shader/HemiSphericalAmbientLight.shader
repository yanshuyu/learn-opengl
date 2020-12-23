#shader vertex
#version 450 core
#include "Transform.glsl"

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in ivec4 a_bones;
layout(location = 5) in vec4 a_weights;


out VS_OUT{
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} vs_out;


uniform mat4 u_VPMat;


void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);

	vs_out.normal_W = normalize(u_Transform(vec4(a_normal, 0.f), a_bones, a_weights).xyz);
	vs_out.tangent_W = normalize(u_Transform(vec4(a_tangent, 0.f), a_bones, a_weights).xyz);
	vs_out.uv = a_uv;
}



///////////////////////////////////////////////



#shader fragment
#version 450 core


in VS_OUT{
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
}fs_in;

out vec4 frag_color;

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform ivec2 u_HasANMap;

uniform vec3 u_AmbientSky;
uniform vec3 u_AmbientGround;


vec3 calcAmibientLight() {
	vec3 normalW = fs_in.normal_W;
	if (u_HasANMap.y == 1) {
		vec3 normal = (texture(u_NormalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		normalW = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	return mix(u_AmbientGround, u_AmbientSky, normalW.y * 0.5 + 0.5);
}


void main() {
	vec4 mainColor = u_HasANMap.x == 1 ? texture(u_AlbedoMap, fs_in.uv) : vec4(1.f);
	if (mainColor.a < 0.1f)
		discard;

	mainColor.rgb *= mainColor.rgb; // sRGB to RGb

	frag_color = vec4(calcAmibientLight() * mainColor.rgb, 1.f);
}

