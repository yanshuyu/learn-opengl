#shader vertex
#version 450 core
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
	float z_Proj; // project clip space depth
} vs_out;


uniform mat4 u_VPMat;

void main() {
	gl_Position = u_VPMat * u_Transform(vec4(a_pos, 1.f), a_bones, a_weights);
	vs_out.pos_W = u_Transform(vec4(a_pos, 1.f), a_bones, a_weights).xyz;
	
	vs_out.normal_W = normalize(u_Transform(vec4(a_normal, 0.f), a_bones, a_weights).xyz);
	vs_out.tangent_W = normalize(u_Transform(vec4(a_tangent, 0.f), a_bones, a_weights).xyz);
	
	vs_out.uv = a_uv;
	vs_out.z_Proj = gl_Position.z;
}


/////////////////////////////////////////////////////////////////////


#shader fragment
#version 450 core
#include "Material.glsl"
#include "Phong.glsl"
#include "PBR.glsl"

#define MAXNUMCASCADE 4
#define PCFCornelSize 3

in VS_OUT{
	vec3 pos_W;
	vec3 normal_W;
	vec3 tangent_W;
	vec2 uv;
	float z_Proj; // project clip space depth
} fs_in;

out vec4 frag_color;

uniform vec3 u_cameraPosW;

uniform sampler2DArray u_shadowMapArray;
uniform mat4 u_lightVP[MAXNUMCASCADE];
uniform float u_cascadesFarZ[MAXNUMCASCADE];
uniform int u_numCascade;

uniform float u_shadowStrength;
uniform float u_shadowBias;

uniform	vec4 u_lightColor; //(a for intensity)
uniform	vec3 u_toLight;



int calcCascadeIndex(in float projDepth) {
	int idx = 0;
	for (int i = 0; i < u_numCascade; i++) {
		if (projDepth <= u_cascadesFarZ[i]) {
			idx = i;
			break;
		}
	}
	return idx;
}


subroutine float ShadowAttenType();
subroutine uniform ShadowAttenType u_shadowAtten;

subroutine (ShadowAttenType)
float noShadow() {
	return 1.f;
}

subroutine (ShadowAttenType)
float hardShadow() {
	int cascadeIdx = calcCascadeIndex(fs_in.z_Proj);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(fs_in.pos_W, 1.f); // convert to light space
	vec3 posProj = posL.xyz / posL.w;
	posProj = posProj * 0.5f + 0.5f; // convert to DNC

	if (posProj.z >= 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	float depthL = texture(u_shadowMapArray, vec3(posProj.xy, cascadeIdx)).r;
	bool inShaow = posProj.z + u_shadowBias > depthL;
	
	return inShaow ? 1.f - u_shadowStrength : 1.f;
}

subroutine (ShadowAttenType)
float softShadow() {
	int cascadeIdx = calcCascadeIndex(fs_in.z_Proj);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(fs_in.pos_W, 1.f); // convert to light space
	vec3 posProj = posL.xyz / posL.w;
	posProj = posProj * 0.5f + 0.5f; // convert to DNC

	if (posProj.z >= 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	vec2 texelSize = vec2(1.f / textureSize(u_shadowMapArray, 0));
	int halfCornelSize = int(PCFCornelSize / 2);
	int shadowArea = 0;
	for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
		for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
			float depthL = texture(u_shadowMapArray, vec3(posProj.xy + vec2(x, y) * texelSize, cascadeIdx)).r;
			shadowArea += posProj.z + u_shadowBias > depthL ? 1 : 0;
		}
	}

	float atten = mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
	return atten;
}


subroutine vec3 ShadingMode(vec3 I, vec3 L, vec3 N, vec3 V, vec3 A);
subroutine uniform ShadingMode u_ShadingMode;

subroutine (ShadingMode)
vec3 PhongShading(vec3 I, vec3 L, vec3 N, vec3 V, vec3 A) {
	vec3 S = u_HasANRMMap.b == 1 ? sRGB2RGB(texture(u_SpecularMap, fs_in.uv).rgb) : vec3(1.f);
	vec3 kd = A;
	vec3 ks = S * ub_Mtl.specular.rgb;
	float shinness = ub_Mtl.specular.a;

	return Phong(I, L, N, V, kd, ks, shinness);
}

subroutine (ShadingMode)
vec3 PBRShading(vec3 I, vec3 L, vec3 N, vec3 V, vec3 A) {
	float R = u_HasANRMMap.b == 1 ? texture(u_RoughnessMap, fs_in.uv).r : 1.f;
	R *= ub_Mtl.roughness;

	float M = u_HasANRMMap.a == 1 ? texture(u_MetallicMap, fs_in.uv).r : 1.f;
	M *= ub_Mtl.metalness;

	return PBR(I, L, N, V, A, M, R);
}

void main() {
	vec3 I = u_lightColor.rgb * u_lightColor.a;
	vec3 L = normalize(u_toLight);
	vec3 V = normalize(u_cameraPosW - fs_in.pos_W);

	vec4 A = u_HasANRMMap.r == 1 ? texture(u_AlbedoMap, fs_in.uv) : vec4(1.f);
	if (A.a < 0.1f) {
		discard;
	}

	A.rgb = sRGB2RGB(A.rgb) * ub_Mtl.albedo.rgb;

	vec3 N = fs_in.normal_W;
	if (u_HasANRMMap.g == 1) {
		vec3 normal = (texture(u_NormalMap, fs_in.uv).xyz - 0.5) * 2;
		vec3 biTangent = normalize(cross(fs_in.normal_W, fs_in.tangent_W));
		N = normalize(mat3(fs_in.tangent_W, biTangent, fs_in.normal_W) * normal);
	}

	vec3 C = u_ShadingMode(I, L, N, V, A.rgb);

	float shadowAtte = u_shadowAtten();

	frag_color = vec4(C * shadowAtte, 1.f);
}


