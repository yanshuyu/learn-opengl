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
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} vs_out;


uniform mat4 u_VPMat;

void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
	
	vs_out.pos_W = u_Transform(vec4(a_pos, 1.f), a_bones, a_weights).xyz;
	vs_out.normal_W = normalize(u_Transform(vec4(a_normal, 0.f), a_bones, a_weights).xyz);
	vs_out.tangent_W = normalize(u_Transform(vec4(a_tangent, 0.f), a_bones, a_weights).xyz);
	vs_out.uv = a_uv;
}


///////////////////////////////////////////////////////////



#shader fragment
#version 450 core

in VS_OUT{
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} fs_in;


layout(location = 0) out vec3 o_posW;
layout(location = 1) out vec3 o_normalW;
layout(location = 2) out vec4 o_albedo;
layout(location = 3) out vec4 o_specular;
layout(location = 4) out vec3 o_emissive;
layout(location = 5) out vec3 o_tmr; // materialtype(1:phong, 2:pbr)/metallic/roughness/


vec3 sRGB2RGB(in vec3 color) {
	return pow(color, vec3(2.2f));
}

uniform sampler2D u_albedoMap;
uniform vec4 u_mainColor; // rgb(diffuse) a(alpha)

uniform sampler2D u_emissiveMap;
uniform vec3 u_emissiveColor;

uniform sampler2D u_normalMap;

uniform sampler2D u_specularMap; // phong material property
uniform vec4 u_specularColor; // rgb(specular) a(shinness)

uniform sampler2D u_roughnessMap; // PBR meterial property
uniform float u_roughness;

uniform sampler2D u_metallicMap;
uniform float u_metalness;

uniform ivec3 u_hasANEMap; // has albedo/normal/emissive map
uniform ivec3 u_hasSMRMap; // has specular/metallic/ roughness map


subroutine void ShadingMode(vec3 P, vec3 N, vec4 A, vec3 E);
subroutine uniform ShadingMode u_ShadingMode;

subroutine (ShadingMode)
void PhongShading(vec3 P, vec3 N, vec4 A, vec3 E) {
	vec3 S = u_hasSMRMap.x == 1 ? sRGB2RGB(texture(u_specularMap, fs_in.uv).rgb) : vec3(1.f);
	S *= u_specularColor.rgb;

	o_posW = P;
	o_normalW = (N + 1.f) * 0.5f;
	o_albedo = A;
	o_specular = vec4(S, u_specularColor.a);
	o_emissive = E;
	o_tmr = vec3(1.f, 0.f, 0.f);
}

subroutine (ShadingMode) 
void PBRShading(vec3 P, vec3 N, vec4 A, vec3 E) {
	float M = u_hasSMRMap.y == 1 ? texture(u_metallicMap, fs_in.uv).r : 1.f;
	float R = u_hasSMRMap.z == 1 ? texture(u_roughnessMap, fs_in.uv).r : 1.f;
	M *= u_metalness;
	R *= u_roughness;

	o_posW = P;
	o_normalW = (N + 1) * 0.5f;
	o_albedo = A;
	o_specular = vec4(0.f);
	o_emissive = E;
	o_tmr = vec3(2.f, M, R);
}


void main() {
	vec3 P = fs_in.pos_W;
	vec3 N = fs_in.normal_W;
	if (u_hasANEMap.y == 1) {
		vec3 normal = (texture(u_normalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		N = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}
	
	vec4 A = u_hasANEMap.x == 1 ? texture(u_albedoMap, fs_in.uv) : vec4(1.f);
	A.rgb = sRGB2RGB(A.rgb) * u_mainColor.rgb;
	A.a *=  u_mainColor.a;

	vec3 E = u_hasANEMap.z == 1 ? sRGB2RGB(texture(u_emissiveMap, fs_in.uv).rgb) : vec3(1.f);
	E *= u_emissiveColor;

	u_ShadingMode(P, N, A, E);
}
