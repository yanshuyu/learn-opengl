#shader vertex
#version 430 core
#include"Transform.glsl"

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


/////////////////////////////////////////////////////////////////////




#shader fragment
#version 430 core
#include "Material.glsl"
#include "Lightting.glsl"

layout(early_fragment_tests) in;

in VS_OUT{
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
} fs_in;


uniform vec3 u_CameraPosW;


struct Fragment { // an entry in piexl frag link list
	vec4 color;
	float depth;
	uint next;
};

layout(std430) buffer FragmentBuffer { // ssao to store all frags
	Fragment b_Frags[];
};

layout(binding = 0, offset = 0) uniform atomic_uint ac_NextBufferIdx;

layout(r32ui) uniform uimage2D u_FragHeader; // store frag link list header of current pixel location 

uniform uint u_MaxNumFrag;



subroutine vec3 ShadingMode(vec3 A, vec3 N);
subroutine uniform ShadingMode u_ShadingMode;

subroutine(ShadingMode)
vec3 PhongShading(vec3 A, vec3 N) {
	vec3 S = u_HasANRMMap.b == 1 ? sRGB2RGB(texture(u_SpecularMap, fs_in.uv).rgb) : vec3(1.f);
	S *= ub_Mtl.specular.rgb;
	float shinness = ub_Mtl.specular.a;

	return PhongLightting(fs_in.pos_W, N, u_CameraPosW, A, S, shinness);
}

subroutine(ShadingMode)
vec3 PBRShading(vec3 A, vec3 N) {
	float R = u_HasANRMMap.b == 1 ? texture(u_RoughnessMap, fs_in.uv).r : 1.f;
	R *= ub_Mtl.roughness;

	float M = u_HasANRMMap.a == 1 ? texture(u_MetallicMap, fs_in.uv).r : 1.f;
	M *= ub_Mtl.metalness;

	return PBRLightting(fs_in.pos_W, N, u_CameraPosW, A, M, R);
}


vec4 ShadingFrag() {
	vec4 A = u_HasANRMMap.r == 1 ? texture(u_AlbedoMap, fs_in.uv) : vec4(1.f);
	A.rgb = sRGB2RGB(A.rgb) * ub_Mtl.albedo.rgb;
	
	vec3 N = fs_in.normal_W;
	if (u_HasANRMMap.g == 1) {
		vec3 normal = (texture(u_NormalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		N = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	vec3 C = u_ShadingMode(A.rgb, N);
	C += ub_Mtl.emissive * A.rgb;

	float a = A.a * ub_Mtl.albedo.a;

	return vec4(C, a);
}



void main() {
	uint idx = atomicCounterIncrement(ac_NextBufferIdx);
	if (idx < u_MaxNumFrag) {
		uint next = imageAtomicExchange(u_FragHeader, ivec2(gl_FragCoord.xy), idx);
		Fragment frag;
		frag.color = ShadingFrag();
		frag.depth = gl_FragCoord.z;
		frag.next = next;
		b_Frags[idx] = frag;
	}
}
