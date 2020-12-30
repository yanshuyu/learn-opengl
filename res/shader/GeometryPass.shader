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
#include "Material.glsl"

in VS_OUT{
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} fs_in;


layout(location = 0) out vec3 o_PosW;
layout(location = 1) out vec3 o_NormalW;
layout(location = 2) out vec4 o_Albedo;
layout(location = 3) out vec4 o_Specular;
layout(location = 4) out vec3 o_EMR; // (emissive/metalness/roughness)
layout(location = 5) out int o_ShadeMode; // (1:phong, 2:pbr)

vec3 sRGB2RGB(in vec3 color) {
	return pow(color, vec3(2.2f));
}


subroutine void ShadingMode(vec3 P, vec3 N, vec4 A);
subroutine uniform ShadingMode u_ShadingMode;

subroutine (ShadingMode)
void PhongShading(vec3 P, vec3 N, vec4 A) {
	vec3 S = u_HasANRMMap.b == 1 ? sRGB2RGB(texture(u_SpecularMap, fs_in.uv).rgb) : vec3(1.f);
	S *= ub_Mtl.specular .rgb;

	o_PosW = P;
	o_NormalW = (N + 1.f) * 0.5f;
	o_Albedo = A;
	o_Specular = vec4(S, ub_Mtl.specular.a);
	o_EMR = vec3(ub_Mtl.emissive, 0.f, 0.f);
	o_ShadeMode = 1;
}

subroutine (ShadingMode) 
void PBRShading(vec3 P, vec3 N, vec4 A) {
	float R = u_HasANRMMap.b == 1 ? texture(u_RoughnessMap, fs_in.uv).r : 1.f;
	float M = u_HasANRMMap.a == 1 ? texture(u_MetallicMap, fs_in.uv).r : 1.f;

	M *= ub_Mtl.metalness;
	R *= ub_Mtl.roughness;

	o_PosW = P;
	o_NormalW = (N + 1) * 0.5f;
	o_Albedo = A;
	o_Specular = vec4(0.f);
	o_EMR = vec3(ub_Mtl.emissive, M, R);
	o_ShadeMode = 2;
}


void main() {
	vec3 P = fs_in.pos_W;

	vec4 A = u_HasANRMMap.r == 1 ? texture(u_AlbedoMap, fs_in.uv) : vec4(1.f);
	A.rgb = sRGB2RGB(A.rgb) * ub_Mtl.albedo.rgb;
	A.a *= ub_Mtl.albedo.a;

	vec3 N = fs_in.normal_W;
	if (u_HasANRMMap.g == 1) {
		vec3 normal = (texture(u_NormalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		N = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	u_ShadingMode(P, N, A);
}
