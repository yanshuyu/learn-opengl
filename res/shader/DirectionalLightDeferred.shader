#shader vertex
#version 450 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(a_pos, 1.f);
	f_uv = a_uv;
}


/////////////////////////////////////////////////////////////////////


#shader fragment
#version 450 core
#include "Phong.glsl"

#define MAXNUMCASCADE 4
#define PCFCornelSize 3

in vec2 f_uv;
out vec4 frag_color;

layout(location = 0) uniform sampler2D u_posW;
layout(location = 1) uniform sampler2D u_nromalW;
layout(location = 2) uniform sampler2D u_diffuse;
layout(location = 3) uniform sampler2D u_specular;
layout(location = 4) uniform sampler2D u_emissive;

layout(location = 5) uniform float u_maxShininess;
layout(location = 6) uniform vec3 u_cameraPosW;

layout(location = 8) uniform sampler2DArray u_shadowMapArray;

layout(location = 9) uniform mat4 u_lightVP[MAXNUMCASCADE];
layout(location = 13) uniform float u_cascadesFarZ[MAXNUMCASCADE];
layout(location = 17) uniform int u_numCascade;

layout(location = 18) uniform mat4 u_VPMat; // to projection space
layout(location = 19) uniform float u_shadowStrength;
layout(location = 20) uniform float u_shadowBias;


layout(std140) uniform LightBlock{
	vec4 u_lightColor; //(a for intensity)
	vec3 u_toLight;
};

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

subroutine float ShadowAttenType(vec3 posW);
subroutine uniform ShadowAttenType u_shadowAtten;

subroutine(ShadowAttenType)
float noShadow(vec3 posW) {
	return 1.f;
}

subroutine(ShadowAttenType)
float hardShadow(vec3 posW) {
	vec3 posProj = vec3(u_VPMat * vec4(posW, 1.f));
	int cascadeIdx = calcCascadeIndex(posProj.z);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posNDC = posL.xyz / posL.w;
	posNDC = posNDC * 0.5f + 0.5f; // convert to DNC

	if (posNDC.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;
 
	float depthL = texture(u_shadowMapArray, vec3(posNDC.xy, cascadeIdx)).r;
	bool inShaow = posNDC.z + u_shadowBias > depthL;
	return inShaow ? 1.f - u_shadowStrength : 1.f;
}

subroutine(ShadowAttenType)
float softShadow(vec3 posW) {
	vec3 posProj = vec3(u_VPMat * vec4(posW, 1.f));
	int cascadeIdx = calcCascadeIndex(posProj.z);
	vec4 posL = u_lightVP[cascadeIdx] * vec4(posW, 1.f); // convert to light space
	vec3 posNDC = posL.xyz / posL.w;
	posNDC = posNDC * 0.5f + 0.5f; // convert to DNC

	if (posNDC.z > 1.f) // fragment is outside of far plane of view frustum
		return 1.f;

	vec2 texelSize = vec2(1.f / textureSize(u_shadowMapArray, 0));
	int halfCornelSize = int(PCFCornelSize / 2);
	int shadowArea = 0;
	for (int x = -halfCornelSize; x <= halfCornelSize; x++) {
		for (int y = -halfCornelSize; y <= halfCornelSize; y++) {
			float depthL = texture(u_shadowMapArray, vec3(posNDC.xy + vec2(x, y) * texelSize, cascadeIdx)).r;
			shadowArea += posNDC.z + u_shadowBias > depthL ? 1 : 0;
		}
	}

	float atten = mix(1.f - u_shadowStrength, 1.f, 1.f - shadowArea / float(PCFCornelSize * PCFCornelSize));
	return atten;
}


vec3 PhongShading(vec3 P) {
	vec4 specular = texture(u_specular, f_uv);
	vec3 kd = texture(u_diffuse, f_uv).rgb;
	vec3 ks = specular.rgb;
	float shinness = specular.a * u_maxShininess;
	
	vec3 I = u_lightColor.rgb * u_lightColor.a;
	vec3 L = normalize(u_toLight);
	vec3 N = (texture(u_nromalW, f_uv).xyz - 0.5) * 2;
	vec3 V = normalize(u_cameraPosW - P);

	return Phong(I, L, N, V, kd, ks, shinness);
}


void main() {
	vec3 P = texture(u_posW, f_uv).xyz;
	vec3 C = PhongShading(P);
	vec3 E = texture(u_emissive, f_uv).rgb;

	float shadowAtte = u_shadowAtten(P);

	frag_color = vec4(C * shadowAtte + E, 1.f);
}


